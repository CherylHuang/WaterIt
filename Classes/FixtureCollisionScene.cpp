#include "FixtureCollisionScene.h"
#include "cocostudio/CocoStudio.h"
#include "ui/CocosGUI.h"

#define  CREATED_REMOVED
#ifdef CREATED_REMOVED
int g_totCreated = 0, g_totRemoved = 0;
#endif

USING_NS_CC;
using namespace cocostudio::timeline;
Color3B filterColor[3] = { Color3B(208,45,45), Color3B(77,204,42), Color3B(14,201,220)};

#ifndef MAX_CIRCLE_OBJECTS
#define MAX_CIRCLE_OBJECTS  11
#endif
extern char g_CircleObject[MAX_CIRCLE_OBJECTS][20];

FixtureCollision::~FixtureCollision()
{

#ifdef BOX2D_DEBUG
	if (_DebugDraw != NULL) delete _DebugDraw;
#endif

	if (_b2World != nullptr) delete _b2World;
//  for releasing Plist&Texture
	SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("box2d.plist");
	Director::getInstance()->getTextureCache()->removeUnusedTextures();

}

Scene* FixtureCollision::createScene()
{
    auto scene = Scene::create();
    auto layer = FixtureCollision::create();
    scene->addChild(layer);
    return scene;
}

// on "init" you need to initialize your instance
bool FixtureCollision::init()
{   
    //////////////////////////////
    // 1. super init first
    if ( !Layer::init() )
    {
        return false;
    }

//  For Loading Plist+Texture
	SpriteFrameCache::getInstance()->addSpriteFramesWithFile("box2d.plist");

	_visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();
    
	// 建立 Box2D world
	_b2World = nullptr;
	b2Vec2 Gravity = b2Vec2(0.0f, -9.8f);		//重力方向
	bool AllowSleep = true;			//允許睡著
	_b2World = new b2World(Gravity);	//創建世界
	_b2World->SetAllowSleeping(AllowSleep);	//設定物件允許睡著

	// Create Scene with csb file
	_csbRoot = CSLoader::createNode("FixtureCollision.csb");
#ifndef BOX2D_DEBUG
	// 設定顯示背景圖示
	auto bgSprite = _csbRoot->getChildByName("bg64_1");
	bgSprite->setVisible(true);

#endif
	addChild(_csbRoot, 1);

	setStaticWalls();
	createStaticBoundary();
	setupDesnity();
	setupFrictionAndFilter();
	setupSensorAndCollision();
	

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

	_b2World->SetContactListener(&_contactListener);

	_listener1 = EventListenerTouchOneByOne::create();	//創建一個一對一的事件聆聽器
	_listener1->onTouchBegan = CC_CALLBACK_2(FixtureCollision::onTouchBegan, this);		//加入觸碰開始事件
	_listener1->onTouchMoved = CC_CALLBACK_2(FixtureCollision::onTouchMoved, this);		//加入觸碰移動事件
	_listener1->onTouchEnded = CC_CALLBACK_2(FixtureCollision::onTouchEnded, this);		//加入觸碰離開事件

	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(_listener1, this);	//加入剛創建的事件聆聽器
	this->schedule(CC_SCHEDULE_SELECTOR(FixtureCollision::doStep));

    return true;
}

void FixtureCollision::setupDesnity()
{
	Point pntLoc = _csbRoot->getPosition();

	b2BodyDef bodyDef;
	bodyDef.userData = NULL;
	// 在 b2World 中產生該 Body, 並傳回產生的 b2Body 物件的指標
	// 產生一次，就可以讓後面所有的 Shape 使用
	b2FixtureDef fixtureDef; // 產生 Fixture
	char tmp[20] = "";

	// 產生三角形靜態物體所需要的 triShape
	// 產生蹺蹺板底座的三角形
	b2Body *seesawBasedbody;
	b2PolygonShape triShape;
	fixtureDef.shape = &triShape;
	fixtureDef.density = 1.0f;
	fixtureDef.friction = 0.2f;
	fixtureDef.restitution = 0.8f;
	for (size_t i = 1; i <= 1; i++)
	{
		// 產生所需要的 Sprite file name int plist 
		// 此處取得的都是相對於 csbRoot 所在位置的相對座標
		// 在計算 edgeShape 的相對應座標時，必須進行轉換
		sprintf(tmp, "triangle1_%02d", i);
		auto triSprite = (Sprite *)_csbRoot->getChildByName(tmp);
		Size ts = triSprite->getContentSize();
		Point loc = triSprite->getPosition();
		float scaleX = triSprite->getScaleX();	// 對 X 軸放大
		float scaleY = triSprite->getScaleY();	// 對 Y 軸放大

		Point lep[3], wep[3];	// triShape 的三個頂點, 0 頂點、 1 左下、 2 右下
		lep[0].x = 0;  lep[0].y = (ts.height - 2) / 2.0f;
		lep[1].x = -(ts.width - 2) / 2.0f; lep[1].y = -(ts.height - 2) / 2.0f;
		lep[2].x = (ts.width - 2) / 2.0f; lep[2].y = -(ts.height - 2) / 2.0f;

		// Step1: 先CHECK 有無旋轉，有旋轉則進行端點的計算
		cocos2d::Mat4 modelMatrix, rotMatrix;
		modelMatrix.m[0] = scaleX;  // 先設定 X 軸的縮放
		modelMatrix.m[5] = scaleY;  // 先設定 Y 軸的縮放
		for (size_t j = 0; j < 3; j++) 
		{   // 納入縮放與旋轉的 local space 的座標計算
			wep[j].x = lep[j].x * modelMatrix.m[0] + lep[j].y * modelMatrix.m[1];
			wep[j].y = lep[j].x * modelMatrix.m[4] + lep[j].y * modelMatrix.m[5];
		}
		b2Vec2 vecs[] = {
			b2Vec2(wep[0].x / PTM_RATIO, wep[0].y / PTM_RATIO),
			b2Vec2(wep[1].x / PTM_RATIO, wep[1].y / PTM_RATIO),
			b2Vec2(wep[2].x / PTM_RATIO, wep[2].y / PTM_RATIO) 
		};
		triShape.Set(vecs, 3);
		bodyDef.type = b2_staticBody;
		bodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
		seesawBasedbody = _b2World->CreateBody(&bodyDef);
		seesawBasedbody->CreateFixture(&fixtureDef);
	}

	// 產生蹺蹺板，此為動態，每一個動態物體在 b2World 中都必須建立實體
	// 設定這個 Body 為 動態的，並在 _b2World 中建立實體，
	bodyDef.type = b2_dynamicBody;
	b2PolygonShape seesawBoardShape;
	fixtureDef.shape = &seesawBoardShape;
	fixtureDef.density = 5.0f;
	fixtureDef.friction = 0.1f;
	fixtureDef.restitution = 0.1f;
	// 取得代表蹺蹺板的圖示，並設定成 sprite
	auto boardSprite = (Sprite *)_csbRoot->getChildByName("seesawBoard");
	bodyDef.userData = boardSprite;
	Size ts = boardSprite->getContentSize();
	Point loc = boardSprite->getPosition();
	float scaleX = boardSprite->getScaleX();	// 對矩形圖示 X 軸縮放值
	float scaleY = boardSprite->getScaleY();	// 對矩形圖示 Y 軸縮放值

	// 設定板子所在的位置，因為是使用 joint 可以不用設定位置
//	bodyDef.position.Set(loc.x/ PTM_RATIO, loc.y/ PTM_RATIO); 
	b2Body *seesawBoardbody = _b2World->CreateBody(&bodyDef); // 在 b2World 中建立實體

	// 算出 seesawBoard 的縮放後的寬高, 4 為預留的寬度，不跟其他的圖片重疊
	float bw = (ts.width -4)* scaleX; 	
	float bh = (ts.height -4)* scaleY;

	// 設定剛體的範圍是一個 BOX （可以縮放成矩形）
	seesawBoardShape.SetAsBox(bw*0.5f / PTM_RATIO, bh*0.5f / PTM_RATIO);
	seesawBoardbody->CreateFixture(&fixtureDef);

	// 建立與基底三角形的 Joint 連結
	b2RevoluteJointDef seesawJoint;
	seesawJoint.bodyA = seesawBasedbody;
	seesawJoint.localAnchorA.Set(0, 1.2f);
	seesawJoint.bodyB = seesawBoardbody;
	seesawJoint.localAnchorB.Set(0,0);
	_b2World->CreateJoint(&seesawJoint);

	// 讀取並建立兩顆球的實體
	// ball1 with density=10
	bodyDef.type = b2_dynamicBody;
	b2CircleShape circleShape;
	fixtureDef.shape = &circleShape;
	fixtureDef.density = 10.0f;
	fixtureDef.friction = 0.2f;
	fixtureDef.restitution = 0.15f;

	auto ballSprite = (Sprite *)_csbRoot->getChildByName("densityball1");
	bodyDef.userData = ballSprite; // body 與 Sprite 連結

	Point ballloc = ballSprite->getPosition();
	Size ballsize = ballSprite->getContentSize();
	float ballScale = ballSprite->getScale();
	// 根據 Sprite 所在位置設定圓形在 b2World 中的位置
	bodyDef.position.Set(ballloc.x/ PTM_RATIO, ballloc.y / PTM_RATIO);
	b2Body* ball1 = _b2World->CreateBody(&bodyDef);
	// 設定圓形的半徑
	circleShape.m_radius = ballsize.width *ballScale * 0.5f / PTM_RATIO;
	ball1->CreateFixture(&fixtureDef);

	// ball1 with density=100
	fixtureDef.density = 100.0f;
	ballSprite = (Sprite *)_csbRoot->getChildByName("densityball2");
	bodyDef.userData = ballSprite;
	ballloc = ballSprite->getPosition();
	ballsize = ballSprite->getContentSize();
	ballScale = ballSprite->getScale();
	bodyDef.position.Set(ballloc.x / PTM_RATIO, ballloc.y / PTM_RATIO);
	b2Body* ball2 = _b2World->CreateBody(&bodyDef);
	circleShape.m_radius = ballsize.width *ballScale * 0.5f / PTM_RATIO;
	ball2->CreateFixture(&fixtureDef);
}

void FixtureCollision::setupFrictionAndFilter()
{
	// 建立產生矩形畫框的按鈕
	auto btnSprite = _csbRoot->getChildByName("rectFrame_btn");
	_rectButton = CButton::create();
	_rectButton->setButtonInfo("dnarrow.png","dnarrowon.png", btnSprite->getPosition());
	_rectButton->setScale(btnSprite->getScale());
	this->addChild(_rectButton,3);
	btnSprite->setVisible(false);
	_iNumofRect = 0;

	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody;
	b2Body *rectbody;
	b2FixtureDef fixtureDef; // 產生 Fixture
	b2PolygonShape rectShape;
	fixtureDef.shape = &rectShape;

	char tmp[20] = "";
	// 設定三個不同顏色代表三個碰撞測試的群組
	for (int i = 1; i <= 3; i++)
	{
		// 取得三個設定碰撞過濾器的靜態物體圖示
		sprintf(tmp, "filter1_%02d", i);
		auto rectSprite = (Sprite *)_csbRoot->getChildByName(tmp);
		bodyDef.userData = rectSprite;
		rectSprite->setColor(filterColor[(i-1)]);	// 使用 filterColor 已經建立的顏色
		Size ts = rectSprite->getContentSize();
		Point loc = rectSprite->getPosition();
		float scaleX = rectSprite->getScaleX();	// 讀取矩形畫框有對 X 軸縮放
		float scaleY = rectSprite->getScaleY();	// 讀取矩形畫框有對 Y 軸縮放

		bodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO); // 設定板子所在的位置
		rectbody = _b2World->CreateBody(&bodyDef); // 在 b2World 中建立實體
		float bw = (ts.width - 4)* scaleX;
		float bh = (ts.height - 4)* scaleY;
		// 設定剛體的範圍是一個 BOX （可以縮放成矩形）
		rectShape.SetAsBox(bw*0.5f / PTM_RATIO, bh*0.5f / PTM_RATIO);
		fixtureDef.filter.categoryBits = 1 << i;
		rectbody->CreateFixture(&fixtureDef);
	}
}

void FixtureCollision::setupSensorAndCollision()
{
	// 設置提醒可以釋放球的燈號圖示
	_light1 = CLight::create();
	_light1->setLightInfo("orange05.png","orange02.png",Point(822,682));
	_light1->setScale(0.75f);
	_light1->setLightStatus(true);
	this->addChild(_light1,5);
	//代表可以釋放球，只要按下產生球的按鈕，這個狀態就設定成 false
	_bReleasingBall = true; 

	// 設置可以產生球的按鈕
	auto btnSprite = _csbRoot->getChildByName("releasingBall_Btn");
	_ballBtn = CButton::create();
	_ballBtn->setButtonInfo("dnarrow.png", "dnarrowon.png", btnSprite->getPosition());
	_ballBtn->setScale(btnSprite->getScale());
	this->addChild(_ballBtn, 5);
	btnSprite->setVisible(false);

	// 以場景中所放置的兩張圖示，名稱分別是 sensor1 與 sensor2
	// 在兩個 sensor 的位置建立靜態物體，不進行碰撞，但使用碰撞事件來感應球是否經過
	// 當球往下經過 sensor1 時，累計加速的次數
	// 直到球離開感應區時，給一個反向力，讓其往上彈出
	// 設定一個變數，讓球是反向彈出時，忽略經過  sensor1 的碰撞事件
	// 當球只要經過 sensor2，就讓 _bReleasingBall 為true 代表可以釋放下一個球

	// 設定兩個靜態物體當成感應器
	char tmp[20] = "";
	for (int i = 1; i <= 2; i++)
	{
		sprintf(tmp, "sensor%02d", i);
		auto sensorSprite = (Sprite *)_csbRoot->getChildByName(tmp);
		Point loc = sensorSprite->getPosition();
		Size  size = sensorSprite->getContentSize();
		float scale = sensorSprite->getScale();
		sensorSprite->setVisible(false);
		b2BodyDef sensorBodyDef;
		sensorBodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
		sensorBodyDef.type = b2_staticBody;

		b2Body* SensorBody = _b2World->CreateBody(&sensorBodyDef);
		b2PolygonShape sensorShape;
		sensorShape.SetAsBox(size.width *0.5f * scale / PTM_RATIO, size.height*0.5f*scale / PTM_RATIO);

		b2FixtureDef SensorFixtureDef;
		SensorFixtureDef.shape = &sensorShape;
		SensorFixtureDef.isSensor = true;	// 設定為 Sensor
		SensorFixtureDef.density = 9999+i; // 故意設定成這個值，方便碰觸時候的判斷
		SensorBody->CreateFixture(&SensorFixtureDef);
	}

	// 建立 _collisionSprite 以展示碰撞發生時的處裡
	// ball1 with density=10
	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	b2CircleShape circleShape;

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &circleShape;
	fixtureDef.density = 1.0f;
	fixtureDef.friction = 0.5f;
	fixtureDef.restitution = 0.5f;

	_collisionSprite = (Sprite *)_csbRoot->getChildByName("collisionSprite");
	bodyDef.userData = _collisionSprite; // body 與 Sprite 連結

	Point ballloc = _collisionSprite->getPosition();
	Size ballsize = _collisionSprite->getContentSize();
	float ballScale = _collisionSprite->getScale();
	// 根據 Sprite 所在位置設定圓形在 b2World 中的位置
	bodyDef.position.Set(ballloc.x / PTM_RATIO, ballloc.y / PTM_RATIO);
	b2Body* ball1 = _b2World->CreateBody(&bodyDef);
	// 設定圓形的半徑
	circleShape.m_radius = ballsize.width *ballScale * 0.5f / PTM_RATIO;
	ball1->CreateFixture(&fixtureDef);
	
	_contactListener.setCollisionTarget(*_collisionSprite);
	_tdelayTime = 0; // 第一次一定可以噴發
	_bSparking = true; // 可以噴發
}


void FixtureCollision::doStep(float dt)
{
	int velocityIterations = 8;	// 速度迭代次數
	int positionIterations = 1; // 位置迭代次數 迭代次數一般設定為8~10 越高越真實但效率越差
	// Instruct the world to perform a single step of simulation.
	// It is generally best to keep the time step and iterations fixed.
	_b2World->Step(dt, velocityIterations, positionIterations);

	// 取得 _b2World 中所有的 body 進行處理
	// 最主要是根據目前運算的結果，更新附屬在 body 中 sprite 的位置
	for (b2Body* body = _b2World->GetBodyList(); body; )
	{
		// 以下是以 Body 有包含 Sprite 顯示為例
		if (body->GetUserData() != NULL)
		{
			Sprite *spriteData = (Sprite*)body->GetUserData();
			spriteData->setPosition(body->GetPosition().x*PTM_RATIO, body->GetPosition().y*PTM_RATIO);
			spriteData->setRotation(-1 * CC_RADIANS_TO_DEGREES(body->GetAngle()));
		}
		else {
			if (!_bReleasingBall) {  // 目前球釋放按鈕不能使用
				if (body->GetFixtureList()->GetDensity() == 10002.0f) { // 代表 sensor2 有球經過，重新開放球按鈕功能
					body->GetFixtureList()->SetDensity(10001.0f);
					_bReleasingBall = true;
					_light1->setLightStatus(_bReleasingBall);
				}
			}			
		}
		// 跑出螢幕外面就讓物體從 b2World 中移除
		if ( body->GetType() == b2BodyType::b2_dynamicBody ) {
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
				g_totRemoved++;
				CCLOG("Removing %4d Particles", g_totRemoved);
#endif
			}
			else body = body->GetNext(); //否則就繼續更新下一個Body
		}
		else body = body->GetNext(); //否則就繼續更新下一個Body
	}

	// 產生火花
	if (_contactListener._bCreateSpark ) {
		_contactListener._bCreateSpark = false;	//產生完關閉
		// 判斷延遲的時間是否滿足
		if ( _bSparking ) { //可以噴發，實現這次的噴發
			_tdelayTime = 0; // 時間重新設定，
			_bSparking = false; // 開始計時
			for (int i = 0; i < _contactListener._NumOfSparks; i++) {
				// 建立 Spark Sprite 並與目前的物體結合
				auto sparkSprite = Sprite::createWithSpriteFrameName("spark.png");
				sparkSprite->setColor(Color3B(rand() % 256, rand() % 256, rand() % 156));
				sparkSprite->setBlendFunc(BlendFunc::ADDITIVE);
				this->addChild(sparkSprite, 5);
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
				RectFixtureDef.density = 1.0f;
				RectFixtureDef.isSensor = true;
				b2Fixture*RectFixture = RectBody->CreateFixture(&RectFixtureDef);

				//給力量
				RectBody->ApplyForce(b2Vec2(rand() % 51 - 25, 50+rand() %30), _contactListener._createLoc, true);
			}
#ifdef CREATED_REMOVED
			g_totCreated += _contactListener._NumOfSparks;
			CCLOG("Creating %4d Particles", g_totCreated);
#endif
		}
	}
	if ( !_bSparking ) {
		_tdelayTime += dt;
		if (_tdelayTime >= 0.075f) {
			_tdelayTime = 0; // 歸零
			_bSparking = true; // 可進行下一次的噴發
		}
	}
}

bool FixtureCollision::onTouchBegan(cocos2d::Touch *pTouch, cocos2d::Event *pEvent)//觸碰開始事件
{
	Point touchLoc = pTouch->getLocation();
	_rectButton->touchesBegin(touchLoc);
	_ballBtn->touchesBegin(touchLoc);
	return true;
}

void  FixtureCollision::onTouchMoved(cocos2d::Touch *pTouch, cocos2d::Event *pEvent) //觸碰移動事件
{
	Point touchLoc = pTouch->getLocation();
	_rectButton->touchesMoved(touchLoc);
	_ballBtn->touchesBegin(touchLoc);

}

void  FixtureCollision::onTouchEnded(cocos2d::Touch *pTouch, cocos2d::Event *pEvent) //觸碰結束事件 
{
	Point touchLoc = pTouch->getLocation();
	if (_rectButton->touchesEnded(touchLoc) && _iNumofRect < 9 ) { // 釋放一個長方形
		// 在 (518,680) 產生一個長方形，然後給一個顏色
		auto rectSprite = Sprite::createWithSpriteFrameName("frame04.png");
		rectSprite->setScale(0.75f);
		rectSprite->setColor(filterColor[_iNumofRect %3]);
		this->addChild(rectSprite, 2);

		b2BodyDef bodyDef;
		bodyDef.type = b2_dynamicBody;
		bodyDef.position.Set(518.0f / PTM_RATIO, 680.0f / PTM_RATIO);
		bodyDef.userData = rectSprite;
		b2Body *body = _b2World->CreateBody(&bodyDef);

		// Define poly shape for our dynamic body.
		b2PolygonShape rectShape;
		Size rectSize = rectSprite->getContentSize();
		rectShape.SetAsBox((rectSize.width-4)*0.5f*0.75f / PTM_RATIO, (rectSize.height-4)*0.5f*0.75f / PTM_RATIO);
		// Define the dynamic body fixture.
		b2FixtureDef fixtureDef;
		fixtureDef.shape = &rectShape;
		fixtureDef.restitution = 0.1f;
		fixtureDef.density = 1.0f;
		fixtureDef.friction = (_iNumofRect +1)*0.1f;

		// 所有 BOX2D 物件的 filter.categoryBits 預設都是 1
		fixtureDef.filter.maskBits = 1 << (_iNumofRect % 3 + 1) | 1;
		body->CreateFixture(&fixtureDef);

		_iNumofRect++;
	}

	// 當按鈕確定被按下，而且允許釋放球的時候，才能放出一顆球
	if (_ballBtn->touchesEnded(touchLoc) && _bReleasingBall ) {
		_bReleasingBall = false; // 直到球經過 sensor2 後才會再次開啟
		_light1->setLightStatus(_bReleasingBall);

		// 隨機選擇一顆球，從 (1247,386) 處讓其自由落下 ，大小縮成50%
		auto ballSprite = Sprite::createWithSpriteFrameName(g_CircleObject[rand() % MAX_CIRCLE_OBJECTS]);
		ballSprite->setScale(0.5f);
		//	ballSprite->setPosition(touchLoc);
		this->addChild(ballSprite, 2);

		// 建立一個簡單的動態球體
		b2BodyDef bodyDef;	// 先以結構 b2BodyDef 宣告一個 Body 的變數
		bodyDef.type = b2_dynamicBody; // 設定為動態物體
		bodyDef.userData = ballSprite;	// 設定 Sprite 為動態物體的顯示圖示
		bodyDef.position.Set(1247.0f / PTM_RATIO, 386.0f / PTM_RATIO);
		// 以 bodyDef 在 b2World  中建立實體並傳回該實體的指標
		b2Body *ballBody = _b2World->CreateBody(&bodyDef);

		// 設定該物體的外型
		b2CircleShape ballShape;	//  宣告物體的外型物件變數，此處是圓形物體
		Size ballsize = ballSprite->getContentSize();	// 根據 Sprite 圖形的大小來設定圓形的半徑
		ballShape.m_radius = 0.5f*(ballsize.width - 4) *0.5f / PTM_RATIO;
		// 以 b2FixtureDef  結構宣告剛體結構變數，並設定剛體的相關物理係數
		b2FixtureDef fixtureDef;
		fixtureDef.shape = &ballShape;			// 指定剛體的外型為圓形
		fixtureDef.restitution = 0.75f;			// 設定彈性係數
		fixtureDef.density = 1.0f;				// 設定密度
		fixtureDef.friction = 0.15f;			// 設定摩擦係數
		ballBody->CreateFixture(&fixtureDef);	// 在 Body 上產生這個剛體的設定
	}
}

void FixtureCollision::setStaticWalls()
{
	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody; // 設定這個 Body 為 靜態的
	bodyDef.userData = NULL;
	// 在 b2World 中產生該 Body, 並傳回產生的 b2Body 物件的指標
	// 產生一次，就可以讓後面所有的 Shape 使用
	b2Body *staticbody = _b2World->CreateBody(&bodyDef);

	// 產生靜態邊界所需要的 EdgeShape，名稱都是 wall 為開頭
	b2EdgeShape edgeShape;
	b2FixtureDef fixtureDef; // 產生 Fixture
	fixtureDef.shape = &edgeShape;
	fixtureDef.friction = 0.1f;
	char tmp[20] = "";
	Point pntLoc = _csbRoot->getPosition();

	for (size_t i = 1; i <= 50; i++)
	{
		// 產生所需要的 Sprite file name int plist 
		// 此處取得的都是相對於 csbRoot 所在位置的相對座標
		// 在計算 edgeShape 的相對應座標時，必須進行轉換
		sprintf(tmp, "wall1_%02d", i);
		auto edgeSprite = (Sprite *)_csbRoot->getChildByName(tmp);
		Size ts = edgeSprite->getContentSize();
		Point loc = edgeSprite->getPosition();
		float angle = edgeSprite->getRotation();
		float scale = edgeSprite->getScaleX();	// 水平的線段圖示假設都只有對 X 軸放大

		Point lep1, lep2, wep1, wep2; // EdgeShape 的兩個端點
		lep1.y = 0; lep1.x = -(ts.width - 4) / 2.0f;
		lep2.y = 0; lep2.x = (ts.width - 4) / 2.0f;

		// 所有的線段圖示都是是本身的中心點為 (0,0)，
		// 根據縮放、旋轉產生所需要的矩陣
		// 根據寬度計算出兩個端點的座標，然後呈上開矩陣
		// 然後進行旋轉，
		// Step1: 先CHECK 有無旋轉，有旋轉則進行端點的計算
		cocos2d::Mat4 modelMatrix, rotMatrix;
		modelMatrix.m[0] = scale;  // 先設定 X 軸的縮放
		cocos2d::Mat4::createRotationZ(angle*M_PI / 180.0f, &rotMatrix);
		modelMatrix.multiply(rotMatrix);
		modelMatrix.m[3] = pntLoc.x + loc.x; //設定 Translation，自己的加上父親的
		modelMatrix.m[7] = pntLoc.y + loc.y; //設定 Translation，自己的加上父親的

											 // 產生兩個端點
		wep1.x = lep1.x * modelMatrix.m[0] + lep1.y * modelMatrix.m[1] + modelMatrix.m[3];
		wep1.y = lep1.x * modelMatrix.m[4] + lep1.y * modelMatrix.m[5] + modelMatrix.m[7];
		wep2.x = lep2.x * modelMatrix.m[0] + lep2.y * modelMatrix.m[1] + modelMatrix.m[3];
		wep2.y = lep2.x * modelMatrix.m[4] + lep2.y * modelMatrix.m[5] + modelMatrix.m[7];

		// bottom edge
		edgeShape.Set(b2Vec2(wep1.x / PTM_RATIO, wep1.y / PTM_RATIO), b2Vec2(wep2.x / PTM_RATIO, wep2.y / PTM_RATIO));
		staticbody->CreateFixture(&fixtureDef);
	}

}

void FixtureCollision::createStaticBoundary()
{
	// 先產生 Body, 設定相關的參數

	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody; // 設定這個 Body 為 靜態的
	bodyDef.userData = NULL;
	// 在 b2World 中產生該 Body, 並傳回產生的 b2Body 物件的指標
	// 產生一次，就可以讓後面所有的 Shape 使用
	b2Body *body = _b2World->CreateBody(&bodyDef);

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
void FixtureCollision::draw(Renderer *renderer, const Mat4 &transform, uint32_t flags)
{
	Director* director = Director::getInstance();

	GL::enableVertexAttribs(cocos2d::GL::VERTEX_ATTRIB_FLAG_POSITION);
	director->pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
	_b2World->DrawDebugData();
	director->popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
}
#endif



CContactListener::CContactListener()
{
	_bApplyImpulse = false;
	_bCreateSpark = false;
	_NumOfSparks = 5;
}
void CContactListener::setCollisionTarget(cocos2d::Sprite &targetSprite)
{
	_targetSprite = &targetSprite;
}

//
// 只要是兩個 body 的 fixtures 碰撞，就會呼叫這個函式
//
void CContactListener::BeginContact(b2Contact* contact)
{
	b2Body* BodyA = contact->GetFixtureA()->GetBody();
	b2Body* BodyB = contact->GetFixtureB()->GetBody();

	// check 是否為落下的球經過 sensor1 ，只要經過就立刻讓他彈出去
	if (BodyA->GetFixtureList()->GetDensity() == 10000.0f ) { // 代表 sensor1
		BodyB->ApplyLinearImpulse(b2Vec2(0, 50 + rand() % 101), BodyB->GetWorldCenter(), true);
		_bApplyImpulse = true;
	}
	else if (BodyB->GetFixtureList()->GetDensity() == 10000.0f) {// 代表 sensor1
		BodyA->ApplyLinearImpulse(b2Vec2(0, 50+rand()%101), BodyB->GetWorldCenter(), true);
		_bApplyImpulse = true;
	}

	if ( BodyA->GetUserData() == _targetSprite ) {
		float lengthV = BodyB->GetLinearVelocity().Length();
		if (lengthV >= 4.25f) { // 接觸時的速度超過一定的值才噴出火花
			_bCreateSpark = true;
			_createLoc = BodyA->GetWorldCenter() + b2Vec2(0, -30 / PTM_RATIO);
		}
	}
	else if ( BodyB->GetUserData() == _targetSprite) {
		float lengthV = BodyB->GetLinearVelocity().Length();
		if (lengthV >= 4.25f) { // 接觸時的速度超過一定的值才噴出火花
			_bCreateSpark = true;
			_createLoc = BodyB->GetWorldCenter() + b2Vec2(0, -30 / PTM_RATIO);
		}
	}
}

//碰撞結束
void CContactListener::EndContact(b2Contact* contact)
{
	b2Body* BodyA = contact->GetFixtureA()->GetBody();
	b2Body* BodyB = contact->GetFixtureB()->GetBody();

	if (BodyA->GetFixtureList()->GetDensity() == 10001.0f && _bApplyImpulse ) { // 代表 sensor2
		BodyA->GetFixtureList()->SetDensity(10002);
		_bApplyImpulse = false;
	}
	else if (BodyB->GetFixtureList()->GetDensity() == 10001.0f && _bApplyImpulse ) {	// 代表 sensor2
		BodyB->GetFixtureList()->SetDensity(10002);
		_bApplyImpulse = false;
	}
}