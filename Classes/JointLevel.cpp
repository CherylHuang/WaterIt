#include "JointLevel.h"
#include "cocostudio/CocoStudio.h"
#include "ui/CocosGUI.h"

#define  CREATED_REMOVED
#ifdef CREATED_REMOVED
int g_totCreated2 = 0, g_totRemoved2 = 0;
#endif

USING_NS_CC;

#define MAX_2(X,Y) (X)>(Y) ? (X) : (Y)

#define StaticAndDynamicBodyExample 1
using namespace cocostudio::timeline;
using namespace ui;

JointLevel::~JointLevel()
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

Scene* JointLevel::createScene()
{
	auto scene = Scene::create();
	auto layer = JointLevel::create();
	scene->addChild(layer);
	return scene;
}

// on "init" you need to initialize your instance
bool JointLevel::init()
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
	//畫筆
	_iFree = LINE_LENGTH;
	_iInUsed = 0;
	_bDrawing = false;
	//----------------------------------------------------------
	//Joint點碰撞
	_bJointCount = false;
	_fcount = 0;
	//----------------------------------------------------------
	// 建立 Box2D world
	_b2World = nullptr;
	b2Vec2 Gravity = b2Vec2(0.0f, -9.8f);		//重力方向
	bool AllowSleep = true;			//允許睡著
	_b2World = new b2World(Gravity);	//創建世界
	_b2World->SetAllowSleeping(AllowSleep);	//設定物件允許睡著

											// Create Scene with csb file
	_csbRoot = CSLoader::createNode("JointLevel.csb");
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

	//----------------------------------------------------------
	//Start & End
	_fStartCount = 0;
	_contactListener._StartSprite = _startWater;
	_contactListener._EndSprite = _Bush_dry;
	_startPt = _startWater->getPosition();
	_fWinCount = 0;

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

	//----------------------------------------------------------
	setStartEndpoint();			//起終點
	createStaticBoundary();		//邊界
	setupStatic();				//靜態物體
	setJointDot();				//手繪物體Joint點

	setupMouseJoint();

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
	_listener1->onTouchBegan = CC_CALLBACK_2(JointLevel::onTouchBegan, this);		//加入觸碰開始事件
	_listener1->onTouchMoved = CC_CALLBACK_2(JointLevel::onTouchMoved, this);		//加入觸碰移動事件
	_listener1->onTouchEnded = CC_CALLBACK_2(JointLevel::onTouchEnded, this);		//加入觸碰離開事件

	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(_listener1, this);	//加入剛創建的事件聆聽器
	this->schedule(CC_SCHEDULE_SELECTOR(JointLevel::doStep));

	return true;
}

void JointLevel::setupStatic()
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

	for (size_t i = 1; i <= 6; i++)
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

void JointLevel::setJointDot()
{
	// 取得並設定 JointDot 圖示為【靜態物體】
	auto DotSprite = (Sprite *)_csbRoot->getChildByName("JointDot");
	Point loc = DotSprite->getPosition();
	Size size = DotSprite->getContentSize();
	b2BodyDef dotBodyDef;
	dotBodyDef.type = b2_staticBody;
	dotBodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	dotBodyDef.userData = DotSprite;
	_dotBody = _b2World->CreateBody(&dotBodyDef);

	b2PolygonShape boxShape;
	boxShape.SetAsBox(size.width*0.5f / PTM_RATIO, size.height*0.5f / PTM_RATIO);

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &boxShape;
	fixtureDef.isSensor = true; // 設定為 Sensor
	_dotBody->CreateFixture(&fixtureDef);

	_contactListener.setCollisionTarget(*DotSprite);
}
void JointLevel::setStartEndpoint()
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

void JointLevel::setupMouseJoint()
{
	// 取得並設定 frame01 畫框圖示為動態物件
	auto frameSprite = _csbRoot->getChildByName("frame01");
	Point loc = frameSprite->getPosition();

	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	bodyDef.userData = frameSprite;
	b2Body *body = _b2World->CreateBody(&bodyDef);

	// Define poly shape for our dynamic body.
	b2PolygonShape rectShape;
	Size frameSize = frameSprite->getContentSize();
	rectShape.SetAsBox((frameSize.width - 4)*0.5f / PTM_RATIO, (frameSize.height - 4)*0.5f / PTM_RATIO);
	// Define the dynamic body fixture.
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &rectShape;
	fixtureDef.restitution = 0.1f;
	fixtureDef.density = 1.0f;
	fixtureDef.friction = 0.1f;
	body->CreateFixture(&fixtureDef);

	//_test = body;
	_bTouchOn = false;
}

void JointLevel::DrawLine(cocos2d::Point prePt, cocos2d::Point Pt, cocos2d::Color4F color, int num)
{
	//畫出線段
	_draw[num] = DrawNode::create();
	_draw[num]->drawLine(prePt, Pt, color);
	_newNode->addChild(_draw[num], 5);
	//CCLOG("%d", num);
}

void JointLevel::doStep(float dt)
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
			spriteData->setPosition(body->GetPosition().x*PTM_RATIO, body->GetPosition().y*PTM_RATIO);
			spriteData->setRotation(-1 * CC_RADIANS_TO_DEGREES(body->GetAngle()));
		}
//		// 跑出螢幕外面就讓物體從 b2World 中移除
//		if (body->GetType() == b2BodyType::b2_dynamicBody) {
//			float x = body->GetPosition().x * PTM_RATIO;
//			float y = body->GetPosition().y * PTM_RATIO;
//			if (x > _visibleSize.width || x < 0 || y >  _visibleSize.height || y < 0) {
//				if (body->GetUserData() != NULL) {
//					Sprite* spriteData = (Sprite *)body->GetUserData();
//					this->removeChild(spriteData);
//				}
//				b2Body* nextbody = body->GetNext(); // 取得下一個 body
//				_b2World->DestroyBody(body); // 釋放目前的 body
//				body = nextbody;  // 讓 body 指向剛才取得的下一個 body
//#ifdef CREATED_REMOVED
//				g_totRemoved2++;
//				CCLOG("Removing %4d Particles", g_totRemoved2);
//#endif
//			}
//			else body = body->GetNext(); //否則就繼續更新下一個Body
//		}
//		else body = body->GetNext(); //否則就繼續更新下一個Body
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
		auto Scene = GearLevel::createScene();
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
	// for drawing
	if (_bJointCount) _fcount += dt;
	if (_b2Linebody != nullptr) {
		if (_contactListener._bRevoluteJoint && _fcount > 0.01f) {		//有和連結點碰撞，產生連結
			_bJointCount = false; //取消計時
			_fcount = 0; //重設計時

			b2RevoluteJointDef revoluteJoint;		// 建立旋轉 Joint 連結
			revoluteJoint.bodyA = _dotBody;
			revoluteJoint.localAnchorA.Set(0, 0);
			revoluteJoint.bodyB = _b2Linebody;
			auto pos = _b2Linebody->GetPosition();
			revoluteJoint.localAnchorB.Set(pos.x + 640.f / PTM_RATIO, pos.y + 360.f / PTM_RATIO);
			_b2World->CreateJoint(&revoluteJoint);
		}
		else if (!(_contactListener._bRevoluteJoint) && _fcount > 0.01f) {	//沒有和連結碰撞，產生egde鋼體
			_bJointCount = false;
			_fcount = 0;

			for (b2Fixture* f = _b2Linebody->GetFixtureList(); f; ) //移除目前body身上所有fixture
			{
				b2Fixture* fixtureToDestroy = f;
				f = f->GetNext();
				_b2Linebody->DestroyFixture(fixtureToDestroy);
			}

			//手繪【線條】----------------------------------------------

			for (int i = _istartPt; _draw[i] != nullptr && i < LINE_LENGTH - 1; i++) { //取得並設定 線段 為【動態物體】
				b2EdgeShape edgeShape;
				b2FixtureDef edgeFixtureDef; // 產生 Fixture
				edgeFixtureDef.shape = &edgeShape;
				edgeShape.Set(b2Vec2(_pt[i].x / PTM_RATIO, _pt[i].y / PTM_RATIO), b2Vec2(_pt[i + 1].x / PTM_RATIO, _pt[i + 1].y / PTM_RATIO)); //畫圖線段
				_b2Linebody->CreateFixture(&edgeFixtureDef);
			}
		}
	}
	// ---------------------------------------------------------------------------
	// 產生繪圖效果
	if (_bDrawing) {	//畫圖中
		if (_iFree > 0) {			//黑筆使用中
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

}

bool JointLevel::onTouchBegan(cocos2d::Touch *pTouch, cocos2d::Event *pEvent)//觸碰開始事件
{
	Point touchLoc = pTouch->getLocation();
	_tp = touchLoc;
	//------------------------------------------------------------
	// For Mouse Joint 
	for (b2Body* body = _b2World->GetBodyList(); body; body = body->GetNext())
	{
		if (body->GetUserData() == NULL) continue; // 靜態物體不處理
												   // 判斷點的位置是否落在動態物體一定的範圍
		Sprite *spriteObj = (Sprite*)body->GetUserData();
		Size objSize = spriteObj->getContentSize();
		float fdist = MAX_2(objSize.width, objSize.height) / 2.0f;
		float x = body->GetPosition().x*PTM_RATIO - touchLoc.x;
		float y = body->GetPosition().y*PTM_RATIO - touchLoc.y;
		float tpdist = x*x + y*y;
		if (tpdist < fdist*fdist)
		{
			_bTouchOn = true;
			b2MouseJointDef mouseJointDef;
			mouseJointDef.bodyA = _bottomBody;
			mouseJointDef.bodyB = body;
			mouseJointDef.target = b2Vec2(touchLoc.x / PTM_RATIO, touchLoc.y / PTM_RATIO);
			mouseJointDef.collideConnected = true;
			mouseJointDef.maxForce = 1000.0f * body->GetMass();
			_MouseJoint = (b2MouseJoint*)_b2World->CreateJoint(&mouseJointDef); // 新增 Mouse Joint
			body->SetAwake(true);
			break;
		}
	}
	//-------------------------------------------------------------------
	//For button
	if (_retrybtn->onTouchBegan(touchLoc) || _homebtn->onTouchBegan(touchLoc)) _bBtnPressed = true;

	//-------------------------------------------------------------------
	//For drawing
	if (!_bTouchOn && !_bBtnPressed)_bDrawing = true;	//不在按鈕上才可畫圖
	if (_bDrawing && _iFree > 0) {
		_newNode = Node::create(); //新增此筆畫控制節點
		this->addChild(_newNode, 5);
		_contactListener._newNode = _newNode; //傳入Box2d world

		_pt[_iInUsed] = touchLoc;
		_b2vec[_iInUsed].x = touchLoc.x / PTM_RATIO;
		_b2vec[_iInUsed].y = touchLoc.y / PTM_RATIO;
		_istartPt = _iInUsed;
		_iInUsed++; _iFree--;
	}

	return true;
}

void  JointLevel::onTouchMoved(cocos2d::Touch *pTouch, cocos2d::Event *pEvent) //觸碰移動事件
{
	Point touchLoc = pTouch->getLocation();
	_tp = touchLoc;
	//------------------------------------------------------------
	if (_bDrawing && _iFree > 0) {
		DrawLine(_pt[_iInUsed - 1], touchLoc, Color4F::BLACK, _iInUsed - 1); //畫線函式
		_b2vec[_iInUsed].x = touchLoc.x / PTM_RATIO;
		_b2vec[_iInUsed].y = touchLoc.y / PTM_RATIO;
		_pt[_iInUsed] = touchLoc;
		_iInUsed++; _iFree--;
		_penBlackBar->setPercent((float)_iFree / (float)LINE_LENGTH * 100.f); //墨水量
	}
	//-------------------------------------------------------------------
	//For button
	_retrybtn->onTouchMoved(touchLoc);
	_homebtn->onTouchMoved(touchLoc);

	//-------------------------------------------------------------------
	if (_bTouchOn)
	{
		_MouseJoint->SetTarget(b2Vec2(touchLoc.x / PTM_RATIO, touchLoc.y / PTM_RATIO));
	}
}

void  JointLevel::onTouchEnded(cocos2d::Touch *pTouch, cocos2d::Event *pEvent) //觸碰結束事件 
{
	Point touchLoc = pTouch->getLocation();
	_tp = touchLoc;
	//----------------------------------------------------------------
	if (_bDrawing) {

		//建立 手繪【動態物體】
		Point loc = _newNode->getPosition();
		Size size = _newNode->getContentSize();
		b2BodyDef bodyDef;				//body只產生一次
		bodyDef.type = b2_dynamicBody;
		bodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
		bodyDef.userData = _newNode;
		_b2Linebody = _b2World->CreateBody(&bodyDef);

		//手繪【多邊形】--------------------------------------------
		_b2polyVec[0] = _b2vec[_istartPt]; //三角形頂點
		for (int i = _istartPt + 1; _draw[i] != nullptr && i < LINE_LENGTH - 1; i++) { //取得並設定 線段 為【動態物體】
			b2PolygonShape polyShape;
			b2FixtureDef fixtureDef; // 產生 Fixture
			fixtureDef.shape = &polyShape;

			_b2polyVec[1] = _b2vec[i]; //每一線段與起點產生三角形
			_b2polyVec[2] = _b2vec[i + 1];

			polyShape.Set(_b2polyVec, 3); //polygon中的小三角形
			_b2Linebody->CreateFixture(&fixtureDef); //加入主body
		}
		_bJointCount = true; //延遲計時
		_bDrawing = false;
	}
	_contactListener._bRevoluteJoint = false;	//重新偵測碰撞

	//---------------------------------------------------------------
	//Button
	_bBtnPressed = false;
	if (_retrybtn->onTouchEnded(touchLoc)) {		//重新遊戲鈕
		auto Scene = JointLevel::createScene();
		Director::getInstance()->replaceScene(TransitionFade::create(1, Scene, Color3B(255, 255, 255)));
	}
	if (_homebtn->onTouchEnded(touchLoc)) {			//回首頁鈕
		auto mainScene = MainMenu::createScene();
		Director::getInstance()->replaceScene(TransitionFade::create(1, mainScene, Color3B(255, 255, 255)));
	}

	//---------------------------------------------------------------
	if (_bTouchOn)
	{
		_bTouchOn = false;
		if (_MouseJoint != NULL)
		{
			_b2World->DestroyJoint(_MouseJoint);
			_MouseJoint = NULL;
		}
	}
}

void JointLevel::createStaticBoundary()
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
void JointLevel::draw(Renderer *renderer, const Mat4 &transform, uint32_t flags)
{
	Director* director = Director::getInstance();

	GL::enableVertexAttribs(cocos2d::GL::VERTEX_ATTRIB_FLAG_POSITION);
	director->pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
	_b2World->DrawDebugData();
	director->popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
}
#endif

//================================================================================
CContactListenerLevel1::CContactListenerLevel1()
{
	_bWin = false;
	_bRevoluteJoint = false;
	_bCreateSpark = false;
	//_NumOfSparks = 20;	//噴發分子數
}
void CContactListenerLevel1::setCollisionTarget(cocos2d::Sprite &targetSprite)
{
	_targetSprite = &targetSprite;
}

// 只要是兩個 body 的 fixtures 碰撞，就會呼叫這個函式
void CContactListenerLevel1::BeginContact(b2Contact* contact)
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
	//Joint點偵測
	if (BodyA->GetUserData() == _targetSprite) { //A:dot, B:polygon
		auto shape = BodyB->GetFixtureList()->GetBody()->GetUserData();
		if (shape == _newNode) {
			_bRevoluteJoint = true;
		}
	}
	else if (BodyB->GetUserData() == _targetSprite) { //A:polygon, B:dot
		auto shape = BodyA->GetFixtureList()->GetBody()->GetUserData();
		if (shape == _newNode) {
			_bRevoluteJoint = true;
		}
	}

}

//碰撞結束
void CContactListenerLevel1::EndContact(b2Contact* contact)
{

}