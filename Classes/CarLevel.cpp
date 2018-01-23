#include "CarLevel.h"
#include "cocostudio/CocoStudio.h"
#include "ui/CocosGUI.h"

USING_NS_CC;

#define MAX_2(X,Y) (X)>(Y) ? (X) : (Y)

#define StaticAndDynamicBodyExample 1
using namespace cocostudio::timeline;
using namespace ui;

CarLevel::~CarLevel()
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

Scene* CarLevel::createScene()
{
	auto scene = Scene::create();
	auto layer = CarLevel::create();
	scene->addChild(layer);
	return scene;
}

// on "init" you need to initialize your instance
bool CarLevel::init()
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
	_iFree = _iFree_red = _iFree_blue = LINE_LENGTH;
	_iInUsed = _iInUsed_red = _iInUsed_blue = 0;
	_bDrawing = false;
	//----------------------------------------------------------
	// 建立 Box2D world
	_b2World = nullptr;
	b2Vec2 Gravity = b2Vec2(0.0f, -9.8f);		//重力方向
	bool AllowSleep = true;			//允許睡著
	_b2World = new b2World(Gravity);	//創建世界
	_b2World->SetAllowSleeping(AllowSleep);	//設定物件允許睡著

											// Create Scene with csb file
	_csbRoot = CSLoader::createNode("CarLevel.csb");
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
	_penRedBar = dynamic_cast<LoadingBar*>(_csbRoot->getChildByName("Ink_red")->getChildByName("penRed_bar")); //墨水量條
	_penRedBar->setPercent(100);
	_penBlueBar = dynamic_cast<LoadingBar*>(_csbRoot->getChildByName("Ink_blue")->getChildByName("penBlue_bar")); //墨水量條
	_penBlueBar->setPercent(100);
	_cut1 = dynamic_cast<Sprite*>(_csbRoot->getChildByName("cut")->getChildByName("cut_1"));
	//_cut2 = dynamic_cast<Sprite*>(_csbRoot->getChildByName("cut")->getChildByName("cut_2"));

	_triSensor = dynamic_cast<Sprite*>(_csbRoot->getChildByName("tri_line"));	//感應點
	_rectSensor = dynamic_cast<Sprite*>(_csbRoot->getChildByName("rect_line"));
	_jeepSensor = dynamic_cast<Sprite*>(_csbRoot->getChildByName("jeep_line"));
	_blackWood0 = dynamic_cast<Sprite*>(_csbRoot->getChildByName("wall_10"));	//物理斜黑木頭
	_blackWood1 = dynamic_cast<Sprite*>(_csbRoot->getChildByName("wall_10_0"));	//斜黑木頭
	_blackWood2 = dynamic_cast<Sprite*>(_csbRoot->getChildByName("wall_11"));	//斜黑木頭
	_blackWood1->setVisible(false); //隱藏
	_blackWood2->setVisible(false);
	_redWood0 = dynamic_cast<Sprite*>(_csbRoot->getChildByName("wall_12"));		//物理斜紅木頭
	_redWood1 = dynamic_cast<Sprite*>(_csbRoot->getChildByName("wall_12_0"));	//斜紅木頭
	_redWood2 = dynamic_cast<Sprite*>(_csbRoot->getChildByName("wall_13"));		//紅木頭
	_redWood1->setVisible(false); //隱藏
	_redWood2->setVisible(false);
	_blueWood = dynamic_cast<Sprite*>(_csbRoot->getChildByName("wall_9"));	//藍木頭

	_jeepSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("jeep"));	//車子
	_contactListener._jeepSprite = _jeepSprite;	//傳入Box2d world

	//----------------------------------------------------------
	//Sensor
	_contactListener._triSensor = _triSensor;	//傳入Box2d世界
	_contactListener._rectSensor = _rectSensor;
	_contactListener._jeepSensor = _jeepSensor;

	//----------------------------------------------------------
	//Cutting
	_cut1->setVisible(true);
	//_cut2->setVisible(true);
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

	//Switch btn
	auto BtnPos = dynamic_cast<Button*>(_csbRoot->getChildByName("btn_shape")); //形狀鈕
	Point loc = BtnPos->getPosition();
	BtnPos->setVisible(false);
	_shapebtn = CSwitchButton::create();
	_shapebtn->setButtonInfo("btn_shape_0.png", "btn_shape_1.png", "btn_shape_2.png", loc);
	_shapebtn->setScale(0.4f);
	this->addChild(_shapebtn, 2);

	//Type 1
	BtnPos = dynamic_cast<Button*>(_csbRoot->getChildByName("btn_retry")); //重新遊戲鈕
	loc = BtnPos->getPosition();
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
	_blackpenbtn->setEnable(false);	//初始用具 設為使用中

	BtnPos = dynamic_cast<Button*>(_csbRoot->getChildByName("Ink_red")->getChildByName("penRed_icon")); //紅筆鈕
	loc = _csbRoot->getChildByName("Ink_red")->convertToWorldSpace(BtnPos->getPosition());
	BtnPos->setVisible(false);
	_redpenbtn = C3SButton::create();
	_redpenbtn->setButtonInfo(SELECT_BTN, "penRed_icon.png", "penRed_icon_on.png", "penRed_icon_on.png", loc);
	_redpenbtn->setScale(0.5f);
	_redpenbtn->setVisible(true);
	this->addChild(_redpenbtn, 3);

	BtnPos = dynamic_cast<Button*>(_csbRoot->getChildByName("Ink_blue")->getChildByName("penBlue_icon")); //藍筆鈕
	loc = _csbRoot->getChildByName("Ink_blue")->convertToWorldSpace(BtnPos->getPosition());
	BtnPos->setVisible(false);
	_bluepenbtn = C3SButton::create();
	_bluepenbtn->setButtonInfo(SELECT_BTN, "penBlue_icon.png", "penBlue_icon_on.png", "penBlue_icon_on.png", loc);
	_bluepenbtn->setScale(0.5f);
	_bluepenbtn->setVisible(true);
	this->addChild(_bluepenbtn, 3);

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
	createStaticBoundary();		//邊界
	setupStatic();				//靜態物體
	setupJeep();				//車子
	setupRopeJoint();			//繩子
	setupSensorPt();			//感應點

	//------------------------------------------------------------------------
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
	_listener1->onTouchBegan = CC_CALLBACK_2(CarLevel::onTouchBegan, this);		//加入觸碰開始事件
	_listener1->onTouchMoved = CC_CALLBACK_2(CarLevel::onTouchMoved, this);		//加入觸碰移動事件
	_listener1->onTouchEnded = CC_CALLBACK_2(CarLevel::onTouchEnded, this);		//加入觸碰離開事件

	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(_listener1, this);	//加入剛創建的事件聆聽器
	this->schedule(CC_SCHEDULE_SELECTOR(CarLevel::doStep));

	return true;
}

void CarLevel::setupStatic()
{
	// 讀取所有 wall_ 開頭的圖示 當成是靜態物體
	char tmp[20] = "";

	for (size_t i = 1; i <= 13; i++)
	{
		sprintf(tmp, "wall_%d", i);
		auto rectSprite = (Sprite *)_csbRoot->getChildByName(tmp);

		Size ts = rectSprite->getContentSize();
		Point loc = rectSprite->getPosition();
		float angle = rectSprite->getRotation();
		float scaleX = rectSprite->getScaleX();	// 水平的線段圖示假設都只有對 X 軸放大
		float scaleY = rectSprite->getScaleY();	// 水平的線段圖示假設都只有對 X 軸放大

		//------------------------------------------
		b2BodyDef staticBodyDef;
		staticBodyDef.type = b2_staticBody;
		staticBodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
		staticBodyDef.userData = rectSprite;
		b2Body* staticBody = _b2World->CreateBody(&staticBodyDef);
		//------------------------------------------

												// rectShape 的四個端點, 0 右上、 1 左上、 2 左下 3 右下
		Point lep[4], wep[4];
		lep[0].x = (ts.width - 4) / 2.0f;;  lep[0].y = (ts.height - 4) / 2.0f;
		lep[1].x = -(ts.width - 4) / 2.0f;; lep[1].y = (ts.height - 4) / 2.0f;
		lep[2].x = -(ts.width - 4) / 2.0f;; lep[2].y = -(ts.height - 4) / 2.0f;
		lep[3].x = (ts.width - 4) / 2.0f;;  lep[3].y = -(ts.height - 4) / 2.0f;

		// 所有的線段圖示都是是本身的中心點為 (0,0)，
		// 根據縮放、旋轉產生所需要的矩陣
		// 根據寬度計算出兩個端點的座標，然後呈上開矩陣
		// 然後進行旋轉，
		// Step1: 先CHECK 有無旋轉，有旋轉則進行端點的計算
		cocos2d::Mat4 modelMatrix, rotMatrix;
		modelMatrix.m[0] = scaleX;  // 先設定 X 軸的縮放
		modelMatrix.m[5] = scaleY;  // 先設定 Y 軸的縮放
		cocos2d::Mat4::createRotationZ(angle*M_PI / 180.0f, &rotMatrix);
		modelMatrix.multiply(rotMatrix);
		//modelMatrix.m[3] = pntLoc.x + loc.x; //設定 Translation，自己的加上父親的
		//modelMatrix.m[7] = pntLoc.y + loc.y; //設定 Translation，自己的加上父親的
		for (size_t j = 0; j < 4; j++)
		{
			wep[j].x = lep[j].x * modelMatrix.m[0] + lep[j].y * modelMatrix.m[1] + modelMatrix.m[3];
			wep[j].y = lep[j].x * modelMatrix.m[4] + lep[j].y * modelMatrix.m[5] + modelMatrix.m[7];
		}
		b2Vec2 vecs[] = {
			b2Vec2(wep[0].x / PTM_RATIO, wep[0].y / PTM_RATIO),
			b2Vec2(wep[1].x / PTM_RATIO, wep[1].y / PTM_RATIO),
			b2Vec2(wep[2].x / PTM_RATIO, wep[2].y / PTM_RATIO),
			b2Vec2(wep[3].x / PTM_RATIO, wep[3].y / PTM_RATIO) };


		// 產生靜態邊界所需要的 boxShape
		b2PolygonShape boxShape;
		boxShape.Set(vecs, 4);

		b2FixtureDef fixtureDef;
		fixtureDef.shape = &boxShape;
		staticBody->CreateFixture(&fixtureDef);

		//取得物理木頭---------------------------------------------
		if (i == 10 || i == 11 || i == 12 || i == 13) {
			auto fixtrue = staticBody->GetFixtureList();
			fixtrue->SetSensor(true);	//設為感應器(設為消失)
		}
		if (i == 9) _b2BlueBody = staticBody;
		if (i == 10) _b2BlackBody1 = staticBody;
		if (i == 11) _b2BlackBody2 = staticBody;
		if (i == 12) _b2RedBody1 = staticBody;
		if (i == 13) _b2RedBody2 = staticBody;
	}
}

void CarLevel::setStartEndpoint()
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

void CarLevel::setupRopeJoint()
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

	//取得 jeep 資訊
	Point locTail = _jeepSprite->getPosition();
	Size sizeTail = _jeepSprite->getContentSize();
	auto scaleTail = _jeepSprite->getScale();

	//產生繩子關節
	b2RopeJointDef JointDef;
	JointDef.bodyA = ropeHeadBody;
	JointDef.bodyB = _CarBody;
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
	revJoint.bodyB = _CarBody;	//與車子連結
	revJoint.localAnchorB.Set(-67.f / PTM_RATIO, 0);
	_b2World->CreateJoint(&revJoint);
}

void CarLevel::setupJeep()
{
	// 取得並設定 jeep 圖示為【動態物體】
	Point loc = _jeepSprite->getPosition();
	Size size = _jeepSprite->getContentSize();
	auto scale = _jeepSprite->getScale();
	b2BodyDef staticBodyDef;
	staticBodyDef.type = b2_dynamicBody;
	staticBodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	staticBodyDef.userData = _jeepSprite;
	_CarBody = _b2World->CreateBody(&staticBodyDef);

	b2PolygonShape boxShape;
	boxShape.SetAsBox(size.width*0.5f*scale / PTM_RATIO, size.height*0.5f*scale / PTM_RATIO);

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &boxShape;
	fixtureDef.density = 5.0f;
	_CarBody->CreateFixture(&fixtureDef);

	//----------------------------------------------------
	// 取得並設定 wheel1 為【動態物體】
	auto wheelSprite = _csbRoot->getChildByName("wheel_1");
	loc = wheelSprite->getPosition();
	size = wheelSprite->getContentSize();
	scale = wheelSprite->getScale();

	b2BodyDef wheel1BodyDef;
	wheel1BodyDef.type = b2_dynamicBody;
	wheel1BodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	wheel1BodyDef.userData = wheelSprite;
	_wheel1Body = _b2World->CreateBody(&wheel1BodyDef);

	b2CircleShape circleShape;
	circleShape.m_radius = size.width*0.5f*scale / PTM_RATIO;

	b2FixtureDef wheel1fixtureDef;
	wheel1fixtureDef.shape = &circleShape;
	wheel1fixtureDef.density = 3.0f;
	_wheel1Body->CreateFixture(&wheel1fixtureDef);

	//----------------------------------------------------
	// 取得並設定 wheel2 為【動態物體】
	wheelSprite = _csbRoot->getChildByName("wheel_2");
	loc = wheelSprite->getPosition();
	size = wheelSprite->getContentSize();
	scale = wheelSprite->getScale();

	b2BodyDef wheel2BodyDef;
	wheel2BodyDef.type = b2_dynamicBody;
	wheel2BodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	wheel2BodyDef.userData = wheelSprite;
	_wheel2Body = _b2World->CreateBody(&wheel2BodyDef);

	b2CircleShape circleShape2;
	circleShape2.m_radius = size.width*0.5f*scale / PTM_RATIO;

	b2FixtureDef wheel2fixtureDef;
	wheel2fixtureDef.shape = &circleShape2;
	wheel2fixtureDef.density = 3.0f;
	_wheel2Body->CreateFixture(&wheel2fixtureDef);
	
	//----------------------------------------------------
	//建立 Joint
	//Revolute Joint
	b2RevoluteJointDef revJoint;
	revJoint.bodyA = _CarBody;		//base
	revJoint.localAnchorA.Set(-46.74f / PTM_RATIO, -33.f / PTM_RATIO);
	revJoint.bodyB = _wheel1Body;	//wheel
	revJoint.localAnchorB.Set(0, 0);
	_b2World->CreateJoint(&revJoint);
	//Revolute Joint
	b2RevoluteJointDef revJoint2;
	revJoint2.bodyA = _CarBody;		//base
	revJoint2.localAnchorA.Set(53.21f / PTM_RATIO, -33.f / PTM_RATIO);
	revJoint2.bodyB = _wheel2Body;	//whee2
	revJoint2.localAnchorB.Set(0, 0);
	_b2World->CreateJoint(&revJoint2);
}

void CarLevel::setupSensorPt()
{
	// 取得並設定 tri_line 圖示為【靜態物體】
	Point loc = _triSensor->getPosition();
	Size size = _triSensor->getContentSize();
	auto scale = _triSensor->getScale();
	b2BodyDef staticBodyDef;
	staticBodyDef.type = b2_staticBody;
	staticBodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	staticBodyDef.userData = _triSensor;
	b2Body* staticBody = _b2World->CreateBody(&staticBodyDef);

	b2PolygonShape boxShape;
	boxShape.SetAsBox(size.width*0.5f*scale / PTM_RATIO, size.height*0.5f*scale / PTM_RATIO);

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &boxShape;
	fixtureDef.isSensor = true; // 設定為 Sensor
	staticBody->CreateFixture(&fixtureDef);

	//-----------------------------------------------
	// 取得並設定 rect_line 圖示為【靜態物體】
	loc = _rectSensor->getPosition();
	size = _rectSensor->getContentSize();
	scale = _rectSensor->getScale();
	b2BodyDef staticBodyDef2;
	staticBodyDef2.type = b2_staticBody;
	staticBodyDef2.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	staticBodyDef2.userData = _rectSensor;
	b2Body* staticBody2 = _b2World->CreateBody(&staticBodyDef2);

	b2PolygonShape boxShape2;
	boxShape2.SetAsBox(size.width*0.5f*scale / PTM_RATIO, size.height*0.5f*scale / PTM_RATIO);

	b2FixtureDef fixtureDef2;
	fixtureDef2.shape = &boxShape2;
	fixtureDef2.isSensor = true; // 設定為 Sensor
	staticBody2->CreateFixture(&fixtureDef2);

	//-----------------------------------------------
	// 取得並設定 jeep_line 圖示為【靜態物體】
	loc = _jeepSensor->getPosition();
	size = _jeepSensor->getContentSize();
	scale = _jeepSensor->getScale();
	b2BodyDef staticBodyDef3;
	staticBodyDef3.type = b2_staticBody;
	staticBodyDef3.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	staticBodyDef3.userData = _jeepSensor;
	b2Body* staticBody3 = _b2World->CreateBody(&staticBodyDef3);

	b2PolygonShape boxShape3;
	boxShape3.SetAsBox(size.width*0.5f*scale / PTM_RATIO, size.height*0.5f*scale / PTM_RATIO);

	b2FixtureDef fixtureDef3;
	fixtureDef3.shape = &boxShape3;
	fixtureDef3.isSensor = true; // 設定為 Sensor
	staticBody3->CreateFixture(&fixtureDef3);
}

//黑木頭
void CarLevel::showBlackWood()
{
	//顯示圖案
	_blackWood1->setVisible(true);
	_blackWood2->setVisible(true);
	//----------------------------------------
	_b2BlackBody1->GetFixtureList()->SetSensor(false); //設為可阻擋
	_b2BlackBody2->GetFixtureList()->SetSensor(false);
}
void CarLevel::destroyBlackWood()
{
	//隱藏圖案
	_blackWood1->setVisible(false);
	_blackWood2->setVisible(false);
	//----------------------------------------
	_b2BlackBody1->GetFixtureList()->SetSensor(true); //設為不可阻擋
	_b2BlackBody2->GetFixtureList()->SetSensor(true);
}
//紅木頭
void CarLevel::showRedWood()
{
	//顯示圖案
	_redWood1->setVisible(true);
	_redWood2->setVisible(true);
	//----------------------------------------
	_b2RedBody1->GetFixtureList()->SetSensor(false); //設為可阻擋
	_b2RedBody2->GetFixtureList()->SetSensor(false);
}
void CarLevel::destroyRedWood()
{
	//隱藏圖案
	_redWood1->setVisible(false);
	_redWood2->setVisible(false);
	//----------------------------------------
	_b2RedBody1->GetFixtureList()->SetSensor(true); //設為不可阻擋
	_b2RedBody2->GetFixtureList()->SetSensor(true);
}
//藍木頭
void CarLevel::showBlueWood()
{
	//顯示圖案
	_blueWood->setVisible(true);
	//----------------------------------------
	_b2BlueBody->GetFixtureList()->SetSensor(false); //設為可阻擋
}
void CarLevel::destroyBlueWood()
{
	//隱藏圖案
	_blueWood->setVisible(false);
	//----------------------------------------
	_b2BlueBody->GetFixtureList()->SetSensor(true); //設為不可阻擋
}

void CarLevel::DrawLine(cocos2d::Point prePt, cocos2d::Point Pt, cocos2d::Color4F color, int num)
{
	//畫出線段
	if (color == Color4F::BLACK) {
		_draw[num] = DrawNode::create();
		_draw[num]->drawLine(prePt, Pt, color);
		_blackNode->addChild(_draw[num], 5);
	}
	else if (color == Color4F::RED) {
		_draw_red[num] = DrawNode::create();
		_draw_red[num]->drawLine(prePt, Pt, color);
		_redNode->addChild(_draw_red[num], 5);
	}
	else if (color == Color4F::BLUE) {
		_draw_blue[num] = DrawNode::create();
		_draw_blue[num]->drawLine(prePt, Pt, color);
		_blueNode->addChild(_draw_blue[num], 5);
	}
}

void CarLevel::DrawCuttingLine(cocos2d::Point firstPt, cocos2d::Point Pt, cocos2d::Color4F color, int num)
{
	//畫出剪取線段
	_drawCut[num] = DrawNode::create();
	_drawCut[num]->drawLine(firstPt, Pt, color);
	//_drawCut[num]->clear();
	_cutNode->addChild(_drawCut[num], 5);
}

void CarLevel::doStep(float dt)
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
	if (_fWinCount > 4.0f) { //跳轉回主頁
		_fWinCount = -100.f; //避免重複執行
		auto Scene = MainMenu::createScene();
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

	//----------------------------------------------------------------------------
	// for sensor
	//三角形感應器
	if (_contactListener._bTriSensor) showBlackWood();		//若碰觸 顯示黑木頭
	else destroyBlackWood();
	//矩形感應器
	if (_contactListener._bRectSensor) showRedWood();		//若碰觸 顯示紅木頭
	else destroyRedWood();
	//車子感應器
	if (_contactListener._bJeepSensor) destroyBlueWood();	//若碰觸 移除藍木頭
	else showBlueWood();

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
		else if (!_redpenbtn->getEnable() && _iFree_red > 0) {	//紅筆使用中
			_fflareCount += dt;
			if (_fflareCount > 0.1f) {
				_fflareCount -= 0.1f;
				_flare[_iflareNum]->setColor(Color3B::RED);
				_flare[_iflareNum]->setVisible(true);
				_b2flare[_iflareNum]->SetType(b2_staticBody);
				_b2flare[_iflareNum]->SetTransform(b2Vec2(_tp.x / PTM_RATIO, _tp.y / PTM_RATIO), _b2flare[_iflareNum]->GetAngle());
				_b2flare[_iflareNum]->SetType(b2_dynamicBody);
				_b2flare[_iflareNum]->ApplyForce(b2Vec2(rand() % 5 - 3, 1 + rand() % 3),
					b2Vec2(_tp.x / PTM_RATIO, _tp.y / PTM_RATIO), true);//給力量
				_iflareNum++;
				if (_iflareNum == SPARKS_NUMBER)_iflareNum = 0;
			}
		}
		else if (!_bluepenbtn->getEnable() && _iFree_blue > 0) {	//藍筆使用中
			_fflareCount += dt;
			if (_fflareCount > 0.1f) {
				_fflareCount -= 0.1f;
				_flare[_iflareNum]->setColor(Color3B::BLUE);
				_flare[_iflareNum]->setVisible(true);
				_b2flare[_iflareNum]->SetType(b2_staticBody);
				_b2flare[_iflareNum]->SetTransform(b2Vec2(_tp.x / PTM_RATIO, _tp.y / PTM_RATIO), _b2flare[_iflareNum]->GetAngle());
				_b2flare[_iflareNum]->SetType(b2_dynamicBody);
				_b2flare[_iflareNum]->ApplyForce(b2Vec2(rand() % 5 - 3, 1 + rand() % 3),
					b2Vec2(_tp.x / PTM_RATIO, _tp.y / PTM_RATIO), true);//給力量
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
	//----------------------------------------------------------------------------
	//車子遮罩
	if (_shapebtn->getStatus()) { //形狀偵測開啟
		b2Filter filter;
		for (b2Fixture* f = _CarBody->GetFixtureList(); f; f = f->GetNext()) {
			filter = f->GetFilterData();
			filter.maskBits = 17;
			f->SetFilterData(filter);
		}
		for (b2Fixture* f = _wheel1Body->GetFixtureList(); f; f = f->GetNext()) {
			filter = f->GetFilterData();
			filter.maskBits = 17;
			f->SetFilterData(filter);
		}
		for (b2Fixture* f = _wheel2Body->GetFixtureList(); f; f = f->GetNext()) {
			filter = f->GetFilterData();
			filter.maskBits = 17;
			f->SetFilterData(filter);
		}
	}
	else {						//形狀偵測關閉
		b2Filter filter;
		for (b2Fixture* f = _CarBody->GetFixtureList(); f; f = f->GetNext()) {
			filter = f->GetFilterData();
			filter.maskBits = 65535;
			f->SetFilterData(filter);
		}
		for (b2Fixture* f = _wheel1Body->GetFixtureList(); f; f = f->GetNext()) {
			filter = f->GetFilterData();
			filter.maskBits = 65535;
			f->SetFilterData(filter);
		}
		for (b2Fixture* f = _wheel2Body->GetFixtureList(); f; f = f->GetNext()) {
			filter = f->GetFilterData();
			filter.maskBits = 65535;
			f->SetFilterData(filter);
		}
	}
	
}

bool CarLevel::onTouchBegan(cocos2d::Touch *pTouch, cocos2d::Event *pEvent)//觸碰開始事件
{
	Point touchLoc = pTouch->getLocation();
	_tp = touchLoc;
	_tp_start = touchLoc;
	//-------------------------------------------------------------------
	//For button
	if (_retrybtn->onTouchBegan(touchLoc) || _homebtn->onTouchBegan(touchLoc)
		|| _blackpenbtn->onTouchBegan(touchLoc) || _cutbtn->onTouchBegan(touchLoc)
		|| _redpenbtn->onTouchBegan(touchLoc) || _bluepenbtn->onTouchBegan(touchLoc)
		|| _shapebtn->touchesBegan(touchLoc)) _bBtnPressed = true;

	//-------------------------------------------------------------------
	//For cutting
	if (!_cutbtn->getEnable()) {	//剪刀按鈕點選才可畫圖
		if (!_bBtnPressed)_bCutting = true;	//不在按鈕上才可畫圖
		if (_bCutting && _icutNum < CUTTING_NUM) {
			_cutNode = Node::create(); //新增此筆畫控制節點
			this->addChild(_cutNode, 5);
			_contactListener._cutNode = _cutNode; //傳入Box2d world
			_cutPt[0] = _cutPt[1] = touchLoc;	//第一點
		}
	}
	//--------------------------------------
	//For drawing
	else {							//畫筆按鈕點選才可畫圖
		if (!_bBtnPressed)_bDrawing = true;	//不在按鈕上才可畫圖
		if (_bDrawing) {
			if (!_blackpenbtn->getEnable() && _iFree > 0) {		//黑筆使用中
				_blackNode = Node::create(); //新增此筆畫控制節點
				this->addChild(_blackNode, 5);
				_contactListener._blackNode = _blackNode; //傳入Box2d world
				_istartPt = _iInUsed;	//紀錄第一點
				_pt[_iInUsed] = touchLoc;
				_iInUsed++; _iFree--;
			}
			else if (!_redpenbtn->getEnable() && _iFree_red > 0) {	//紅筆使用中
				_redNode = Node::create(); //新增此筆畫控制節點
				this->addChild(_redNode, 5);
				_contactListener._redNode = _redNode; //傳入Box2d world
				_istartPt_red = _iInUsed_red;	//紀錄第一點
				_pt_red[_iInUsed_red] = touchLoc;
				_iInUsed_red++; _iFree_red--;
			}
			else if (!_bluepenbtn->getEnable() && _iFree_blue > 0) {	//藍筆使用中
				_blueNode = Node::create(); //新增此筆畫控制節點
				this->addChild(_blueNode, 5);
				//_contactListener._blueNode = _blueNode; //傳入Box2d world
				_istartPt_blue = _iInUsed_blue;	//紀錄第一點
				_pt_blue[_iInUsed_blue] = touchLoc;
				_iInUsed_blue++; _iFree_blue--;
			}
		}
	}

	return true;
}

void  CarLevel::onTouchMoved(cocos2d::Touch *pTouch, cocos2d::Event *pEvent) //觸碰移動事件
{
	Point touchLoc = pTouch->getLocation();
	_tp = touchLoc;
	//-------------------------------------------------------------------
	//For cutting
	if (!_cutbtn->getEnable()) {	//剪刀按鈕點選才可畫圖
		if (_bCutting && _icutNum < CUTTING_NUM) {
			_cutPt[1] = touchLoc;
		}
	}
	//--------------------------------------
	//For drawing
	else {							//畫筆按鈕點選才可畫圖
		if (_bDrawing) {
			if (!_blackpenbtn->getEnable() && _iFree > 0) {		//黑筆使用中
				DrawLine(_pt[_iInUsed - 1], touchLoc, Color4F::BLACK, _iInUsed - 1); //畫線函式
				_pt[_iInUsed] = touchLoc;
				_iInUsed++; _iFree--;
				_penBlackBar->setPercent((float)_iFree / (float)LINE_LENGTH * 100.f); //墨水量
			}
			else if (!_redpenbtn->getEnable() && _iFree_red > 0) {	//紅筆使用中
				DrawLine(_pt_red[_iInUsed_red - 1], touchLoc, Color4F::RED, _iInUsed_red - 1); //畫線函式
				_pt_red[_iInUsed_red] = touchLoc;
				_iInUsed_red++; _iFree_red--;
				_penRedBar->setPercent((float)_iFree_red / (float)LINE_LENGTH * 100.f); //墨水量
			}
			else if (!_bluepenbtn->getEnable() && _iFree_blue > 0) {	//藍筆使用中
				DrawLine(_pt_blue[_iInUsed_blue - 1], touchLoc, Color4F::BLUE, _iInUsed_blue - 1); //畫線函式
				_pt_blue[_iInUsed_blue] = touchLoc;
				_iInUsed_blue++; _iFree_blue--;
				_penBlueBar->setPercent((float)_iFree_blue / (float)LINE_LENGTH * 100.f); //墨水量
			}
		}
	}
	//-------------------------------------------------------------------
	//For button
	_retrybtn->onTouchMoved(touchLoc);
	_homebtn->onTouchMoved(touchLoc);
	_blackpenbtn->onTouchMoved(touchLoc);
	_redpenbtn->onTouchMoved(touchLoc);
	_bluepenbtn->onTouchMoved(touchLoc);
	_cutbtn->onTouchMoved(touchLoc);
	_shapebtn->touchesMoved(touchLoc);

}

void  CarLevel::onTouchEnded(cocos2d::Touch *pTouch, cocos2d::Event *pEvent) //觸碰結束事件 
{
	Point touchLoc = pTouch->getLocation();
	_tp = touchLoc;
	//-------------------------------------------------------------------
	//For cutting
	if (!_cutbtn->getEnable()) {		//剪刀按鈕點選才可畫圖
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
			edgeShape.Set(b2Vec2(_cutPt[0].x / PTM_RATIO, _cutPt[0].y / PTM_RATIO),
				b2Vec2(_cutPt[1].x / PTM_RATIO, _cutPt[1].y / PTM_RATIO)); //剪取線段
			body->CreateFixture(&edgeFixtureDef);

			_icutNum++;				//已剪取次數+1
			_cut1->setVisible(false);

			_bCutting = false;
		}
	}
	//--------------------------------------
	//for drawing
	else {								//畫筆按鈕點選才可畫圖
		if (_bDrawing) {
			if (!_blackpenbtn->getEnable()) {		//黑筆使用中
				//建立 手繪【動態物體】
				Point loc = _blackNode->getPosition();
				Size size = _blackNode->getContentSize();
				b2BodyDef bodyDef;				//body只產生一次
				bodyDef.type = b2_dynamicBody;
				bodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
				bodyDef.userData = _blackNode;
				_b2Linebody = _b2World->CreateBody(&bodyDef);

				//手繪【線條】----------------------------------------------

				for (int i = _istartPt; _draw[i] != nullptr && i < LINE_LENGTH - 1; i++) { //取得並設定 線段 為【動態物體】
					b2EdgeShape edgeShape;
					b2FixtureDef edgeFixtureDef; // 產生 Fixture
					edgeFixtureDef.shape = &edgeShape;
					edgeShape.Set(b2Vec2(_pt[i].x / PTM_RATIO, _pt[i].y / PTM_RATIO),
						b2Vec2(_pt[i + 1].x / PTM_RATIO, _pt[i + 1].y / PTM_RATIO)); //畫圖線段

					//設定遮罩
					if (_shapebtn->getStatus()) edgeFixtureDef.filter.categoryBits = 2;
					else edgeFixtureDef.filter.categoryBits = 1;

					_b2Linebody->CreateFixture(&edgeFixtureDef);
				}
			}
			else if (!_redpenbtn->getEnable()) {	//紅筆使用中
				//建立 手繪【動態物體】
				Point loc = _redNode->getPosition();
				Size size = _redNode->getContentSize();
				b2BodyDef bodyDef;				//body只產生一次
				bodyDef.type = b2_dynamicBody;
				bodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
				bodyDef.userData = _redNode;
				_b2Linebody_red = _b2World->CreateBody(&bodyDef);

				//手繪【線條】----------------------------------------------

				for (int i = _istartPt_red; _draw_red[i] != nullptr && i < LINE_LENGTH - 1; i++) { //取得並設定 線段 為【動態物體】
					b2EdgeShape edgeShape;
					b2FixtureDef edgeFixtureDef; // 產生 Fixture
					edgeFixtureDef.shape = &edgeShape;
					edgeShape.Set(b2Vec2(_pt_red[i].x / PTM_RATIO, _pt_red[i].y / PTM_RATIO),
						b2Vec2(_pt_red[i + 1].x / PTM_RATIO, _pt_red[i + 1].y / PTM_RATIO)); //畫圖線段

					//設定遮罩
					if (_shapebtn->getStatus()) edgeFixtureDef.filter.categoryBits = 4;
					else edgeFixtureDef.filter.categoryBits = 1;

					_b2Linebody_red->CreateFixture(&edgeFixtureDef);
				}
			}
			else if (!_bluepenbtn->getEnable()) {	//藍筆使用中
				//建立 手繪【動態物體】
				Point loc = _blueNode->getPosition();
				Size size = _blueNode->getContentSize();
				b2BodyDef bodyDef;				//body只產生一次
				bodyDef.type = b2_dynamicBody;
				bodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
				bodyDef.userData = _blueNode;
				_b2Linebody_blue = _b2World->CreateBody(&bodyDef);

				//手繪【線條】----------------------------------------------

				for (int i = _istartPt_blue; _draw_blue[i] != nullptr && i < LINE_LENGTH - 1; i++) { //取得並設定 線段 為【動態物體】
					b2EdgeShape edgeShape;
					b2FixtureDef edgeFixtureDef; // 產生 Fixture
					edgeFixtureDef.shape = &edgeShape;
					edgeShape.Set(b2Vec2(_pt_blue[i].x / PTM_RATIO, _pt_blue[i].y / PTM_RATIO),
						b2Vec2(_pt_blue[i + 1].x / PTM_RATIO, _pt_blue[i + 1].y / PTM_RATIO)); //畫圖線段

					//設定遮罩
					if (_shapebtn->getStatus()) edgeFixtureDef.filter.categoryBits = 8;
					else edgeFixtureDef.filter.categoryBits = 1;

					_b2Linebody_blue->CreateFixture(&edgeFixtureDef);
				}
			}
			_bDrawing = false;
		}
	}
	//---------------------------------------------------------------
	//Button
	_bBtnPressed = false;
	if (_retrybtn->onTouchEnded(touchLoc)) {		//重新遊戲鈕
		auto Scene = CarLevel::createScene();
		Director::getInstance()->replaceScene(TransitionFade::create(1, Scene, Color3B(255, 255, 255)));
	}
	if (_homebtn->onTouchEnded(touchLoc)) {			//回首頁鈕
		auto mainScene = MainMenu::createScene();
		Director::getInstance()->replaceScene(TransitionFade::create(1, mainScene, Color3B(255, 255, 255)));
	}
	if (_blackpenbtn->onTouchEnded(touchLoc)) {		//黑筆鈕
		_redpenbtn->setEnable(true);
		_bluepenbtn->setEnable(true);
		_cutbtn->setEnable(true);
	}
	if (_redpenbtn->onTouchEnded(touchLoc)) {		//紅筆鈕
		_blackpenbtn->setEnable(true);
		_bluepenbtn->setEnable(true);
		_cutbtn->setEnable(true);
	}
	if (_bluepenbtn->onTouchEnded(touchLoc)) {		//藍筆鈕
		_blackpenbtn->setEnable(true);
		_redpenbtn->setEnable(true);
		_cutbtn->setEnable(true);
	}
	if (_cutbtn->onTouchEnded(touchLoc)) {			//剪刀鈕
		_redpenbtn->setEnable(true);
		_bluepenbtn->setEnable(true);
		_blackpenbtn->setEnable(true);
	}
	_shapebtn->touchesEnded(touchLoc);				//形狀紐
}

void CarLevel::createStaticBoundary()
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
	//// bottom edge
	//edgeShape.Set(b2Vec2(0.0f / PTM_RATIO, 0.0f / PTM_RATIO), b2Vec2(_visibleSize.width / PTM_RATIO, 0.0f / PTM_RATIO));
	//body->CreateFixture(&edgeFixtureDef);

	//// left edge
	//edgeShape.Set(b2Vec2(0.0f / PTM_RATIO, 0.0f / PTM_RATIO), b2Vec2(0.0f / PTM_RATIO, _visibleSize.height / PTM_RATIO));
	//body->CreateFixture(&edgeFixtureDef);

	// right edge
	edgeShape.Set(b2Vec2(_visibleSize.width / PTM_RATIO, 0.0f / PTM_RATIO), b2Vec2(_visibleSize.width / PTM_RATIO, _visibleSize.height / PTM_RATIO));
	body->CreateFixture(&edgeFixtureDef);

	//// top edge
	//edgeShape.Set(b2Vec2(0.0f / PTM_RATIO, _visibleSize.height / PTM_RATIO), b2Vec2(_visibleSize.width / PTM_RATIO, _visibleSize.height / PTM_RATIO));
	//body->CreateFixture(&edgeFixtureDef);
}

#ifdef BOX2D_DEBUG
//改寫繪製方法
void CarLevel::draw(Renderer *renderer, const Mat4 &transform, uint32_t flags)
{
	Director* director = Director::getInstance();

	GL::enableVertexAttribs(cocos2d::GL::VERTEX_ATTRIB_FLAG_POSITION);
	director->pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
	_b2World->DrawDebugData();
	director->popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
}
#endif

//================================================================================
 CContactListenerLevel5:: CContactListenerLevel5()
{
	_bWin = false;
	_bCreateSpark = false;
	//_NumOfSparks = 20;	//噴發分子數

	_bDeleteJoint = false;

	_bTriSensor = false;
	_bRectSensor = false;
	_bJeepSensor = false;
}

// 只要是兩個 body 的 fixtures 碰撞，就會呼叫這個函式
void  CContactListenerLevel5::BeginContact(b2Contact* contact)
{
	b2Body* BodyA = contact->GetFixtureA()->GetBody();
	b2Body* BodyB = contact->GetFixtureB()->GetBody();
	//----------------------------------------------------------------------
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
	//----------------------------------------------------------------------
	// Sensor 偵測
	if (BodyA->GetUserData() == _triSensor) { //A:三角偵測點, B:黑色筆跡
		auto shape = BodyB->GetFixtureList()->GetBody()->GetUserData();
		if (shape == _blackNode) {
			_bTriSensor = true;
		}
	}
	else if (BodyB->GetUserData() == _triSensor) { //A:黑色筆跡, B:三角偵測點
		auto shape = BodyA->GetFixtureList()->GetBody()->GetUserData();
		if (shape == _blackNode) {
			_bTriSensor = true;
		}
	}
	//--------------
	if (BodyA->GetUserData() == _rectSensor) { //A:矩形偵測點, B:紅色筆跡
		auto shape = BodyB->GetFixtureList()->GetBody()->GetUserData();
		if (shape == _redNode) {
			_bRectSensor = true;
		}
	}
	else if (BodyB->GetUserData() == _rectSensor) { //A:紅色筆跡, B:矩形偵測點
		auto shape = BodyA->GetFixtureList()->GetBody()->GetUserData();
		if (shape == _redNode) {
			_bRectSensor = true;
		}
	}
	//--------------
	if (BodyA->GetUserData() == _jeepSensor) { //A:車子偵測點, B:車子
		auto shape = BodyB->GetFixtureList()->GetBody()->GetUserData();
		if (shape == _jeepSprite) {
			_bJeepSensor = true;
		}
	}
	else if (BodyB->GetUserData() == _jeepSensor) { //A:車子, B:車子偵測點
		auto shape = BodyA->GetFixtureList()->GetBody()->GetUserData();
		if (shape == _jeepSprite) {
			_bJeepSensor = true;
		}
	}
}

//碰撞結束
void  CContactListenerLevel5::EndContact(b2Contact* contact)
{
	b2Body* BodyA = contact->GetFixtureA()->GetBody();
	b2Body* BodyB = contact->GetFixtureB()->GetBody();

	// Sensor 偵測
	if (BodyA->GetUserData() == _triSensor) { //A:三角偵測點, B:黑色筆跡
		auto shape = BodyB->GetFixtureList()->GetBody()->GetUserData();
		if (shape == _blackNode) {
			_bTriSensor = false;
		}
	}
	else if (BodyB->GetUserData() == _triSensor) { //A:黑色筆跡, B:三角偵測點
		auto shape = BodyA->GetFixtureList()->GetBody()->GetUserData();
		if (shape == _blackNode) {
			_bTriSensor = false;
		}
	}
	//--------------
	if (BodyA->GetUserData() == _rectSensor) { //A:矩形偵測點, B:紅色筆跡
		auto shape = BodyB->GetFixtureList()->GetBody()->GetUserData();
		if (shape == _redNode) {
			_bRectSensor = false;
		}
	}
	else if (BodyB->GetUserData() == _rectSensor) { //A:紅色筆跡, B:矩形偵測點
		auto shape = BodyA->GetFixtureList()->GetBody()->GetUserData();
		if (shape == _redNode) {
			_bRectSensor = false;
		}
	}
	//--------------
	if (BodyA->GetUserData() == _jeepSensor) { //A:車子偵測點, B:車子
		auto shape = BodyB->GetFixtureList()->GetBody()->GetUserData();
		if (shape == _jeepSprite) {
			_bJeepSensor = false;
		}
	}
	else if (BodyB->GetUserData() == _jeepSensor) { //A:車子, B:車子偵測點
		auto shape = BodyA->GetFixtureList()->GetBody()->GetUserData();
		if (shape == _jeepSprite) {
			_bJeepSensor = false;
		}
	}
}