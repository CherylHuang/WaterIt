#include "GearLevel.h"
#include "cocostudio/CocoStudio.h"
#include "ui/CocosGUI.h"

USING_NS_CC;

#define MAX_2(X,Y) (X)>(Y) ? (X) : (Y)

#define StaticAndDynamicBodyExample 1
using namespace cocostudio::timeline;
using namespace ui;

GearLevel::~GearLevel()
{

#ifdef BOX2D_DEBUG
	if (_DebugDraw != NULL) delete _DebugDraw;
#endif

	if (_b2World != nullptr) delete _b2World;
	//  for releasing Plist&Texture
		SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("box2d.plist");
		SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("box2d_pic.plist");
		SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("box2d_bg_pic.plist");
	//Director::getInstance()->getTextureCache()->removeUnusedTextures();

}

Scene* GearLevel::createScene()
{
	auto scene = Scene::create();
	auto layer = GearLevel::create();
	scene->addChild(layer);
	return scene;
}

// on "init" you need to initialize your instance
bool GearLevel::init()
{
	//////////////////////////////
	// 1. super init first
	if (!Layer::init())
	{
		return false;
	}

	//  For Loading Plist+Texture
	SpriteFrameCache::getInstance()->addSpriteFramesWithFile("box2d.plist");
	SpriteFrameCache::getInstance()->addSpriteFramesWithFile("box2d_pic.plist");
	SpriteFrameCache::getInstance()->addSpriteFramesWithFile("box2d_bg_pic.plist");

	_visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	//----------------------------------------------------------
	//剪刀
	_bCutting = false;
	_icutNum = 0;
	_iCutLineNum = 0;
	//----------------------------------------------------------
	// 建立 Box2D world
	_b2World = nullptr;
	b2Vec2 Gravity = b2Vec2(0.0f, -9.8f);		//重力方向
	//b2Vec2 Gravity = b2Vec2(0.0f, 9.8f);		//重力方向
	bool AllowSleep = true;			//允許睡著
	_b2World = new b2World(Gravity);	//創建世界
	_b2World->SetAllowSleeping(AllowSleep);	//設定物件允許睡著

											// Create Scene with csb file
	_csbRoot = CSLoader::createNode("GearLevel.csb");
#ifndef BOX2D_DEBUG
	// 設定顯示背景圖示
	auto bgSprite = _csbRoot->getChildByName("bg");
	bgSprite->setVisible(true);

#endif
	addChild(_csbRoot, 1);

	//讀取物件
	_startWater = dynamic_cast<Sprite*>(_csbRoot->getChildByName("water"));
	_Bush_dry = dynamic_cast<Sprite*>(_csbRoot->getChildByName("Bush_dry"));
	_Bush_live = dynamic_cast<Sprite*>(_csbRoot->getChildByName("Bush_live"));

	_DoorIn = dynamic_cast<Sprite*>(_csbRoot->getChildByName("trainsDoor_in"));
	_contactListener._DoorIn = _DoorIn; //傳給碰撞偵測
	_DoorOut = dynamic_cast<Sprite*>(_csbRoot->getChildByName("trainsDoor_out"));

	//滑輪線用
	_cactus1 = dynamic_cast<Sprite*>(_csbRoot->getChildByName("cactus"));
	_cactus2 = dynamic_cast<Sprite*>(_csbRoot->getChildByName("cactus_2"));
	_gear1 = dynamic_cast<Sprite*>(_csbRoot->getChildByName("gear_1")); this->addChild(_gear1, 3);
	_gear2 = dynamic_cast<Sprite*>(_csbRoot->getChildByName("gear_2")); this->addChild(_gear2, 3);
	_board = dynamic_cast<Sprite*>(_csbRoot->getChildByName("seesaw01_1")); this->addChild(_board, 2);
	_board_left = dynamic_cast<Sprite*>(_csbRoot->getChildByName("seesaw01_2")); this->addChild(_board_left, 3);
	_board_right = dynamic_cast<Sprite*>(_csbRoot->getChildByName("seesaw01_3")); this->addChild(_board_right, 3);
	_iLineNum = 0;

	//轉動輪子用
	_fakeTurn1 = dynamic_cast<Sprite*>(_csbRoot->getChildByName("bubble_1"));
	_fakeTurn2 = dynamic_cast<Sprite*>(_csbRoot->getChildByName("bubble_2"));

	//----------------------------------------------------------
	//Start & End
	_fStartCount = 0;
	_contactListener._StartSprite = _startWater;
	_contactListener._EndSprite = _Bush_dry;
	_startPt = _startWater->getPosition();

	for (int i = 0; i < SPARKS_NUMBER; i++) {		//儲存粒子圖片
		_sparks[i] = Sprite::createWithSpriteFrameName("pen.png");
		_sparks[i]->setColor(Color3B(0, 146 - rand() % 20, 200 - rand() % 20));
		_sparks[i]->setBlendFunc(BlendFunc::ADDITIVE);
		this->addChild(_sparks[i], 5);
		_sparks[i]->setVisible(false);
	}
	//----------------------------------------------------------
	//Button
	_bBtnPressed = false;

	//Type 1
	auto BtnPos = dynamic_cast<Button*>(_csbRoot->getChildByName("btn_retry")); //重新遊戲鈕
	Point loc = BtnPos->getPosition();
	BtnPos->setVisible(false);
	_retrybtn = C3SButton::create();
	_retrybtn->setButtonInfo(NORMAL_BTN, "btn_retry_0.png", "btn_retry_2.png", "btn_retry_1.png", loc);
	_retrybtn->setScale(0.4f);
	_retrybtn->setVisible(true);
	this->addChild(_retrybtn, 3);

	BtnPos = dynamic_cast<Button*>(_csbRoot->getChildByName("btn_home")); //回首頁鈕
	loc = BtnPos->getPosition();
	BtnPos->setVisible(false);
	_homebtn = C3SButton::create();
	_homebtn->setButtonInfo(NORMAL_BTN, "btn_home_0.png", "btn_home_2.png", "btn_home_1.png", loc);
	_homebtn->setScale(0.4f);
	_homebtn->setVisible(true);
	this->addChild(_homebtn, 3);

	//Type 2
	BtnPos = dynamic_cast<Button*>(_csbRoot->getChildByName("cut")->getChildByName("cut_icon")); //剪刀鈕
	loc = _csbRoot->getChildByName("cut")->convertToWorldSpace(BtnPos->getPosition());
	BtnPos->setVisible(false);
	_cutbtn = C3SButton::create();
	_cutbtn->setButtonInfo(SELECT_BTN, "cut_icon.png", "cut_icon_on.png", "cut_icon_on.png", loc);
	_cutbtn->setScale(0.5f);
	_cutbtn->setVisible(true);
	this->addChild(_cutbtn, 3);

	//----------------------------------------------------------
	setStartEndpoint();			//起終點
	//createStaticBoundary();	//邊界
	setupStatic();				//靜態物體
	setupRopeJoint();			//繩子1
	setupRope2Joint();			//繩子2
	setupRope3Joint();			//繩子3
	setupRope4Joint();			//繩子4
	setupWoodBoard();			//木板設定
	setupPulleyJoint();			//滑輪
	setupMoveDoor();			//傳送門
	setupGearStatic();			//滑輪齒輪 靜態物品設定
	setupGear();				//齒輪


#ifdef BOX2D_DEBUG
	//DebugDrawInit
	_DebugDraw = nullptr;
	_DebugDraw = new GLESDebugDraw(PTM_RATIO);
	//設定DebugDraw
	_b2World->SetDebugDraw(_DebugDraw);
	//選擇繪製型別
	uint32 flags = 0;
	flags += GLESDebugDraw::e_shapeBit;						//繪製形狀
	flags += GLESDebugDraw::e_pairBit;
	flags += GLESDebugDraw::e_jointBit;
	flags += GLESDebugDraw::e_centerOfMassBit;
	flags += GLESDebugDraw::e_aabbBit;
	//設定繪製類型
	_DebugDraw->SetFlags(flags);
#endif

	_b2World->SetContactListener(&_contactListener); //碰撞聆聽器

	_listener1 = EventListenerTouchOneByOne::create();	//創建一個一對一的事件聆聽器
	_listener1->onTouchBegan = CC_CALLBACK_2(GearLevel::onTouchBegan, this);		//加入觸碰開始事件
	_listener1->onTouchMoved = CC_CALLBACK_2(GearLevel::onTouchMoved, this);		//加入觸碰移動事件
	_listener1->onTouchEnded = CC_CALLBACK_2(GearLevel::onTouchEnded, this);		//加入觸碰離開事件

	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(_listener1, this);	//加入剛創建的事件聆聽器
	this->schedule(CC_SCHEDULE_SELECTOR(GearLevel::doStep));

	return true;
}

void GearLevel::setupStatic()
{
	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody; // 設定這個 Body 為 靜態的
	bodyDef.userData = NULL;
	// 在 b2World 中產生該 Body, 並傳回產生的 b2Body 物件的指標
	// 產生一次，就可以讓後面所有的 Shape 使用
	b2Body *body = _b2World->CreateBody(&bodyDef);

	b2FixtureDef fixtureDef; // 產生 Fixture

	// 讀取所有 wall_ 開頭的圖示 當成是靜態物體
	char tmp[20] = "";

	for (size_t i = 1; i <= 3; i++)
	{
		sprintf(tmp, "wall_%d", i);
		auto frameSprite = (Sprite *)_csbRoot->getChildByName(tmp);

		Point loc = frameSprite->getPosition();
		Size size = frameSprite->getContentSize();
		float scale = frameSprite->getScale();
		b2BodyDef staticBodyDef;
		staticBodyDef.type = b2_staticBody;
		staticBodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
		staticBodyDef.userData = frameSprite;
		b2Body* staticBody = _b2World->CreateBody(&staticBodyDef);

		// 產生靜態邊界所需要的 boxShape
		b2PolygonShape boxShape;
		boxShape.SetAsBox((size.width*0.5f - 2.0f)*scale / PTM_RATIO, (size.height*0.5f - 2.0f)*scale / PTM_RATIO);

		b2FixtureDef fixtureDef;
		fixtureDef.shape = &boxShape;
		staticBody->CreateFixture(&fixtureDef);
	}
}

void GearLevel::setStartEndpoint()
{
	// 取得並設定終點 Bush_dry 圖示為【靜態物體】
	Point loc = _Bush_dry->getPosition();
	Size size = _Bush_dry->getContentSize();
	b2BodyDef staticBodyDef;
	staticBodyDef.type = b2_staticBody;
	staticBodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	staticBodyDef.userData = _Bush_dry;
	b2Body* staticBody = _b2World->CreateBody(&staticBodyDef);

	b2PolygonShape boxShape;
	boxShape.SetAsBox(size.width*0.5f / PTM_RATIO, size.height*0.5f / PTM_RATIO);

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &boxShape;
	fixtureDef.isSensor = true; // 設定為 Sensor
	staticBody->CreateFixture(&fixtureDef);

	//----------------------------------------------------
	// 取得並設定起點 water 為【動態物體】
	loc = _startWater->getPosition();
	size = _startWater->getContentSize();
	auto scale = _startWater->getScale();

	b2BodyDef dynamicBodyDef;
	dynamicBodyDef.type = b2_dynamicBody;
	dynamicBodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	dynamicBodyDef.userData = _startWater;
	_waterBody = _b2World->CreateBody(&dynamicBodyDef);

	b2CircleShape circleShape;
	circleShape.m_radius = size.width*0.5f*scale / PTM_RATIO;

	b2FixtureDef fixtureDef2;
	fixtureDef2.shape = &circleShape;
	fixtureDef2.density = 5.0f;
	_waterBody->CreateFixture(&fixtureDef2);
}

//木板
void GearLevel::setupWoodBoard()
{
	// 取得並設定 seesaw01 為【動態物體】
	auto seesawSprite = _csbRoot->getChildByName("seesaw01_1");
	Point loc = seesawSprite->getPosition();
	Size size = seesawSprite->getContentSize();
	auto scale = seesawSprite->getScale();

	b2BodyDef dynamicBodyDef;
	dynamicBodyDef.type = b2_dynamicBody;
	dynamicBodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	dynamicBodyDef.userData = seesawSprite;
	b2Body* seesawBody = _b2World->CreateBody(&dynamicBodyDef);

	b2PolygonShape boxShape;
	boxShape.SetAsBox((size.width*0.5f*scale) / PTM_RATIO, (size.height*0.5f*scale + 2.f) / PTM_RATIO);

	b2FixtureDef fixtureDef2;
	fixtureDef2.shape = &boxShape;
	fixtureDef2.density = 2.0f;
	seesawBody->CreateFixture(&fixtureDef2);
	_Board = seesawBody;
	//------
	seesawSprite = _csbRoot->getChildByName("seesaw01_2");
	loc = seesawSprite->getPosition();
	size = seesawSprite->getContentSize();
	scale = seesawSprite->getScale();

	b2BodyDef dynamicBodyDef2;
	dynamicBodyDef2.type = b2_dynamicBody;
	dynamicBodyDef2.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	dynamicBodyDef2.userData = seesawSprite;
	b2Body* seesawBody2 = _b2World->CreateBody(&dynamicBodyDef2);

	b2PolygonShape boxShape2;
	boxShape2.SetAsBox((size.width*0.5f*scale - 2.f) / PTM_RATIO, (size.height*0.5f*scale - 10.f) / PTM_RATIO); //高度修正

	b2FixtureDef fixtureDef3;
	fixtureDef3.shape = &boxShape2;
	fixtureDef3.density = 2.0f;
	seesawBody2->CreateFixture(&fixtureDef3);
	_leftBoard = seesawBody2; //紀錄左邊木板
	//------
	seesawSprite = _csbRoot->getChildByName("seesaw01_3");
	loc = seesawSprite->getPosition();
	size = seesawSprite->getContentSize();
	scale = seesawSprite->getScale();

	b2BodyDef dynamicBodyDef3;
	dynamicBodyDef3.type = b2_dynamicBody;
	dynamicBodyDef3.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	dynamicBodyDef3.userData = seesawSprite;
	b2Body* seesawBody3 = _b2World->CreateBody(&dynamicBodyDef3);

	b2PolygonShape boxShape3;
	boxShape3.SetAsBox((size.width*0.5f*scale - 2.f) / PTM_RATIO, (size.height*0.5f*scale - 10.f) / PTM_RATIO); //高度修正

	b2FixtureDef fixtureDef4;
	fixtureDef4.shape = &boxShape3;
	fixtureDef4.density = 2.0f;
	seesawBody3->CreateFixture(&fixtureDef4);
	_rightBoard = seesawBody3;

	//----------------------------------------------------
	// 建立 焊接Joint 連結
	b2WeldJointDef WeldJointDef;
	WeldJointDef.Initialize(seesawBody, seesawBody2, seesawBody->GetPosition() + b2Vec2(-100 / PTM_RATIO, 0));
	_b2World->CreateJoint(&WeldJointDef); // 使用預設值焊接

	// 建立 焊接Joint 連結
	b2WeldJointDef JointDef;
	JointDef.Initialize(seesawBody, seesawBody3, seesawBody->GetPosition() + b2Vec2(100 / PTM_RATIO, 0));
	_b2World->CreateJoint(&JointDef); // 使用預設值焊接
}
//左1繩子
void GearLevel::setupRopeJoint()
{
	// 取得並設定 stickRope 圖示為【靜態物體】
	auto frameSprite = _csbRoot->getChildByName("stickRope_1");
	Point locHead = frameSprite->getPosition();
	Size sizeHead = frameSprite->getContentSize();
	auto scaleHead = frameSprite->getScale();

	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody;
	bodyDef.position.Set(locHead.x / PTM_RATIO, locHead.y / PTM_RATIO);
	bodyDef.userData = frameSprite;
	b2Body* ropeHeadBody = _b2World->CreateBody(&bodyDef);
	b2FixtureDef  fixtureDef;
	fixtureDef.density = 1.0f;  fixtureDef.friction = 0.25f; fixtureDef.restitution = 0.25f;
	b2PolygonShape boxShape;
	boxShape.SetAsBox((sizeHead.width*0.5f*scaleHead - 2.f) / PTM_RATIO, (sizeHead.height*0.5f*scaleHead - 2.f) / PTM_RATIO);
	fixtureDef.shape = &boxShape;
	ropeHeadBody->CreateFixture(&fixtureDef);

	//取得並設定 cactus 為【動態物體】
	auto circleSprite = _csbRoot->getChildByName("cactus");
	Point locTail = circleSprite->getPosition();
	Size sizeTail = circleSprite->getContentSize();
	auto scaleTail = circleSprite->getScale();

	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(locTail.x / PTM_RATIO, locTail.y / PTM_RATIO);
	bodyDef.userData = circleSprite;
	_cactusBody = _b2World->CreateBody(&bodyDef);
	b2CircleShape circleShape;
	circleShape.m_radius = (sizeTail.width - 4)*0.5f*scaleTail*0.85f / PTM_RATIO; //0.85 : 向內修正(縮小)
	fixtureDef.shape = &circleShape;
	fixtureDef.density = 7.f;  fixtureDef.friction = 0.25f; fixtureDef.restitution = 0.25f;
	_cactusBody->CreateFixture(&fixtureDef);


	//產生繩子關節
	b2RopeJointDef JointDef;
	JointDef.bodyA = ropeHeadBody;
	JointDef.bodyB = _cactusBody;
	JointDef.localAnchorA = b2Vec2(0, 0);
	JointDef.localAnchorB = b2Vec2(0, 0);
	JointDef.maxLength = (locHead.y - locTail.y) / PTM_RATIO;
	JointDef.collideConnected = true;
	_MainRopeJoint = (b2RopeJoint*)_b2World->CreateJoint(&JointDef);

	// 中間再以線段相連，
	char tmp[20] = "";
	Sprite *ropeSprite[10];
	Point loc[10];
	Size  size[11];
	b2Body* ropeBody[10];

	bodyDef.type = b2_dynamicBody;
	// 因為是繩子所以重量不要太重
	fixtureDef.density = 0.01f;  fixtureDef.friction = 1.0f; fixtureDef.restitution = 0.0f;
	fixtureDef.shape = &boxShape;
	// 產生一系列的繩子段落 rope01_01 ~ rope01_15，同時接起來
	for (int i = 0; i < 10; i++)
	{
		sprintf(tmp, "rope01_%02d", i + 1);
		ropeSprite[i] = (Sprite *)_csbRoot->getChildByName(tmp);
		loc[i] = ropeSprite[i]->getPosition();
		size[i] = ropeSprite[i]->getContentSize();

		bodyDef.position.Set(loc[i].x / PTM_RATIO, (loc[i].y - 10.f) / PTM_RATIO);
		bodyDef.userData = ropeSprite[i];
		ropeBody[i] = _b2World->CreateBody(&bodyDef);
		boxShape.SetAsBox((size[i].width - 4)*0.5f / PTM_RATIO, (size[i].height - 4)*0.5f / PTM_RATIO);
		ropeBody[i]->CreateFixture(&fixtureDef);
	}
	// 利用 RevoluteJoint 將線段彼此連接在一起
	// 先連結 ropeHeadBody 與  ropeBody[0]

	float locAnchor = 0.5f*(size[0].height - 10) / PTM_RATIO;
	b2RevoluteJointDef revJoint;
	revJoint.bodyA = ropeHeadBody;		//Head與線段連結
	revJoint.localAnchorA.Set(0, -0.5f);
	revJoint.bodyB = ropeBody[0];
	revJoint.localAnchorB.Set(0, locAnchor);
	_b2World->CreateJoint(&revJoint);
	for (int i = 0; i < 9; i++) {		//中間線段連結
		revJoint.bodyA = ropeBody[i];
		revJoint.localAnchorA.Set(0, -locAnchor);
		revJoint.bodyB = ropeBody[i + 1];
		revJoint.localAnchorB.Set(0, locAnchor);
		_b2World->CreateJoint(&revJoint);
	}
	revJoint.bodyA = ropeBody[9];		//Tail與線段連結
	revJoint.localAnchorA.Set(0, -locAnchor);
	revJoint.bodyB = _cactusBody;
	revJoint.localAnchorB.Set(-0.5f, 1.f);
	_b2World->CreateJoint(&revJoint);
}
//左2繩子
void GearLevel::setupRope2Joint()
{
	// 取得並設定 stickRope 圖示為【靜態物體】
	auto frameSprite = _csbRoot->getChildByName("stickRope_2");
	Point locHead = frameSprite->getPosition();
	Size sizeHead = frameSprite->getContentSize();
	auto scaleHead = frameSprite->getScale();

	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody;
	bodyDef.position.Set(locHead.x / PTM_RATIO, locHead.y / PTM_RATIO);
	bodyDef.userData = frameSprite;
	b2Body* ropeHeadBody = _b2World->CreateBody(&bodyDef);
	b2FixtureDef  fixtureDef;
	fixtureDef.density = 1.0f;  fixtureDef.friction = 0.25f; fixtureDef.restitution = 0.25f;
	b2PolygonShape boxShape;
	boxShape.SetAsBox((sizeHead.width*0.5f*scaleHead - 2.f) / PTM_RATIO, (sizeHead.height*0.5f*scaleHead - 2.f) / PTM_RATIO);
	fixtureDef.shape = &boxShape;
	ropeHeadBody->CreateFixture(&fixtureDef);

	//取得並設定 cactus 為【動態物體】
	auto circleSprite = _csbRoot->getChildByName("cactus");
	Point locTail = circleSprite->getPosition();
	Size sizeTail = circleSprite->getContentSize();
	auto scaleTail = circleSprite->getScale();

	//產生繩子關節
	b2RopeJointDef JointDef;
	JointDef.bodyA = ropeHeadBody;
	JointDef.bodyB = _cactusBody;
	JointDef.localAnchorA = b2Vec2(0, 0);
	JointDef.localAnchorB = b2Vec2(0, 0);
	JointDef.maxLength = (locHead.y - locTail.y) / PTM_RATIO;
	JointDef.collideConnected = true;
	_MainRope2Joint = (b2RopeJoint*)_b2World->CreateJoint(&JointDef);

	// 中間再以線段相連，
	char tmp[20] = "";
	Sprite *ropeSprite[10];
	Point loc[10];
	Size  size[11];
	b2Body* ropeBody[10];

	bodyDef.type = b2_dynamicBody;
	// 因為是繩子所以重量不要太重
	fixtureDef.density = 0.012f;  fixtureDef.friction = 1.0f; fixtureDef.restitution = 0.0f;
	fixtureDef.shape = &boxShape;
	// 產生一系列的繩子段落 rope01_01 ~ rope01_15，同時接起來
	for (int i = 0; i < 10; i++)
	{
		sprintf(tmp, "rope02_%02d", i + 1);
		ropeSprite[i] = (Sprite *)_csbRoot->getChildByName(tmp);
		loc[i] = ropeSprite[i]->getPosition();
		size[i] = ropeSprite[i]->getContentSize();

		bodyDef.position.Set(loc[i].x / PTM_RATIO, (loc[i].y - 10.f) / PTM_RATIO);
		bodyDef.userData = ropeSprite[i];
		ropeBody[i] = _b2World->CreateBody(&bodyDef);
		boxShape.SetAsBox((size[i].width - 4)*0.5f / PTM_RATIO, (size[i].height - 4)*0.5f / PTM_RATIO);
		ropeBody[i]->CreateFixture(&fixtureDef);
	}
	// 利用 RevoluteJoint 將線段彼此連接在一起
	// 先連結 ropeHeadBody 與  ropeBody[0]

	float locAnchor = 0.5f*(size[0].height - 10) / PTM_RATIO;
	b2RevoluteJointDef revJoint;
	revJoint.bodyA = ropeHeadBody;		//Head與線段連結
	revJoint.localAnchorA.Set(0, -0.5f);
	revJoint.bodyB = ropeBody[0];
	revJoint.localAnchorB.Set(0, locAnchor);
	_b2World->CreateJoint(&revJoint);
	for (int i = 0; i < 9; i++) {		//中間線段連結
		revJoint.bodyA = ropeBody[i];
		revJoint.localAnchorA.Set(0, -locAnchor);
		revJoint.bodyB = ropeBody[i + 1];
		revJoint.localAnchorB.Set(0, locAnchor);
		_b2World->CreateJoint(&revJoint);
	}
	revJoint.bodyA = ropeBody[9];		//Tail與線段連結
	revJoint.localAnchorA.Set(0, -locAnchor);
	revJoint.bodyB = _cactusBody;
	revJoint.localAnchorB.Set(0.5f, 1.f);
	_b2World->CreateJoint(&revJoint);
}
//右2繩子
void GearLevel::setupRope3Joint()
{
	// 取得並設定 stickRope 圖示為【靜態物體】
	auto frameSprite = _csbRoot->getChildByName("stickRope_3");
	Point locHead = frameSprite->getPosition();
	Size sizeHead = frameSprite->getContentSize();
	auto scaleHead = frameSprite->getScale();

	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody;
	bodyDef.position.Set(locHead.x / PTM_RATIO, locHead.y / PTM_RATIO);
	bodyDef.userData = frameSprite;
	b2Body* ropeHeadBody = _b2World->CreateBody(&bodyDef);
	b2FixtureDef  fixtureDef;
	fixtureDef.density = 1.0f;  fixtureDef.friction = 0.25f; fixtureDef.restitution = 0.25f;
	b2PolygonShape boxShape;
	boxShape.SetAsBox((sizeHead.width*0.5f*scaleHead - 2.f) / PTM_RATIO, (sizeHead.height*0.5f*scaleHead - 2.f) / PTM_RATIO);
	fixtureDef.shape = &boxShape;
	ropeHeadBody->CreateFixture(&fixtureDef);

	//取得並設定 cactus 為【動態物體】
	auto circleSprite = _csbRoot->getChildByName("cactus_2");
	Point locTail = circleSprite->getPosition();
	Size sizeTail = circleSprite->getContentSize();
	auto scaleTail = circleSprite->getScale();

	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(locTail.x / PTM_RATIO, locTail.y / PTM_RATIO);
	bodyDef.userData = circleSprite;
	_cactus2Body = _b2World->CreateBody(&bodyDef);
	b2CircleShape circleShape;
	circleShape.m_radius = (sizeTail.width - 4)*0.5f*scaleTail*0.85f / PTM_RATIO; //0.85 : 向內修正(縮小)
	fixtureDef.shape = &circleShape;
	fixtureDef.density = 7.f;  fixtureDef.friction = 0.25f; fixtureDef.restitution = 0.25f;
	_cactus2Body->CreateFixture(&fixtureDef);


	//產生繩子關節
	b2RopeJointDef JointDef;
	JointDef.bodyA = ropeHeadBody;
	JointDef.bodyB = _cactus2Body;
	JointDef.localAnchorA = b2Vec2(0, 0);
	JointDef.localAnchorB = b2Vec2(0, 0);
	JointDef.maxLength = (locHead.y - locTail.y) / PTM_RATIO;
	JointDef.collideConnected = true;
	_MainRope3Joint = (b2RopeJoint*)_b2World->CreateJoint(&JointDef);

	// 中間再以線段相連，
	char tmp[20] = "";
	Sprite *ropeSprite[10];
	Point loc[10];
	Size  size[11];
	b2Body* ropeBody[10];

	bodyDef.type = b2_dynamicBody;
	// 因為是繩子所以重量不要太重
	fixtureDef.density = 0.013f;  fixtureDef.friction = 1.0f; fixtureDef.restitution = 0.0f;
	fixtureDef.shape = &boxShape;
	// 產生一系列的繩子段落 rope01_01 ~ rope01_15，同時接起來
	for (int i = 0; i < 10; i++)
	{
		sprintf(tmp, "rope03_%02d", i + 1);
		ropeSprite[i] = (Sprite *)_csbRoot->getChildByName(tmp);
		loc[i] = ropeSprite[i]->getPosition();
		size[i] = ropeSprite[i]->getContentSize();

		bodyDef.position.Set(loc[i].x / PTM_RATIO, (loc[i].y - 10.f) / PTM_RATIO);
		bodyDef.userData = ropeSprite[i];
		ropeBody[i] = _b2World->CreateBody(&bodyDef);
		boxShape.SetAsBox((size[i].width - 4)*0.5f / PTM_RATIO, (size[i].height - 4)*0.5f / PTM_RATIO);
		ropeBody[i]->CreateFixture(&fixtureDef);
	}
	// 利用 RevoluteJoint 將線段彼此連接在一起
	// 先連結 ropeHeadBody 與  ropeBody[0]

	float locAnchor = 0.5f*(size[0].height - 10) / PTM_RATIO;
	b2RevoluteJointDef revJoint;
	revJoint.bodyA = ropeHeadBody;		//Head與線段連結
	revJoint.localAnchorA.Set(0, -0.5f);
	revJoint.bodyB = ropeBody[0];
	revJoint.localAnchorB.Set(0, locAnchor);
	_b2World->CreateJoint(&revJoint);
	for (int i = 0; i < 9; i++) {		//中間線段連結
		revJoint.bodyA = ropeBody[i];
		revJoint.localAnchorA.Set(0, -locAnchor);
		revJoint.bodyB = ropeBody[i + 1];
		revJoint.localAnchorB.Set(0, locAnchor);
		_b2World->CreateJoint(&revJoint);
	}
	revJoint.bodyA = ropeBody[9];		//Tail與線段連結
	revJoint.localAnchorA.Set(0, -locAnchor);
	revJoint.bodyB = _cactus2Body;
	revJoint.localAnchorB.Set(-0.5f, 1.f);
	_b2World->CreateJoint(&revJoint);
}
//右1繩子
void GearLevel::setupRope4Joint()
{
	// 取得並設定 stickRope 圖示為【靜態物體】
	auto frameSprite = _csbRoot->getChildByName("stickRope_4");
	Point locHead = frameSprite->getPosition();
	Size sizeHead = frameSprite->getContentSize();
	auto scaleHead = frameSprite->getScale();

	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody;
	bodyDef.position.Set(locHead.x / PTM_RATIO, locHead.y / PTM_RATIO);
	bodyDef.userData = frameSprite;
	b2Body* ropeHeadBody = _b2World->CreateBody(&bodyDef);
	b2FixtureDef  fixtureDef;
	fixtureDef.density = 1.0f;  fixtureDef.friction = 0.25f; fixtureDef.restitution = 0.25f;
	b2PolygonShape boxShape;
	boxShape.SetAsBox((sizeHead.width*0.5f*scaleHead - 2.f) / PTM_RATIO, (sizeHead.height*0.5f*scaleHead - 2.f) / PTM_RATIO);
	fixtureDef.shape = &boxShape;
	ropeHeadBody->CreateFixture(&fixtureDef);

	//取得並設定 cactus 為【動態物體】
	auto circleSprite = _csbRoot->getChildByName("cactus_2");
	Point locTail = circleSprite->getPosition();
	Size sizeTail = circleSprite->getContentSize();
	auto scaleTail = circleSprite->getScale();

	//產生繩子關節
	b2RopeJointDef JointDef;
	JointDef.bodyA = ropeHeadBody;
	JointDef.bodyB = _cactus2Body;
	JointDef.localAnchorA = b2Vec2(0, 0);
	JointDef.localAnchorB = b2Vec2(0, 0);
	JointDef.maxLength = (locHead.y - locTail.y) / PTM_RATIO;
	JointDef.collideConnected = true;
	_MainRope4Joint = (b2RopeJoint*)_b2World->CreateJoint(&JointDef);

	// 中間再以線段相連，
	char tmp[20] = "";
	Sprite *ropeSprite[10];
	Point loc[10];
	Size  size[11];
	b2Body* ropeBody[10];

	bodyDef.type = b2_dynamicBody;
	// 因為是繩子所以重量不要太重
	fixtureDef.density = 0.014f;  fixtureDef.friction = 1.0f; fixtureDef.restitution = 0.0f;
	fixtureDef.shape = &boxShape;
	// 產生一系列的繩子段落 rope01_01 ~ rope01_15，同時接起來
	for (int i = 0; i < 10; i++)
	{
		sprintf(tmp, "rope04_%02d", i + 1);
		ropeSprite[i] = (Sprite *)_csbRoot->getChildByName(tmp);
		loc[i] = ropeSprite[i]->getPosition();
		size[i] = ropeSprite[i]->getContentSize();

		bodyDef.position.Set(loc[i].x / PTM_RATIO, (loc[i].y - 10.f) / PTM_RATIO);
		bodyDef.userData = ropeSprite[i];
		ropeBody[i] = _b2World->CreateBody(&bodyDef);
		boxShape.SetAsBox((size[i].width - 4)*0.5f / PTM_RATIO, (size[i].height - 4)*0.5f / PTM_RATIO);
		ropeBody[i]->CreateFixture(&fixtureDef);
	}
	// 利用 RevoluteJoint 將線段彼此連接在一起
	// 先連結 ropeHeadBody 與  ropeBody[0]

	float locAnchor = 0.5f*(size[0].height - 10) / PTM_RATIO;
	b2RevoluteJointDef revJoint;
	revJoint.bodyA = ropeHeadBody;		//Head與線段連結
	revJoint.localAnchorA.Set(0, -0.5f);
	revJoint.bodyB = ropeBody[0];
	revJoint.localAnchorB.Set(0, locAnchor);
	_b2World->CreateJoint(&revJoint);
	for (int i = 0; i < 9; i++) {		//中間線段連結
		revJoint.bodyA = ropeBody[i];
		revJoint.localAnchorA.Set(0, -locAnchor);
		revJoint.bodyB = ropeBody[i + 1];
		revJoint.localAnchorB.Set(0, locAnchor);
		_b2World->CreateJoint(&revJoint);
	}
	revJoint.bodyA = ropeBody[9];		//Tail與線段連結
	revJoint.localAnchorA.Set(0, -locAnchor);
	revJoint.bodyB = _cactus2Body;
	revJoint.localAnchorB.Set(0.5f, 1.f);
	_b2World->CreateJoint(&revJoint);
}
//滑輪
void GearLevel::setupPulleyJoint()
{
	// 左側滑輪位置
	auto Sprite = _csbRoot->getChildByName("gear_1");
	Point loc = Sprite->getPosition();

	//產生滑輪關節
	b2PulleyJointDef JointDef;
	JointDef.Initialize(_cactusBody, _Board,
		b2Vec2((loc.x - 48.f) / PTM_RATIO, loc.y / PTM_RATIO), //齒輪位置
		b2Vec2((loc.x + 48.f) / PTM_RATIO, loc.y / PTM_RATIO),
		_cactusBody->GetWorldCenter(),
		b2Vec2(_Board->GetWorldCenter().x - (100.f / PTM_RATIO), _Board->GetWorldCenter().y),
		1);
	_b2World->CreateJoint(&JointDef);

	//---------------------------------------------
	// 右側滑輪位置
	Sprite = _csbRoot->getChildByName("gear_2");
	loc = Sprite->getPosition();

	//產生滑輪關節
	JointDef.Initialize(_Board, _cactus2Body,
		b2Vec2((loc.x - 48.f) / PTM_RATIO, loc.y / PTM_RATIO), //齒輪位置
		b2Vec2((loc.x + 48.f) / PTM_RATIO, loc.y / PTM_RATIO),
		b2Vec2(_Board->GetWorldCenter().x + (100.f / PTM_RATIO), _Board->GetWorldCenter().y),
		_cactus2Body->GetWorldCenter(),
		1);
	_b2World->CreateJoint(&JointDef);
}
//傳送門
void GearLevel::setupMoveDoor()
{
	// 取得並設定進入點 trainsDoor_in 圖示為【靜態物體】感應器
	Point loc = _DoorIn->getPosition();
	Size size = _DoorIn->getContentSize();
	auto scale = _DoorIn->getScale();
	b2BodyDef doorInBodyDef;
	doorInBodyDef.type = b2_staticBody;
	doorInBodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	doorInBodyDef.userData = _DoorIn;
	b2Body* doorInBody = _b2World->CreateBody(&doorInBodyDef);

	b2CircleShape circleShape;
	circleShape.m_radius = size.width*0.5f*scale*0.85f / PTM_RATIO; //0.85f : 修正

	b2FixtureDef doorInfixtureDef;
	doorInfixtureDef.shape = &circleShape;
	doorInfixtureDef.isSensor = true; // 設定為 Sensor
	doorInBody->CreateFixture(&doorInfixtureDef);
}
//滑輪齒輪
void GearLevel::setupGearStatic()
{
	// 取得並設定 bubble 為【靜態物體】
	Point loc = _fakeTurn1->getPosition();
	Size size = _fakeTurn1->getContentSize();
	auto scale = _fakeTurn1->getScale();

	b2BodyDef staticBodyDef;
	staticBodyDef.type = b2_staticBody;
	staticBodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	staticBodyDef.userData = _fakeTurn1;
	_b2fakeTurn1 = _b2World->CreateBody(&staticBodyDef);

	b2CircleShape circleShape;
	circleShape.m_radius = size.width*0.5f*scale / PTM_RATIO;

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &circleShape;
	fixtureDef.density = 10.0f;
	_b2fakeTurn1->CreateFixture(&fixtureDef);
	//---------------------------------------------------------------
	// 取得並設定 bubble 為【靜態物體】
	loc = _fakeTurn2->getPosition();
	size = _fakeTurn2->getContentSize();
	scale = _fakeTurn2->getScale();

	b2BodyDef staticBodyDef2;
	staticBodyDef2.type = b2_staticBody;
	staticBodyDef2.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	staticBodyDef2.userData = _fakeTurn2;
	_b2fakeTurn2 = _b2World->CreateBody(&staticBodyDef2);

	_b2fakeTurn2->CreateFixture(&fixtureDef);

	//---------------------------------------------------------------
	// 取得並設定 gear 背景靜態物體
	loc = _gear1->getPosition();
	size = _gear1->getContentSize();
	scale = _gear1->getScale();

	b2BodyDef staticBodyDef3;
	staticBodyDef3.type = b2_staticBody;
	staticBodyDef3.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	staticBodyDef3.userData = NULL;
	_groundbody = _b2World->CreateBody(&staticBodyDef3);

	_groundbody->CreateFixture(&fixtureDef);
	//---------------------------------------------------------------
	// 取得並設定 gear 背景靜態物體
	loc = _gear2->getPosition();
	size = _gear2->getContentSize();
	scale = _gear2->getScale();

	b2BodyDef staticBodyDef4;
	staticBodyDef4.type = b2_staticBody;
	staticBodyDef4.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	staticBodyDef4.userData = NULL;
	_groundbody2 = _b2World->CreateBody(&staticBodyDef4);

	_groundbody2->CreateFixture(&fixtureDef);
}
void GearLevel::setupGear()
{
	// 取得並設定起點 gear1 為【動態物體】
	auto loc = _gear1->getPosition();
	auto size = _gear1->getContentSize();
	auto scale = _gear1->getScale();

	b2BodyDef dynamicBodyDef;
	dynamicBodyDef.type = b2_dynamicBody;
	dynamicBodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	dynamicBodyDef.userData = _gear1;
	b2Body *gearBody1 = _b2World->CreateBody(&dynamicBodyDef);

	b2CircleShape circleShape;
	circleShape.m_radius = size.width*0.5f*scale / PTM_RATIO;

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &circleShape;
	fixtureDef.density = 0.1f;
	gearBody1->CreateFixture(&fixtureDef);
	//----------------------------------------------------
	// 取得並設定起點 gear2 為【動態物體】
	loc = _gear2->getPosition();
	size = _gear2->getContentSize();
	scale = _gear2->getScale();

	b2BodyDef dynamicBodyDef2;
	dynamicBodyDef2.type = b2_dynamicBody;
	dynamicBodyDef2.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	dynamicBodyDef2.userData = _gear2;
	b2Body *gearBody2 = _b2World->CreateBody(&dynamicBodyDef2);

	gearBody2->CreateFixture(&fixtureDef);
	//----------------------------------------------------
	//建立 Joint
	//Revolute Joint
	b2RevoluteJointDef revJoint;
	revJoint.bodyA = _groundbody;		//base
	revJoint.localAnchorA.Set(0, 0);
	revJoint.bodyB = gearBody1;	//gear
	revJoint.localAnchorB.Set(0, 0);
	_b2World->CreateJoint(&revJoint);
	//Revolute Joint
	b2RevoluteJointDef revJoint2;
	revJoint2.bodyA = _groundbody2;		//base
	revJoint2.localAnchorA.Set(0, 0);
	revJoint2.bodyB = gearBody2;	//gear
	revJoint2.localAnchorB.Set(0, 0);
	_b2World->CreateJoint(&revJoint2);
}

void GearLevel::DrawCuttingLine(cocos2d::Point firstPt, cocos2d::Point Pt, cocos2d::Color4F color, int num)
{
	//畫出剪取線段
	_drawCut[num] = DrawNode::create();
	_drawCut[num]->drawLine(firstPt, Pt, color);
	//_drawCut[num]->clear();
	_cutNode->addChild(_drawCut[num], 5);
}

void GearLevel::doStep(float dt)
{
	int velocityIterations = 8;	// 速度迭代次數
	int positionIterations = 1; // 位置迭代次數 迭代次數一般設定為8~10 越高越真實但效率越差
								// Instruct the world to perform a single step of simulation.
								// It is generally best to keep the time step and iterations fixed.
	_b2World->Step(dt, velocityIterations, positionIterations);

	// 取得 _b2World 中所有的 body 進行處理
	// 最主要是根據目前運算的結果，更新附屬在 body 中 sprite 的位置
	for (b2Body* body = _b2World->GetBodyList(); body; body = body->GetNext())
	{
		// 以下是以 Body 有包含 Sprite 顯示為例
		if (body->GetUserData() != NULL)
		{
			Sprite *spriteData = (Sprite*)body->GetUserData();
			if (spriteData != NULL) {
				spriteData->setPosition(body->GetPosition().x*PTM_RATIO, body->GetPosition().y*PTM_RATIO);
				spriteData->setRotation(-1 * CC_RADIANS_TO_DEGREES(body->GetAngle()));
			}
		}
		//// 跑出螢幕外面就讓物體從 b2World 中移除
		//if (body->GetType() == b2BodyType::b2_dynamicBody) {
		//	float x = body->GetPosition().x * PTM_RATIO;
		//	float y = body->GetPosition().y * PTM_RATIO;
		//	if (x > _visibleSize.width || x < 0 || y >  _visibleSize.height || y < 0) {
		//		if (body->GetUserData() != NULL) {
		//			Sprite* spriteData = (Sprite *)body->GetUserData();
		//			this->removeChild(spriteData);
		//		}
		//		b2Body* nextbody = body->GetNext(); // 取得下一個 body
		//		_b2World->DestroyBody(body); // 釋放目前的 body
		//		body = nextbody;  // 讓 body 指向剛才取得的下一個 body
		//	}
		//	else body = body->GetNext(); //否則就繼續更新下一個Body
		//}
		//else body = body->GetNext(); //否則就繼續更新下一個Body
	}

	//----------------------------------------------------------------------------
	//for Start & End
	if(!_waterBody->IsAwake() && !_contactListener._bWin)_fStartCount += dt; //水球靜止中、遊戲尚未結束 : 開始計時
	if (_fStartCount > 3.0f) {	//靜止超過三秒，重置位置
		_fStartCount -= 3.0f;
		_waterBody->SetType(b2_staticBody);
		_waterBody->SetTransform(b2Vec2(_startPt.x / PTM_RATIO, _startPt.y / PTM_RATIO), _waterBody->GetAngle());
		_waterBody->SetType(b2_dynamicBody); //BUG修正 : 直接設定位置會卡住，轉換Type讓Body持續落下
	}
	if (_contactListener._bWin) {		//過關
		_startWater->setVisible(false);
		_Bush_dry->setVisible(false);
		_Bush_live->setVisible(true);
		_fWinCount += dt;
	}
	if (_fWinCount > 4.0f) { //跳轉下一關
		_fWinCount = -100.f; //避免重複執行
		auto Scene = GravityLevel::createScene();
		Director::getInstance()->replaceScene(TransitionFade::create(1, Scene, Color3B(255, 255, 255)));
	}
	// 產生過關水花------------------------------------
	if (_contactListener._bCreateSpark) {
		_contactListener._bCreateSpark = false;	//產生完關閉
		for (int i = 0; i < SPARKS_NUMBER; i++) {
			_sparks[i]->setVisible(true);
			//產生小方塊資料
			b2BodyDef RectBodyDef;
			RectBodyDef.position.Set(_contactListener._createLoc.x, _contactListener._createLoc.y);
			RectBodyDef.type = b2_dynamicBody;
			RectBodyDef.userData = _sparks[i];
			b2PolygonShape RectShape;
			RectShape.SetAsBox(5 / PTM_RATIO, 5 / PTM_RATIO);
			b2Body* RectBody = _b2World->CreateBody(&RectBodyDef);
			b2FixtureDef RectFixtureDef;
			RectFixtureDef.shape = &RectShape;
			RectFixtureDef.density = 1.3f;
			RectFixtureDef.isSensor = true;
			b2Fixture*RectFixture = RectBody->CreateFixture(&RectFixtureDef);

			//給力量
			RectBody->ApplyForce(b2Vec2(rand() % 51 - 25, 50 + rand() % 30), _contactListener._createLoc, true);
		}
	}

	//----------------------------------------------------------------------------
	// for cutting
	if (_contactListener._bDeleteJoint) { //delete joint
		_contactListener._bDeleteJoint = false;
		for (b2Joint* j = _b2World->GetJointList(); j; ) //若rope碰撞 移除其身上的joint
		{
			b2Joint* jointToDestroy = j;
			j = j->GetNext();
			if (jointToDestroy->GetBodyA() == _contactListener._ropeBody) _b2World->DestroyJoint(jointToDestroy);	//切斷繩子
			if (jointToDestroy == _MainRopeJoint && _contactListener._bDeleteRope1) {
				_b2fakeTurn1->SetType(b2_dynamicBody);
				_b2World->DestroyJoint(_MainRopeJoint);	//切斷主繩
			}
			if (jointToDestroy == _MainRope2Joint && _contactListener._bDeleteRope2) {
				_b2fakeTurn1->SetType(b2_dynamicBody);
				_b2World->DestroyJoint(_MainRope2Joint);	//切斷主繩
			}
			if (jointToDestroy == _MainRope3Joint && _contactListener._bDeleteRope3) {
				_b2fakeTurn2->SetType(b2_dynamicBody);
				_b2World->DestroyJoint(_MainRope3Joint);	//切斷主繩
			}
			if (jointToDestroy == _MainRope4Joint && _contactListener._bDeleteRope4) {
				_b2fakeTurn2->SetType(b2_dynamicBody);
				_b2World->DestroyJoint(_MainRope4Joint);	//切斷主繩
			}
		}
	}
	//----------------------------------------------------------------------------
	//for transition
	if (_contactListener._bMove) {
		_contactListener._bMove = false;
		_waterBody->SetType(b2_staticBody);
		_waterBody->SetTransform(b2Vec2(_DoorOut->getPositionX() / PTM_RATIO, _DoorOut->getPositionY() / PTM_RATIO), _waterBody->GetAngle());
		_waterBody->SetType(b2_dynamicBody);
		_waterBody->ApplyLinearImpulse(b2Vec2(0, 100 + rand() % 51), _waterBody->GetWorldCenter(), true); //向上的力
	}

	//----------------------------------------------------------------------------
	//繪製滑輪繩
	auto loc_gear1 = _gear1->getPosition();
	auto loc_cactus1 = _cactus1->getPosition();
	auto loc_gear2 = _gear2->getPosition();
	auto loc_cactus2 = _cactus2->getPosition();
	auto loc_board = _board->getPosition();

	_pulleyline1 = DrawNode::create();
	_pulleyline1->drawLine(loc_gear1 + Vec2(-33.21f, 0), loc_cactus1 + Vec2(0, 46.5f), Color4F::BLACK);
	this->addChild(_pulleyline1, 2);

	_pulleyline2 = DrawNode::create();
	_pulleyline2->drawLine(loc_gear1 + Vec2(33.21f, 0), loc_board + Vec2(-100.f, 0), Color4F::BLACK);
	this->addChild(_pulleyline2, 2);

	_pulleyline3 = DrawNode::create();
	_pulleyline3->drawLine(loc_gear2 + Vec2(33.21f, 0), loc_cactus2 + Vec2(0, 46.5f), Color4F::BLACK);
	this->addChild(_pulleyline3, 2);

	_pulleyline4 = DrawNode::create();
	_pulleyline4->drawLine(loc_gear2 + Vec2(-33.21f, 0), loc_board + Vec2(100.f, 0), Color4F::BLACK);
	this->addChild(_pulleyline4, 2);

	_iLineNum++;
	if (_iLineNum >= 2) {
		this->removeChild(_destroyLine1);
		this->removeChild(_destroyLine2);
		this->removeChild(_destroyLine3);
		this->removeChild(_destroyLine4);
		_iLineNum--;
	}
	_destroyLine1 = _pulleyline1;
	_destroyLine2 = _pulleyline2;
	_destroyLine3 = _pulleyline3;
	_destroyLine4 = _pulleyline4;
	//----------------------------------------------------------------------------
	//繪製剪取線
	if (_bCutting) {
		_CutDrawLine = DrawNode::create();
		_CutDrawLine->drawLine(_tp_start, _tp, Color4F::GREEN);
		this->addChild(_CutDrawLine, 5);
		_iCutLineNum++;
		if (_iCutLineNum >= 2) {
			this->removeChild(_destroyCutDrawLine);
			_iCutLineNum--;
		}
		_destroyCutDrawLine = _CutDrawLine;
	}
}

bool GearLevel::onTouchBegan(cocos2d::Touch *pTouch, cocos2d::Event *pEvent)//觸碰開始事件
{
	Point touchLoc = pTouch->getLocation();
	_tp_start = touchLoc;
	_tp = touchLoc;
	//-------------------------------------------------------------------
	//For button
	if (_retrybtn->onTouchBegan(touchLoc) || _homebtn->onTouchBegan(touchLoc)) _bBtnPressed = true;

	//-------------------------------------------------------------------
	//for cutting
	if (!_bBtnPressed && _icutNum < CUTTING_NUM)_bCutting = true;	//不在按鈕上 還有剩餘畫圖次數才可畫圖
	if (_bCutting) {
		_cutNode = Node::create(); //新增此筆畫控制節點
		this->addChild(_cutNode, 5);
		_contactListener._cutNode = _cutNode; //傳入Box2d world
		_cutPt[0] = _cutPt[1] = touchLoc;	//第一點
	}
	//-------------------------------------------------------------------

	return true;
}

void  GearLevel::onTouchMoved(cocos2d::Touch *pTouch, cocos2d::Event *pEvent) //觸碰移動事件
{
	Point touchLoc = pTouch->getLocation();
	_tp = touchLoc;
	//-------------------------------------------------------------------
	//For cutting
	if (_bCutting && _icutNum < CUTTING_NUM) {
		_cutPt[1] = touchLoc;
	}
	//-------------------------------------------------------------------
	//For button
	_retrybtn->onTouchMoved(touchLoc);
	_homebtn->onTouchMoved(touchLoc);
}

void  GearLevel::onTouchEnded(cocos2d::Touch *pTouch, cocos2d::Event *pEvent) //觸碰結束事件 
{
	Point touchLoc = pTouch->getLocation();
	_tp = touchLoc;
	//-------------------------------------------------------------------
	//For cutting
	if (_bCutting && _icutNum < CUTTING_NUM) {
		DrawCuttingLine(_cutPt[0], _cutPt[1], Color4F::GREEN, _icutNum); //畫線函式

		//建立 【靜態物體】
		Point cutloc = _cutNode->getPosition();
		b2BodyDef cutbodyDef;				//body只產生一次
		cutbodyDef.type = b2_staticBody;
		cutbodyDef.position.Set(cutloc.x / PTM_RATIO, cutloc.y / PTM_RATIO);
		cutbodyDef.userData = _cutNode;
		b2Body *body = _b2World->CreateBody(&cutbodyDef);

		b2EdgeShape edgeShape;
		b2FixtureDef edgeFixtureDef; // 產生 Fixture
		edgeFixtureDef.shape = &edgeShape;
		edgeFixtureDef.isSensor = true;
		edgeShape.Set(b2Vec2(_cutPt[0].x / PTM_RATIO, _cutPt[0].y / PTM_RATIO), b2Vec2(_cutPt[1].x / PTM_RATIO, _cutPt[1].y / PTM_RATIO)); //剪取線段
		body->CreateFixture(&edgeFixtureDef);

		_icutNum++;				//已剪取次數+1
		char tmp[10] = "";
		sprintf(tmp, "cut_%d", CUTTING_NUM - _icutNum + 1); //取得剪取UI
		auto cut_ui = (Sprite *)_csbRoot->getChildByName("cut")->getChildByName(tmp);
		cut_ui->setVisible(false); //隱藏

		_bCutting = false;
	}
	//---------------------------------------------------------------
	//Button
	_bBtnPressed = false;
	if (_retrybtn->onTouchEnded(touchLoc)) {		//重新遊戲鈕
		auto Scene = GearLevel::createScene();
		Director::getInstance()->replaceScene(TransitionFade::create(1, Scene, Color3B(255, 255, 255)));
	}
	if (_homebtn->onTouchEnded(touchLoc)) {			//回首頁鈕
		auto mainScene = MainMenu::createScene();
		Director::getInstance()->replaceScene(TransitionFade::create(1, mainScene, Color3B(255, 255, 255)));
	}

}

void GearLevel::createStaticBoundary()
{
	// 先產生 Body, 設定相關的參數

	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody; // 設定這個 Body 為 靜態的
	bodyDef.userData = NULL;
	// 在 b2World 中產生該 Body, 並傳回產生的 b2Body 物件的指標
	// 產生一次，就可以讓後面所有的 Shape 使用
	b2Body *body = _b2World->CreateBody(&bodyDef);

	_bottomBody = body;
	// 產生靜態邊界所需要的 EdgeShape
	b2EdgeShape edgeShape;
	b2FixtureDef edgeFixtureDef; // 產生 Fixture
	edgeFixtureDef.shape = &edgeShape;
	// bottom edge
	edgeShape.Set(b2Vec2(0.0f / PTM_RATIO, 0.0f / PTM_RATIO), b2Vec2(_visibleSize.width / PTM_RATIO, 0.0f / PTM_RATIO));
	body->CreateFixture(&edgeFixtureDef);

	// left edge
	edgeShape.Set(b2Vec2(0.0f / PTM_RATIO, 0.0f / PTM_RATIO), b2Vec2(0.0f / PTM_RATIO, _visibleSize.height / PTM_RATIO));
	body->CreateFixture(&edgeFixtureDef);

	// right edge
	edgeShape.Set(b2Vec2(_visibleSize.width / PTM_RATIO, 0.0f / PTM_RATIO), b2Vec2(_visibleSize.width / PTM_RATIO, _visibleSize.height / PTM_RATIO));
	body->CreateFixture(&edgeFixtureDef);

	// top edge
	edgeShape.Set(b2Vec2(0.0f / PTM_RATIO, _visibleSize.height / PTM_RATIO), b2Vec2(_visibleSize.width / PTM_RATIO, _visibleSize.height / PTM_RATIO));
	body->CreateFixture(&edgeFixtureDef);
}

#ifdef BOX2D_DEBUG
//改寫繪製方法
void GearLevel::draw(Renderer *renderer, const Mat4 &transform, uint32_t flags)
{
	Director* director = Director::getInstance();

	GL::enableVertexAttribs(cocos2d::GL::VERTEX_ATTRIB_FLAG_POSITION);
	director->pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
	_b2World->DrawDebugData();
	director->popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
}
#endif

//================================================================================
 CContactListenerLevel4:: CContactListenerLevel4()
{
	_bWin = false;
	_bCreateSpark = false;
	//_NumOfSparks = 20;	//噴發分子數

	_bDeleteJoint = false;
	_bDeleteRope1 = false;
	_bDeleteRope2 = false;

	_bMove = false; //傳送門
}

// 只要是兩個 body 的 fixtures 碰撞，就會呼叫這個函式
void  CContactListenerLevel4::BeginContact(b2Contact* contact)
{
	b2Body* BodyA = contact->GetFixtureA()->GetBody();
	b2Body* BodyB = contact->GetFixtureB()->GetBody();

	//過關偵測
	if (BodyA->GetUserData() == _EndSprite) { //A:End, B:Start
		auto shape = BodyB->GetFixtureList()->GetBody()->GetUserData();
		if (shape == _StartSprite) {
			_bWin = true;
			_bCreateSpark = true;
			_createLoc = BodyA->GetWorldCenter() + b2Vec2(0, -38.f / PTM_RATIO); //取得噴發點
		}
	}
	else if (BodyB->GetUserData() == _EndSprite) { //A:Start, B:End
		auto shape = BodyA->GetFixtureList()->GetBody()->GetUserData();
		if (shape == _StartSprite) {
			_bWin = true;
			_bCreateSpark = true;
			_createLoc = BodyB->GetWorldCenter() + b2Vec2(0, -38.f / PTM_RATIO);
		}
	}
	//----------------------------------------------------------------------
	//傳送門偵測
	if (BodyA->GetUserData() == _DoorIn) { //A:door in, B:water
		auto shape = BodyB->GetFixtureList()->GetBody()->GetUserData();
		if (shape == _StartSprite) {
			_bMove = true;
		}
	}
	else if (BodyB->GetUserData() == _DoorIn) { //A:water, B:door in
		auto shape = BodyA->GetFixtureList()->GetBody()->GetUserData();
		if (shape == _StartSprite) {
			_bMove = true;
		}
	}
	//----------------------------------------------------------------------
	// rope cut 點偵測
	auto shapeA = BodyA->GetFixtureList()->GetBody()->GetUserData();
	auto shapeB = BodyB->GetFixtureList()->GetBody()->GetUserData();
	if (shapeA == _cutNode) { // A:cutline
		if (BodyB->GetFixtureList()->GetDensity() == 0.01f) { // B:rope1
			_bDeleteJoint = true;
			_bDeleteRope1 = true;
			_lineBody = BodyA;	//傳回rope
			_ropeBody = BodyB;	//傳回line
		}
		else if (BodyB->GetFixtureList()->GetDensity() == 0.012f) { // B:rope2
			_bDeleteJoint = true;
			_bDeleteRope2 = true;
			_lineBody = BodyA;	//傳回rope
			_ropeBody = BodyB;	//傳回line
		}
		else if (BodyB->GetFixtureList()->GetDensity() == 0.013f) { // B:rope3
			_bDeleteJoint = true;
			_bDeleteRope3 = true;
			_lineBody = BodyA;	//傳回rope
			_ropeBody = BodyB;	//傳回line
		}
		else if (BodyB->GetFixtureList()->GetDensity() == 0.014f) { // B:rope4
			_bDeleteJoint = true;
			_bDeleteRope4 = true;
			_lineBody = BodyA;	//傳回rope
			_ropeBody = BodyB;	//傳回line
		}
	}
	else if (shapeB == _cutNode) { // B:cutline
		if (BodyA->GetFixtureList()->GetDensity() == 0.01f) { // A:rope1
			_bDeleteJoint = true;
			_bDeleteRope1 = true;
			_ropeBody = BodyA;	//傳回rope
			_lineBody = BodyB;	//傳回line
		}
		else if (BodyA->GetFixtureList()->GetDensity() == 0.012f) { // A:rope2
			_bDeleteJoint = true;
			_bDeleteRope2 = true;
			_ropeBody = BodyA;	//傳回rope
			_lineBody = BodyB;	//傳回line
		}
		else if (BodyA->GetFixtureList()->GetDensity() == 0.013f) { // A:rope3
			_bDeleteJoint = true;
			_bDeleteRope3 = true;
			_ropeBody = BodyA;	//傳回rope
			_lineBody = BodyB;	//傳回line
		}
		else if (BodyA->GetFixtureList()->GetDensity() == 0.014f) { // A:rope4
			_bDeleteJoint = true;
			_bDeleteRope4 = true;
			_ropeBody = BodyA;	//傳回rope
			_lineBody = BodyB;	//傳回line
		}
	}

}

//碰撞結束
void  CContactListenerLevel4::EndContact(b2Contact* contact)
{

}