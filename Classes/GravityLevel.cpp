#include "GravityLevel.h"
#include "cocostudio/CocoStudio.h"
#include "ui/CocosGUI.h"

USING_NS_CC;

#define MAX_2(X,Y) (X)>(Y) ? (X) : (Y)

#define StaticAndDynamicBodyExample 1
using namespace cocostudio::timeline;
using namespace ui;

GravityLevel::~GravityLevel()
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

Scene* GravityLevel::createScene()
{
	auto scene = Scene::create();
	auto layer = GravityLevel::create();
	scene->addChild(layer);
	return scene;
}

// on "init" you need to initialize your instance
bool GravityLevel::init()
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
	//畫筆
	_iFree = LINE_LENGTH;
	_iInUsed = 0;
	_bDrawing = false;
	//----------------------------------------------------------
	// 建立 Box2D world
	_b2World = nullptr;
	b2Vec2 Gravity = b2Vec2(0.0f, -9.8f);		//重力方向
	//b2Vec2 Gravity = b2Vec2(0.0f, 9.8f);		//重力方向
	bool AllowSleep = true;			//允許睡著
	_b2World = new b2World(Gravity);	//創建世界
	_b2World->SetAllowSleeping(AllowSleep);	//設定物件允許睡著

											// Create Scene with csb file
	_csbRoot = CSLoader::createNode("GravityLevel.csb");
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
	_penBlackBar = dynamic_cast<LoadingBar*>(_csbRoot->getChildByName("Ink_black")->getChildByName("penBlack_bar")); //墨水量條
	_penBlackBar->setPercent(100);
	_cut1 = dynamic_cast<Sprite*>(_csbRoot->getChildByName("cut")->getChildByName("cut_1"));
	_cut2 = dynamic_cast<Sprite*>(_csbRoot->getChildByName("cut")->getChildByName("cut_2"));
	_gravityUp = dynamic_cast<Sprite*>(_csbRoot->getChildByName("gravityUp"));
	_gravityUp_on = dynamic_cast<Sprite*>(_csbRoot->getChildByName("gravityUp_on"));
	_gravityDown = dynamic_cast<Sprite*>(_csbRoot->getChildByName("gravityDown"));
	_gravityDown_on = dynamic_cast<Sprite*>(_csbRoot->getChildByName("gravityDown_on"));

	//----------------------------------------------------------
	//Cutting
	_cut1->setVisible(true);
	_cut2->setVisible(true);
	//----------------------------------------------------------
	//Gravity
	_contactListener._gravityUpSprite = _gravityUp;
	_contactListener._gravityDownSprite = _gravityDown;
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
	//畫圖效果
	for (int i = 0; i < SPARKS_NUMBER; i++) {		//儲存粒子圖片
		_flare[i] = Sprite::createWithSpriteFrameName("pen.png");
		_flare[i]->setScale(0.5f);
		_flare[i]->setColor(Color3B::BLACK);
		//_flare[i]->setBlendFunc(BlendFunc::ADDITIVE);
		this->addChild(_flare[i], 5);
		_flare[i]->setVisible(false);
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
	BtnPos = dynamic_cast<Button*>(_csbRoot->getChildByName("Ink_black")->getChildByName("penBlack_icon")); //黑筆鈕
	loc = _csbRoot->getChildByName("Ink_black")->convertToWorldSpace(BtnPos->getPosition());
	BtnPos->setVisible(false);
	_blackpenbtn = C3SButton::create();
	_blackpenbtn->setButtonInfo(SELECT_BTN, "penBlack_icon.png", "penBlack_icon_on.png", "penBlack_icon_on.png", loc);
	_blackpenbtn->setScale(0.5f);
	_blackpenbtn->setVisible(true);
	this->addChild(_blackpenbtn, 3);
	_blackpenbtn->setEnable(false);	//初始用具

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
	//createStaticBoundary();		//邊界
	setupStatic();				//靜態物體
	setupSeesaw1();				//蹺蹺板1
	setupSeesaw2();				//蹺蹺板2
	setupGravityPt();			//重力轉換點
	setupRopeJoint();			//繩子

	//------------------------------------------------------------------------
	//繪圖特效鋼體
	_iflareNum = 0;
	_fflareCount = 0;
	for (int i = 0; i < SPARKS_NUMBER; i++) {
		//_flare[i]->setVisible(true);
		//產生小方塊資料
		b2BodyDef RectBodyDef;
		RectBodyDef.position.Set(_Bush_dry->getPositionX() / PTM_RATIO, _Bush_dry->getPositionY() / PTM_RATIO);
		RectBodyDef.type = b2_dynamicBody;
		RectBodyDef.userData = _flare[i];
		b2PolygonShape RectShape;
		RectShape.SetAsBox(5 / PTM_RATIO, 5 / PTM_RATIO);
		_b2flare[i] = _b2World->CreateBody(&RectBodyDef);
		b2FixtureDef RectFixtureDef;
		RectFixtureDef.shape = &RectShape;
		RectFixtureDef.density = 0.1f;
		RectFixtureDef.isSensor = true;
		b2Fixture*RectFixture = _b2flare[i]->CreateFixture(&RectFixtureDef);

		//給力量
		_b2flare[i]->ApplyForce(b2Vec2(rand() % 26 - 13, 10 + rand() % 10), b2Vec2(_Bush_dry->getPositionX() / PTM_RATIO, _Bush_dry->getPositionY() / PTM_RATIO), true);
	}


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
	_listener1->onTouchBegan = CC_CALLBACK_2(GravityLevel::onTouchBegan, this);		//加入觸碰開始事件
	_listener1->onTouchMoved = CC_CALLBACK_2(GravityLevel::onTouchMoved, this);		//加入觸碰移動事件
	_listener1->onTouchEnded = CC_CALLBACK_2(GravityLevel::onTouchEnded, this);		//加入觸碰離開事件

	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(_listener1, this);	//加入剛創建的事件聆聽器
	this->schedule(CC_SCHEDULE_SELECTOR(GravityLevel::doStep));

	return true;
}

void GravityLevel::setupStatic()
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

	for (size_t i = 1; i <= 4; i++)
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

void GravityLevel::setStartEndpoint()
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

//蹺蹺板
void GravityLevel::setupSeesaw1()
{
	// 取得並設定 dot 圖示為【靜態物體】
	auto dotSprite = _csbRoot->getChildByName("dot_1");
	Point loc = dotSprite->getPosition();
	Size size = dotSprite->getContentSize();
	auto scale = dotSprite->getScale();
	b2BodyDef staticBodyDef;
	staticBodyDef.type = b2_staticBody;
	staticBodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	staticBodyDef.userData = dotSprite;
	b2Body* dotBody = _b2World->CreateBody(&staticBodyDef);

	b2CircleShape circleShape;
	circleShape.m_radius = size.width*0.5f*scale / PTM_RATIO;

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &circleShape;
	fixtureDef.isSensor = true; // 設定為 Sensor
	dotBody->CreateFixture(&fixtureDef);

	//----------------------------------------------------
	// 取得並設定 seesaw01 為【動態物體】
	auto seesawSprite = _csbRoot->getChildByName("seesaw01_1");
	loc = seesawSprite->getPosition();
	size = seesawSprite->getContentSize();
	scale = seesawSprite->getScale();

	b2BodyDef dynamicBodyDef;
	dynamicBodyDef.type = b2_dynamicBody;
	dynamicBodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	dynamicBodyDef.userData = seesawSprite;
	b2Body* seesawBody = _b2World->CreateBody(&dynamicBodyDef);

	b2PolygonShape boxShape;
	boxShape.SetAsBox((size.width*0.5f*scale - 2.f) / PTM_RATIO, (size.height*0.5f*scale - 2.f) / PTM_RATIO);

	b2FixtureDef fixtureDef2;
	fixtureDef2.shape = &boxShape;
	fixtureDef2.density = 5.0f;
	seesawBody->CreateFixture(&fixtureDef2);
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
	boxShape2.SetAsBox((size.width*0.5f*scale - 2.f) / PTM_RATIO, (size.height*0.5f*scale - 2.f) / PTM_RATIO);

	b2FixtureDef fixtureDef3;
	fixtureDef3.shape = &boxShape2;
	fixtureDef3.density = 5.0f;
	seesawBody2->CreateFixture(&fixtureDef3);
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
	boxShape3.SetAsBox((size.width*0.5f*scale - 2.f) / PTM_RATIO, (size.height*0.5f*scale - 2.f) / PTM_RATIO);

	b2FixtureDef fixtureDef4;
	fixtureDef4.shape = &boxShape3;
	fixtureDef4.density = 5.0f;
	seesawBody3->CreateFixture(&fixtureDef4);

	//----------------------------------------------------
	// 建立 焊接Joint 連結
	b2WeldJointDef WeldJointDef;
	WeldJointDef.Initialize(seesawBody, seesawBody2, seesawBody->GetPosition() + b2Vec2(-270 / PTM_RATIO, 0));
	_b2World->CreateJoint(&WeldJointDef); // 使用預設值焊接

	// 建立 焊接Joint 連結
	b2WeldJointDef JointDef;
	JointDef.Initialize(seesawBody, seesawBody3, seesawBody->GetPosition() + b2Vec2(270 / PTM_RATIO, 0));
	_b2World->CreateJoint(&JointDef); // 使用預設值焊接

	// 建立 旋轉Joint 連結
	b2RevoluteJointDef seesawJoint;
	seesawJoint.bodyA = dotBody;
	seesawJoint.localAnchorA.Set(0, 0);
	seesawJoint.bodyB = seesawBody;
	seesawJoint.localAnchorB.Set(0, 0);
	_b2World->CreateJoint(&seesawJoint);
}
void GravityLevel::setupSeesaw2()
{
	// 取得並設定 dot 圖示為【靜態物體】
	auto dotSprite = _csbRoot->getChildByName("dot_2");
	Point loc = dotSprite->getPosition();
	Size size = dotSprite->getContentSize();
	auto scale = dotSprite->getScale();
	b2BodyDef staticBodyDef;
	staticBodyDef.type = b2_staticBody;
	staticBodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	staticBodyDef.userData = dotSprite;
	b2Body* dotBody = _b2World->CreateBody(&staticBodyDef);

	b2CircleShape circleShape;
	circleShape.m_radius = size.width*0.5f*scale / PTM_RATIO;

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &circleShape;
	fixtureDef.isSensor = true; // 設定為 Sensor
	dotBody->CreateFixture(&fixtureDef);

	//----------------------------------------------------
	// 取得並設定 seesaw01 為【動態物體】
	auto seesawSprite = _csbRoot->getChildByName("seesaw02_1");
	loc = seesawSprite->getPosition();
	size = seesawSprite->getContentSize();
	scale = seesawSprite->getScale();

	b2BodyDef dynamicBodyDef;
	dynamicBodyDef.type = b2_dynamicBody;
	dynamicBodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	dynamicBodyDef.userData = seesawSprite;
	b2Body* seesawBody = _b2World->CreateBody(&dynamicBodyDef);

	b2PolygonShape boxShape;
	boxShape.SetAsBox((size.width*0.5f*scale - 2.f) / PTM_RATIO, (size.height*0.5f*scale - 8.f) / PTM_RATIO);

	b2FixtureDef fixtureDef2;
	fixtureDef2.shape = &boxShape;
	fixtureDef2.density = 5.0f;
	seesawBody->CreateFixture(&fixtureDef2);
	//------
	seesawSprite = _csbRoot->getChildByName("seesaw02_2");
	loc = seesawSprite->getPosition();
	size = seesawSprite->getContentSize();
	scale = seesawSprite->getScale();

	b2BodyDef dynamicBodyDef2;
	dynamicBodyDef2.type = b2_dynamicBody;
	dynamicBodyDef2.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	dynamicBodyDef2.userData = seesawSprite;
	b2Body* seesawBody2 = _b2World->CreateBody(&dynamicBodyDef2);

	b2PolygonShape boxShape2;
	boxShape2.SetAsBox((size.width*0.5f*scale - 2.f) / PTM_RATIO, (size.height*0.5f*scale - 2.f) / PTM_RATIO);

	b2FixtureDef fixtureDef3;
	fixtureDef3.shape = &boxShape2;
	fixtureDef3.density = 5.0f;
	seesawBody2->CreateFixture(&fixtureDef3);
	//------
	seesawSprite = _csbRoot->getChildByName("seesaw02_3");
	loc = seesawSprite->getPosition();
	size = seesawSprite->getContentSize();
	scale = seesawSprite->getScale();

	b2BodyDef dynamicBodyDef3;
	dynamicBodyDef3.type = b2_dynamicBody;
	dynamicBodyDef3.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	dynamicBodyDef3.userData = seesawSprite;
	b2Body* seesawBody3 = _b2World->CreateBody(&dynamicBodyDef3);

	b2PolygonShape boxShape3;
	boxShape3.SetAsBox((size.width*0.5f*scale - 2.f) / PTM_RATIO, (size.height*0.5f*scale - 2.f) / PTM_RATIO);

	b2FixtureDef fixtureDef4;
	fixtureDef4.shape = &boxShape3;
	fixtureDef4.density = 5.0f;
	seesawBody3->CreateFixture(&fixtureDef4);

	//----------------------------------------------------
	// 建立 焊接Joint 連結
	b2WeldJointDef WeldJointDef;
	WeldJointDef.Initialize(seesawBody, seesawBody2, seesawBody->GetPosition() + b2Vec2(-270 / PTM_RATIO, 0));
	_b2World->CreateJoint(&WeldJointDef); // 使用預設值焊接

										  // 建立 焊接Joint 連結
	b2WeldJointDef JointDef;
	JointDef.Initialize(seesawBody, seesawBody3, seesawBody->GetPosition() + b2Vec2(270 / PTM_RATIO, 0));
	_b2World->CreateJoint(&JointDef); // 使用預設值焊接

									  // 建立 旋轉Joint 連結
	b2RevoluteJointDef seesawJoint;
	seesawJoint.bodyA = dotBody;
	seesawJoint.localAnchorA.Set(0, 0);
	seesawJoint.bodyB = seesawBody;
	seesawJoint.localAnchorB.Set(0, 0);
	_b2World->CreateJoint(&seesawJoint);
}

void GravityLevel::setupGravityPt()
{
	// 取得並設定 gravityUp 圖示為【靜態物體】
	Point loc = _gravityUp->getPosition();
	Size size = _gravityUp->getContentSize();
	auto scale = _gravityUp->getScale();
	b2BodyDef staticBodyDef;
	staticBodyDef.type = b2_staticBody;
	staticBodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	staticBodyDef.userData = _gravityUp;
	b2Body* staticBody = _b2World->CreateBody(&staticBodyDef);

	b2PolygonShape boxShape;
	boxShape.SetAsBox(size.width*0.5f*scale / PTM_RATIO, size.height*0.5f*scale / PTM_RATIO);

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &boxShape;
	fixtureDef.isSensor = true; // 設定為 Sensor
	staticBody->CreateFixture(&fixtureDef);

	//-----------------------------------------------
	// 取得並設定 gravityDown 圖示為【靜態物體】
	loc = _gravityDown->getPosition();
	size = _gravityDown->getContentSize();
	scale = _gravityDown->getScale();
	b2BodyDef staticBodyDef2;
	staticBodyDef2.type = b2_staticBody;
	staticBodyDef2.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	staticBodyDef2.userData = _gravityDown;
	b2Body* staticBody2 = _b2World->CreateBody(&staticBodyDef2);

	b2PolygonShape boxShape2;
	boxShape2.SetAsBox(size.width*0.5f*scale / PTM_RATIO, size.height*0.5f*scale / PTM_RATIO);

	b2FixtureDef fixtureDef2;
	fixtureDef2.shape = &boxShape2;
	fixtureDef2.isSensor = true; // 設定為 Sensor
	staticBody2->CreateFixture(&fixtureDef2);
}

void GravityLevel::setupRopeJoint()
{
	// 取得並設定 stickRope 圖示為【靜態物體】
	auto frameSprite = _csbRoot->getChildByName("stickRope");
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
	b2Body* cactusBody = _b2World->CreateBody(&bodyDef);
	b2CircleShape circleShape;
	circleShape.m_radius = (sizeTail.width - 4)*0.5f*scaleTail*0.85f / PTM_RATIO; //0.85 : 向內修正(縮小)
	fixtureDef.shape = &circleShape;
	fixtureDef.density = 5.f;  fixtureDef.friction = 0.25f; fixtureDef.restitution = 0.25f;
	cactusBody->CreateFixture(&fixtureDef);


	//產生繩子關節
	b2RopeJointDef JointDef;
	JointDef.bodyA = ropeHeadBody;
	JointDef.bodyB = cactusBody;
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
	revJoint.bodyB = cactusBody;
	revJoint.localAnchorB.Set(0, 1.4f);
	_b2World->CreateJoint(&revJoint);
}

void GravityLevel::DrawLine(cocos2d::Point prePt, cocos2d::Point Pt, cocos2d::Color4F color, int num)
{
	//畫出線段
	_draw[num] = DrawNode::create();
	_draw[num]->drawLine(prePt, Pt, color);
	_newNode->addChild(_draw[num], 5);
}

void GravityLevel::DrawCuttingLine(cocos2d::Point firstPt, cocos2d::Point Pt, cocos2d::Color4F color, int num)
{
	//畫出剪取線段
	_drawCut[num] = DrawNode::create();
	_drawCut[num]->drawLine(firstPt, Pt, color);
	//_drawCut[num]->clear();
	_cutNode->addChild(_drawCut[num], 5);
}

void GravityLevel::doStep(float dt)
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
		auto Scene = CarLevel::createScene();
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
	// for gravity
	if (_contactListener._bGravityUp) {		//重力向上
		_contactListener._bGravityUp = false;
		_gravityUp_on->setVisible(true);
		_gravityDown_on->setVisible(false);
		_b2World->SetGravity(b2Vec2(0, 9.8f));
	}
	if (_contactListener._bGravityDown) {	//重力向下
		_contactListener._bGravityDown = false;
		_gravityUp_on->setVisible(false);
		_gravityDown_on->setVisible(true);
		_b2World->SetGravity(b2Vec2(0, -9.8f));
	}
	//----------------------------------------------------------------------------
	// for cutting
	if (_contactListener._bDeleteJoint) { //delete joint
		_contactListener._bDeleteJoint = false;
		for (b2Joint* j = _b2World->GetJointList(); j; ) //若rope碰撞 移除其身上的joint
		{
			b2Joint* jointToDestroy = j;
			j = j->GetNext();
			if (jointToDestroy->GetBodyA() == _contactListener._ropeBody
				/*|| jointToDestroy->GetBodyB() == _contactListener._ropeBody*/) _b2World->DestroyJoint(jointToDestroy);
			if (jointToDestroy == _MainRopeJoint)_b2World->DestroyJoint(_MainRopeJoint);	//切斷主繩
		}
		for (b2Body* b = _b2World->GetBodyList(); b; ) //若rope碰撞 移除線段身上的body(避免重複碰撞) -> 一次只能切一條
		{
			b2Body* bodyToDestroy = b;
			b = b->GetNext();
			if(bodyToDestroy == _contactListener._lineBody)_b2World->DestroyBody(_contactListener._lineBody);	//移除線段Body
		}
	}
	// ---------------------------------------------------------------------------
	// 產生繪圖效果
	if (_bDrawing) {	//畫圖中
		if (!_blackpenbtn->getEnable() && _iFree > 0) {			//黑筆使用中
			_fflareCount += dt;
			if (_fflareCount > 0.1f) {
				_fflareCount -= 0.1f;
				//for (int i = _iflareNum; i < _iflareNum + 4; i++) {
				_flare[_iflareNum]->setColor(Color3B::BLACK);
				_flare[_iflareNum]->setVisible(true);
				_b2flare[_iflareNum]->SetType(b2_staticBody);
				_b2flare[_iflareNum]->SetTransform(b2Vec2(_tp.x / PTM_RATIO, _tp.y / PTM_RATIO), _b2flare[_iflareNum]->GetAngle());
				_b2flare[_iflareNum]->SetType(b2_dynamicBody);
				_b2flare[_iflareNum]->ApplyForce(b2Vec2(rand() % 5 - 3, 1 + rand() % 3),
					b2Vec2(_tp.x / PTM_RATIO, _tp.y / PTM_RATIO), true);//給力量
																		//}
																		//_iflareNum += 4;///////////////////	
				_iflareNum++;
				if (_iflareNum == SPARKS_NUMBER)_iflareNum = 0;
			}
		}
	}
	else {			//未畫圖
		_fflareCount = 0;	//重置計時
		_iflareNum = 0;		//重置計數
		for (int i = 0; i < SPARKS_NUMBER; i++) {	//定位回歸
			_flare[i]->setVisible(false);
			_b2flare[i]->SetType(b2_staticBody);
			_b2flare[i]->SetTransform(b2Vec2(_Bush_dry->getPositionX() / PTM_RATIO, _Bush_dry->getPositionY() / PTM_RATIO), _b2flare[i]->GetAngle());
			_b2flare[i]->SetType(b2_dynamicBody);
		}
	}
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

bool GravityLevel::onTouchBegan(cocos2d::Touch *pTouch, cocos2d::Event *pEvent)//觸碰開始事件
{
	Point touchLoc = pTouch->getLocation();
	_tp = touchLoc;
	_tp_start = touchLoc;
	//-------------------------------------------------------------------
	//For button
	if (_retrybtn->onTouchBegan(touchLoc) || _homebtn->onTouchBegan(touchLoc)
		|| _blackpenbtn->onTouchBegan(touchLoc) || _cutbtn->onTouchBegan(touchLoc)) _bBtnPressed = true;

	//-------------------------------------------------------------------
	
	//For drawing
	if (!_blackpenbtn->getEnable()) {	//按鈕點選才可畫圖
		if (!_bBtnPressed)_bDrawing = true;	//不在按鈕上才可畫圖
		if (_bDrawing && _iFree > 0) {
			_newNode = Node::create(); //新增此筆畫控制節點
			this->addChild(_newNode, 5);
			_contactListener._newNode = _newNode; //傳入Box2d world
			_istartPt = _iInUsed;	//紀錄第一點
			_pt[_iInUsed] = touchLoc;
			_b2vec[_iInUsed].x = touchLoc.x / PTM_RATIO;
			_b2vec[_iInUsed].y = touchLoc.y / PTM_RATIO;
			_iInUsed++; _iFree--;
		}
	}

	if (!_cutbtn->getEnable()) {	//剪刀
		if (!_bBtnPressed)_bCutting = true;	//不在按鈕上才可畫圖
		if (_bCutting && _icutNum < CUTTING_NUM) {
			_cutNode = Node::create(); //新增此筆畫控制節點
			this->addChild(_cutNode, 5);
			_contactListener._cutNode = _cutNode; //傳入Box2d world
			_cutPt[0] = _cutPt[1] = touchLoc;	//第一點
		}
	}

	return true;
}

void  GravityLevel::onTouchMoved(cocos2d::Touch *pTouch, cocos2d::Event *pEvent) //觸碰移動事件
{
	Point touchLoc = pTouch->getLocation();
	_tp = touchLoc;
	//---------------------------------------------------------------------
	//For drawing
	if (!_blackpenbtn->getEnable()) {	//按鈕點選才可畫圖
		if (_bDrawing && _iFree > 0) {
			DrawLine(_pt[_iInUsed - 1], touchLoc, Color4F::BLACK, _iInUsed - 1); //畫線函式
			_b2vec[_iInUsed].x = touchLoc.x / PTM_RATIO;
			_b2vec[_iInUsed].y = touchLoc.y / PTM_RATIO;
			_pt[_iInUsed] = touchLoc;
			_iInUsed++; _iFree--;
			_penBlackBar->setPercent((float)_iFree / (float)LINE_LENGTH * 100.f); //墨水量
		}
	}
	//-------------------------------------------------------------------
	//For cutting
	if (!_cutbtn->getEnable()) {	//按鈕點選才可畫圖
		if (_bCutting && _icutNum < CUTTING_NUM) {
			_cutPt[1] = touchLoc;
		}
	}
	//-------------------------------------------------------------------
	//For button
	_retrybtn->onTouchMoved(touchLoc);
	_homebtn->onTouchMoved(touchLoc);
	_blackpenbtn->onTouchMoved(touchLoc);
	_cutbtn->onTouchMoved(touchLoc);

}

void  GravityLevel::onTouchEnded(cocos2d::Touch *pTouch, cocos2d::Event *pEvent) //觸碰結束事件 
{
	Point touchLoc = pTouch->getLocation();
	_tp = touchLoc;
	//----------------------------------------------------------------
	if (!_blackpenbtn->getEnable()) {	//按鈕點選才可畫圖
		if (_bDrawing) {

			//建立 手繪【動態物體】
			Point loc = _newNode->getPosition();
			Size size = _newNode->getContentSize();
			b2BodyDef bodyDef;				//body只產生一次
			bodyDef.type = b2_dynamicBody;
			bodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
			bodyDef.userData = _newNode;
			_b2Linebody = _b2World->CreateBody(&bodyDef);

			//手繪【線條】----------------------------------------------

			for (int i = _istartPt; _draw[i] != nullptr && i < LINE_LENGTH - 1; i++) { //取得並設定 線段 為【動態物體】
				b2EdgeShape edgeShape;
				b2FixtureDef edgeFixtureDef; // 產生 Fixture
				edgeFixtureDef.shape = &edgeShape;
				edgeShape.Set(b2Vec2(_pt[i].x / PTM_RATIO, _pt[i].y / PTM_RATIO), b2Vec2(_pt[i + 1].x / PTM_RATIO, _pt[i + 1].y / PTM_RATIO)); //畫圖線段
				_b2Linebody->CreateFixture(&edgeFixtureDef);
			}
			////手繪【多邊形】--------------------------------------------
			//_b2polyVec[0] = _b2vec[_istartPt]; //三角形頂點
			//for (int i = _istartPt + 1; _draw[i] != nullptr && i < LINE_LENGTH - 1; i++) { //取得並設定 線段 為【動態物體】
			//	b2PolygonShape polyShape;
			//	b2FixtureDef fixtureDef; // 產生 Fixture
			//	fixtureDef.shape = &polyShape;

			//	_b2polyVec[1] = _b2vec[i]; //每一線段與起點產生三角形
			//	_b2polyVec[2] = _b2vec[i + 1];

			//	polyShape.Set(_b2polyVec, 3); //polygon中的小三角形
			//	_b2Linebody->CreateFixture(&fixtureDef); //加入主body
			//}

			_bDrawing = false;
		}
	}
	//-------------------------------------------------------------------
	//For cutting
	if (!_cutbtn->getEnable()) {	//按鈕點選才可畫圖
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
			if (_icutNum == 1) _cut2->setVisible(false);
			if (_icutNum == 2) _cut1->setVisible(false);

			_bCutting = false;
		}
	}
	//---------------------------------------------------------------
	//Button
	_bBtnPressed = false;
	if (_retrybtn->onTouchEnded(touchLoc)) {		//重新遊戲鈕
		auto Scene = GravityLevel::createScene();
		Director::getInstance()->replaceScene(TransitionFade::create(1, Scene, Color3B(255, 255, 255)));
	}
	if (_homebtn->onTouchEnded(touchLoc)) {			//回首頁鈕
		auto mainScene = MainMenu::createScene();
		Director::getInstance()->replaceScene(TransitionFade::create(1, mainScene, Color3B(255, 255, 255)));
	}
	if (_blackpenbtn->onTouchEnded(touchLoc)) {		//黑筆鈕
		_cutbtn->setEnable(true);
	}
	if (_cutbtn->onTouchEnded(touchLoc)) {			//剪刀鈕
		_blackpenbtn->setEnable(true);
	}
}

void GravityLevel::createStaticBoundary()
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
void GravityLevel::draw(Renderer *renderer, const Mat4 &transform, uint32_t flags)
{
	Director* director = Director::getInstance();

	GL::enableVertexAttribs(cocos2d::GL::VERTEX_ATTRIB_FLAG_POSITION);
	director->pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
	_b2World->DrawDebugData();
	director->popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
}
#endif

//================================================================================
 CContactListenerLevel3:: CContactListenerLevel3()
{
	_bWin = false;
	_bCreateSpark = false;
	//_NumOfSparks = 20;	//噴發分子數

	_bGravityUp = false;
	_bGravityDown = false;

	_bDeleteJoint = false;
}

// 只要是兩個 body 的 fixtures 碰撞，就會呼叫這個函式
void  CContactListenerLevel3::BeginContact(b2Contact* contact)
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
	//改變 Gravity UP 點偵測
	if (BodyA->GetUserData() == _gravityUpSprite) { //A:gravityUp, B:water
		auto shape = BodyB->GetFixtureList()->GetBody()->GetUserData();
		if (shape == _StartSprite) {
			_bGravityUp = true;
		}
	}
	else if (BodyB->GetUserData() == _gravityUpSprite) { //A:water, B:gravityUp
		auto shape = BodyA->GetFixtureList()->GetBody()->GetUserData();
		if (shape == _StartSprite) {
			_bGravityUp = true;
		}
	}
	//----------------------------------------------------------------------
	//改變 Gravity DOWN 點偵測
	if (BodyA->GetUserData() == _gravityDownSprite) { //A:gravityDown, B:water
		auto shape = BodyB->GetFixtureList()->GetBody()->GetUserData();
		if (shape == _StartSprite) {
			_bGravityDown = true;
		}
	}
	else if (BodyB->GetUserData() == _gravityDownSprite) { //A:water, B:gravityDown
		auto shape = BodyA->GetFixtureList()->GetBody()->GetUserData();
		if (shape == _StartSprite) {
			_bGravityDown = true;
		}
	}
	//----------------------------------------------------------------------
	// cut 點偵測
	if (BodyA->GetFixtureList()->GetDensity() == 0.01f) { //A:rope, B:cutline
		auto shape = BodyB->GetFixtureList()->GetBody()->GetUserData();
		if (shape == _cutNode) {
			_bDeleteJoint = true;
			_ropeBody = BodyA;	//傳回rope
			_lineBody = BodyB;	//傳回line
		}
	}
	else if (BodyB->GetFixtureList()->GetDensity() == 0.01f) { //A:cutline, B:rope
		auto shape = BodyA->GetFixtureList()->GetBody()->GetUserData();
		if (shape == _cutNode) {
			_bDeleteJoint = true;
			_ropeBody = BodyB;	//傳回rope
			_lineBody = BodyA;	//傳回line
		}
	}

}

//碰撞結束
void  CContactListenerLevel3::EndContact(b2Contact* contact)
{

}