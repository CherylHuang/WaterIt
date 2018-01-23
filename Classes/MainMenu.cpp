#include "MainMenu.h"
#include "Common/C3SButton.h"
#include "cocostudio/CocoStudio.h"
#include "JointLevel.h"
#include "GearLevel.h"
#include "GravityLevel.h"
#include "CarLevel.h"

USING_NS_CC;

using namespace cocostudio::timeline;
using namespace ui;
using namespace CocosDenshion;

int MainMenu::level = 0; //靜態變數初始值

Scene* MainMenu::createScene()
{
	// 'scene' is an autorelease object
	auto scene = Scene::create();

	// 'layer' is an autorelease object
	auto layer = MainMenu::create();

	// add layer as a child to scene
	scene->addChild(layer);

	// return the scene
	return scene;
}

MainMenu::~MainMenu()
{
	AnimationCache::destroyInstance();  // 釋放 AnimationCache 取得的資源
	SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("box2d_pic.plist");
	SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("box2d_bg_pic.plist");
	SpriteFrameCache::getInstance()->removeUnusedSpriteFrames();
	Director::getInstance()->getTextureCache()->removeUnusedTextures();
}

// on "init" you need to initialize your instance
bool MainMenu::init()
{

	_visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	if (!Layer::init())
	{
		return false;
	}

	_rootNode = CSLoader::createNode("MainMenu.csb");
	addChild(_rootNode);

	//讀入圖檔
	SpriteFrameCache::getInstance()->addSpriteFramesWithFile("box2d_pic.plist");
	SpriteFrameCache::getInstance()->addSpriteFramesWithFile("box2d_bg_pic.plist");

	//變數初始值
	settingVisible = false;
	level = 1; //沒有選關卡

	//讀取物件
	_black_bg = dynamic_cast<Sprite*>(_rootNode->getChildByName("black_bg")); this->addChild(_black_bg, 3);
	_settingWindow = dynamic_cast<Sprite*>(_rootNode->getChildByName("setting_window")); this->addChild(_settingWindow, 3);
	//----------------------------------------------------------
	//Start & End
	_startWater = dynamic_cast<Sprite*>(_rootNode->getChildByName("water"));
	_Bush_dry = dynamic_cast<Sprite*>(_rootNode->getChildByName("Bush_dry"));
	_Bush_live = dynamic_cast<Sprite*>(_rootNode->getChildByName("Bush_live"));
	_fStartCount = 0;
	_contactListener._StartSprite = _startWater;
	_contactListener._EndSprite = _Bush_dry;
	_startPt = _startWater->getPosition();
	_fcount = 0;

	// Button -------------------------------------------------------------------------------------
	//Type 1
	auto BtnPos = dynamic_cast<Button*>(_rootNode->getChildByName("btn_start")); //開始鈕
	Point loc = BtnPos->getPosition();
	BtnPos->setVisible(false);
	_startbtn = C3SButton::create();
	_startbtn->setButtonInfo(NORMAL_BTN, "btn_start_0.png", "btn_start_2.png", "btn_start_1.png", loc);
	_startbtn->setScale(0.5f);
	this->addChild(_startbtn, 1);
	
	BtnPos = dynamic_cast<Button*>(_rootNode->getChildByName("btn_setting")); //設定鈕
	loc = BtnPos->getPosition();
	BtnPos->setVisible(false);
	_settingbtn = C3SButton::create();
	_settingbtn->setButtonInfo(NORMAL_BTN, "btn_setting_0.png", "btn_setting_2.png", "btn_setting_1.png", loc);
	_settingbtn->setScale(0.5f);
	this->addChild(_settingbtn, 1);

	BtnPos = dynamic_cast<Button*>(_rootNode->getChildByName("btn_exit")); //離開鈕
	loc = BtnPos->getPosition();
	BtnPos->setVisible(false);
	_exitbtn = C3SButton::create();
	_exitbtn->setButtonInfo(NORMAL_BTN, "btn_exit_0.png", "btn_exit_2.png", "btn_exit_1.png", loc);
	_exitbtn->setScale(0.5f);
	this->addChild(_exitbtn, 1);

	BtnPos = dynamic_cast<Button*>(_rootNode->getChildByName("btn_okay")); //OK鈕
	loc = BtnPos->getPosition();
	BtnPos->setVisible(false);
	_okaybtn = C3SButton::create();
	_okaybtn->setButtonInfo(NORMAL_BTN, "btn_okay_0.png", "btn_okay_2.png", "btn_okay_1.png", loc);
	_okaybtn->setScale(0.5f);
	_okaybtn->setVisible(false);
	this->addChild(_okaybtn, 4);

	//Type 2
	BtnPos = dynamic_cast<Button*>(_rootNode->getChildByName("btn_easy")); //LEVEL1鈕
	loc = BtnPos->getPosition();
	BtnPos->setVisible(false);
	_easybtn = C3SButton::create();
	_easybtn->setButtonInfo(SELECT_BTN, "btn_level1_0.png", "btn_level1_2.png", "btn_level1_1.png", loc);
	_easybtn->setScale(0.4f);
	_easybtn->setVisible(false);
	this->addChild(_easybtn, 3);

	BtnPos = dynamic_cast<Button*>(_rootNode->getChildByName("btn_normal")); //LEVEL2鈕
	loc = BtnPos->getPosition();
	BtnPos->setVisible(false);
	_normalbtn = C3SButton::create();
	_normalbtn->setButtonInfo(SELECT_BTN, "btn_level2_0.png", "btn_level2_2.png", "btn_level2_1.png", loc);
	_normalbtn->setScale(0.4f);
	_normalbtn->setVisible(false);
	this->addChild(_normalbtn, 3);

	BtnPos = dynamic_cast<Button*>(_rootNode->getChildByName("btn_hard")); //LEVEL3鈕
	loc = BtnPos->getPosition();
	BtnPos->setVisible(false);
	_hardbtn = C3SButton::create();
	_hardbtn->setButtonInfo(SELECT_BTN, "btn_level3_0.png", "btn_level3_2.png", "btn_level3_1.png", loc);
	_hardbtn->setScale(0.4f);
	_hardbtn->setVisible(false);
	this->addChild(_hardbtn, 3);

	BtnPos = dynamic_cast<Button*>(_rootNode->getChildByName("btn_final")); //LEVEL4鈕
	loc = BtnPos->getPosition();
	BtnPos->setVisible(false);
	_finalbtn = C3SButton::create();
	_finalbtn->setButtonInfo(SELECT_BTN, "btn_level4_0.png", "btn_level4_2.png", "btn_level4_1.png", loc);
	_finalbtn->setScale(0.4f);
	_finalbtn->setVisible(false);
	this->addChild(_finalbtn, 3);
	//---------------------------------------------------------------------------------------------	
	// 建立 Box2D world
	_b2World = nullptr;
	b2Vec2 Gravity = b2Vec2(0.0f, -9.8f);		//重力方向
	bool AllowSleep = true;			//允許睡著
	_b2World = new b2World(Gravity);	//創建世界
	_b2World->SetAllowSleeping(AllowSleep);	//設定物件允許睡著
	_b2World->SetContactListener(&_contactListener); //碰撞聆聽器

	//---------------------------------------------------------------------------------------------	
	setStartEndpoint();			//起終點
	setupStatic();				//靜態物體

	_listener1 = EventListenerTouchOneByOne::create();	//創建一個一對一的事件聆聽器
	_listener1->onTouchBegan = CC_CALLBACK_2(MainMenu::onTouchBegan, this);		//加入觸碰開始事件
	_listener1->onTouchMoved = CC_CALLBACK_2(MainMenu::onTouchMoved, this);		//加入觸碰移動事件
	_listener1->onTouchEnded = CC_CALLBACK_2(MainMenu::onTouchEnded, this);		//加入觸碰離開事件

	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(_listener1, this);	//加入剛創建的事件聆聽器
	this->schedule(CC_SCHEDULE_SELECTOR(MainMenu::doStep));
	return true;
}

void MainMenu::doStep(float dt)
{
	int velocityIterations = 8;	// 速度迭代次數
	int positionIterations = 1; // 位置迭代次數 迭代次數一般設定為8~10 越高越真實但效率越差
								// Instruct the world to perform a single step of simulation.
								// It is generally best to keep the time step and iterations fixed.
	_b2World->Step(dt, velocityIterations, positionIterations);

	// 取得 _b2World 中所有的 body 進行處理
	// 最主要是根據目前運算的結果，更新附屬在 body 中 sprite 的位置
	for (b2Body* body = _b2World->GetBodyList(); body;)
	{
		// 以下是以 Body 有包含 Sprite 顯示為例
		if (body->GetUserData() != NULL)
		{
			Sprite *spriteData = (Sprite*)body->GetUserData();
			spriteData->setPosition(body->GetPosition().x*PTM_RATIO, body->GetPosition().y*PTM_RATIO);
			spriteData->setRotation(-1 * CC_RADIANS_TO_DEGREES(body->GetAngle()));
		}
				// 跑出螢幕外面就讓物體從 b2World 中移除
				if (body->GetType() == b2BodyType::b2_dynamicBody) {
					float x = body->GetPosition().x * PTM_RATIO;
					float y = body->GetPosition().y * PTM_RATIO;
					if (x > _visibleSize.width || x < 0 || y >  _visibleSize.height || y < 0) {
						if (body->GetUserData() != NULL) {
							Sprite* spriteData = (Sprite *)body->GetUserData();
							this->removeChild(spriteData);
						}
						b2Body* nextbody = body->GetNext(); // 取得下一個 body
						_b2World->DestroyBody(body); // 釋放目前的 body
						body = nextbody;  // 讓 body 指向剛才取得的下一個 body
		#ifdef CREATED_REMOVED
						g_totRemoved2++;
						CCLOG("Removing %4d Particles", g_totRemoved2);
		#endif
					}
					else body = body->GetNext(); //否則就繼續更新下一個Body
				}
				else body = body->GetNext(); //否則就繼續更新下一個Body
	}

	//----------------------------------------------------------------------------
	//for Start & End
	if (!_waterBody->IsAwake() && !_contactListener._bWin)_fStartCount += dt; //水球靜止中、遊戲尚未結束 : 開始計時
	if (_fStartCount > 3.0f) {	//靜止超過三秒，重置位置
		_fStartCount -= 3.0f;

		_startWater->setVisible(true);
		_Bush_live->setVisible(false);

		_waterBody->SetType(b2_staticBody);
		_waterBody->SetTransform(b2Vec2(_startPt.x / PTM_RATIO, _startPt.y / PTM_RATIO), _waterBody->GetAngle());
		_waterBody->SetType(b2_dynamicBody); //BUG修正 : 直接設定位置會卡住，轉換Type讓Body持續落下
	}
	if (_contactListener._bWin) {		//過關
		_startWater->setVisible(false);
		_Bush_live->setVisible(true);
		_fcount += dt;
	}
	if (_fcount > 1.0f) {				//超過3秒 重置
		_contactListener._bWin = false;
		_fcount = 0;
	}

#ifdef WATER_PARTICLE
	// 產生過關水花
	if (_contactListener._bCreateSpark) {
		_contactListener._bCreateSpark = false;	//產生完關閉
		for (int i = 0; i < _contactListener._NumOfSparks; i++) {
			// 建立 Spark Sprite 並與目前的物體結合
			auto sparkSprite = Sprite::createWithSpriteFrameName("pen.png");
			sparkSprite->setColor(Color3B(0, 146 - rand() % 20, 200 - rand() % 20));
			sparkSprite->setBlendFunc(BlendFunc::ADDITIVE);
			this->addChild(sparkSprite, 2);
			//產生小方塊資料
			b2BodyDef RectBodyDef;
			RectBodyDef.position.Set(_contactListener._createLoc.x, _contactListener._createLoc.y);
			RectBodyDef.type = b2_dynamicBody;
			RectBodyDef.userData = sparkSprite;
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
#endif
}

void MainMenu::setupStatic()
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
		auto frameSprite = (Sprite *)_rootNode->getChildByName(tmp);

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

void MainMenu::setStartEndpoint()
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

bool MainMenu::onTouchBegan(cocos2d::Touch *pTouch, cocos2d::Event *pEvent)//觸碰開始事件
{
	Point touchLoc = pTouch->getLocation();
	_settingbtn->onTouchBegan(touchLoc);
	_okaybtn->onTouchBegan(touchLoc);
	_startbtn->onTouchBegan(touchLoc);
	_exitbtn->onTouchBegan(touchLoc);
	_easybtn->onTouchBegan(touchLoc);
	_normalbtn->onTouchBegan(touchLoc);
	_hardbtn->onTouchBegan(touchLoc);
	_finalbtn->onTouchBegan(touchLoc);
	return true;
}

void  MainMenu::onTouchMoved(cocos2d::Touch *pTouch, cocos2d::Event *pEvent) //觸碰移動事件
{
	Point touchLoc = pTouch->getLocation();
	_settingbtn->onTouchMoved(touchLoc);
	_okaybtn->onTouchMoved(touchLoc);
	_startbtn->onTouchMoved(touchLoc);
	_exitbtn->onTouchMoved(touchLoc);
	_easybtn->onTouchMoved(touchLoc);
	_normalbtn->onTouchMoved(touchLoc);
	_hardbtn->onTouchMoved(touchLoc);
	_finalbtn->onTouchMoved(touchLoc);
}

void  MainMenu::onTouchEnded(cocos2d::Touch *pTouch, cocos2d::Event *pEvent) //觸碰結束事件 
{
	Point touchLoc = pTouch->getLocation();
	if (_settingbtn->onTouchEnded(touchLoc) || _okaybtn->onTouchEnded(touchLoc)) { //設定&OK btn
		//隱藏3個選項
		_settingbtn->setVisible(settingVisible); 
		_startbtn->setVisible(settingVisible);
		_exitbtn->setVisible(settingVisible);

		//顯示設定視窗
		_okaybtn->setVisible(!settingVisible);
		_easybtn->setVisible(!settingVisible);
		_normalbtn->setVisible(!settingVisible);
		_hardbtn->setVisible(!settingVisible);
		_finalbtn->setVisible(!settingVisible);
		settingVisible = !settingVisible;
		_settingWindow->setVisible(settingVisible);
		_black_bg->setVisible(settingVisible);
	}
	if (_startbtn->onTouchEnded(touchLoc)) { //開始鈕
		if (level == 2) {
			auto scene01 = GearLevel::createScene();
			Director::getInstance()->replaceScene(TransitionFade::create(1, scene01, Color3B(255, 255, 255)));
		}
		else if (level == 3) {
			auto scene01 = GravityLevel::createScene();
			Director::getInstance()->replaceScene(TransitionFade::create(1, scene01, Color3B(255, 255, 255)));
		}
		else if (level == 4) {
			auto scene01 = CarLevel::createScene();
			Director::getInstance()->replaceScene(TransitionFade::create(1, scene01, Color3B(255, 255, 255)));
		}
		else {
			auto scene01 = JointLevel::createScene(); //切換初始場景
			Director::getInstance()->replaceScene(TransitionFade::create(1, scene01, Color3B(255, 255, 255)));
		}
	}
	if (_exitbtn->onTouchEnded(touchLoc)) { //離開鈕
		unscheduleAllCallbacks();  // 將所有註冊在 Schedule 中的程序都取消
		Director::getInstance()->end(); // 終止 Director 的執行
	}
	if (_easybtn->onTouchEnded(touchLoc)) { //level btn
		_normalbtn->setEnable(true);
		_hardbtn->setEnable(true);
		_finalbtn->setEnable(true);
		level = 1;
	}
	if (_normalbtn->onTouchEnded(touchLoc)) {
		_easybtn->setEnable(true);
		_hardbtn->setEnable(true);
		_finalbtn->setEnable(true);
		level = 2;
	}
	if (_hardbtn->onTouchEnded(touchLoc)) {
		_easybtn->setEnable(true);
		_normalbtn->setEnable(true);
		_finalbtn->setEnable(true);
		level = 3;
	}
	if (_finalbtn->onTouchEnded(touchLoc)) {
		_easybtn->setEnable(true);
		_normalbtn->setEnable(true);
		_hardbtn->setEnable(true);
		level = 4;
	}
}

//================================================================================
CContactListenerMainMenu::CContactListenerMainMenu()
{
	_bWin = false;
	_bCreateSpark = false;
	_NumOfSparks = 20;	//噴發分子數
}

// 只要是兩個 body 的 fixtures 碰撞，就會呼叫這個函式
void CContactListenerMainMenu::BeginContact(b2Contact* contact)
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
}

//碰撞結束
void CContactListenerMainMenu::EndContact(b2Contact* contact)
{

}