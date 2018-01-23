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

int MainMenu::level = 0; //�R�A�ܼƪ�l��

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
	AnimationCache::destroyInstance();  // ���� AnimationCache ���o���귽
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

	//Ū�J����
	SpriteFrameCache::getInstance()->addSpriteFramesWithFile("box2d_pic.plist");
	SpriteFrameCache::getInstance()->addSpriteFramesWithFile("box2d_bg_pic.plist");

	//�ܼƪ�l��
	settingVisible = false;
	level = 1; //�S�������d

	//Ū������
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
	auto BtnPos = dynamic_cast<Button*>(_rootNode->getChildByName("btn_start")); //�}�l�s
	Point loc = BtnPos->getPosition();
	BtnPos->setVisible(false);
	_startbtn = C3SButton::create();
	_startbtn->setButtonInfo(NORMAL_BTN, "btn_start_0.png", "btn_start_2.png", "btn_start_1.png", loc);
	_startbtn->setScale(0.5f);
	this->addChild(_startbtn, 1);
	
	BtnPos = dynamic_cast<Button*>(_rootNode->getChildByName("btn_setting")); //�]�w�s
	loc = BtnPos->getPosition();
	BtnPos->setVisible(false);
	_settingbtn = C3SButton::create();
	_settingbtn->setButtonInfo(NORMAL_BTN, "btn_setting_0.png", "btn_setting_2.png", "btn_setting_1.png", loc);
	_settingbtn->setScale(0.5f);
	this->addChild(_settingbtn, 1);

	BtnPos = dynamic_cast<Button*>(_rootNode->getChildByName("btn_exit")); //���}�s
	loc = BtnPos->getPosition();
	BtnPos->setVisible(false);
	_exitbtn = C3SButton::create();
	_exitbtn->setButtonInfo(NORMAL_BTN, "btn_exit_0.png", "btn_exit_2.png", "btn_exit_1.png", loc);
	_exitbtn->setScale(0.5f);
	this->addChild(_exitbtn, 1);

	BtnPos = dynamic_cast<Button*>(_rootNode->getChildByName("btn_okay")); //OK�s
	loc = BtnPos->getPosition();
	BtnPos->setVisible(false);
	_okaybtn = C3SButton::create();
	_okaybtn->setButtonInfo(NORMAL_BTN, "btn_okay_0.png", "btn_okay_2.png", "btn_okay_1.png", loc);
	_okaybtn->setScale(0.5f);
	_okaybtn->setVisible(false);
	this->addChild(_okaybtn, 4);

	//Type 2
	BtnPos = dynamic_cast<Button*>(_rootNode->getChildByName("btn_easy")); //LEVEL1�s
	loc = BtnPos->getPosition();
	BtnPos->setVisible(false);
	_easybtn = C3SButton::create();
	_easybtn->setButtonInfo(SELECT_BTN, "btn_level1_0.png", "btn_level1_2.png", "btn_level1_1.png", loc);
	_easybtn->setScale(0.4f);
	_easybtn->setVisible(false);
	this->addChild(_easybtn, 3);

	BtnPos = dynamic_cast<Button*>(_rootNode->getChildByName("btn_normal")); //LEVEL2�s
	loc = BtnPos->getPosition();
	BtnPos->setVisible(false);
	_normalbtn = C3SButton::create();
	_normalbtn->setButtonInfo(SELECT_BTN, "btn_level2_0.png", "btn_level2_2.png", "btn_level2_1.png", loc);
	_normalbtn->setScale(0.4f);
	_normalbtn->setVisible(false);
	this->addChild(_normalbtn, 3);

	BtnPos = dynamic_cast<Button*>(_rootNode->getChildByName("btn_hard")); //LEVEL3�s
	loc = BtnPos->getPosition();
	BtnPos->setVisible(false);
	_hardbtn = C3SButton::create();
	_hardbtn->setButtonInfo(SELECT_BTN, "btn_level3_0.png", "btn_level3_2.png", "btn_level3_1.png", loc);
	_hardbtn->setScale(0.4f);
	_hardbtn->setVisible(false);
	this->addChild(_hardbtn, 3);

	BtnPos = dynamic_cast<Button*>(_rootNode->getChildByName("btn_final")); //LEVEL4�s
	loc = BtnPos->getPosition();
	BtnPos->setVisible(false);
	_finalbtn = C3SButton::create();
	_finalbtn->setButtonInfo(SELECT_BTN, "btn_level4_0.png", "btn_level4_2.png", "btn_level4_1.png", loc);
	_finalbtn->setScale(0.4f);
	_finalbtn->setVisible(false);
	this->addChild(_finalbtn, 3);
	//---------------------------------------------------------------------------------------------	
	// �إ� Box2D world
	_b2World = nullptr;
	b2Vec2 Gravity = b2Vec2(0.0f, -9.8f);		//���O��V
	bool AllowSleep = true;			//���\�ε�
	_b2World = new b2World(Gravity);	//�Ыإ@��
	_b2World->SetAllowSleeping(AllowSleep);	//�]�w���󤹳\�ε�
	_b2World->SetContactListener(&_contactListener); //�I����ť��

	//---------------------------------------------------------------------------------------------	
	setStartEndpoint();			//�_���I
	setupStatic();				//�R�A����

	_listener1 = EventListenerTouchOneByOne::create();	//�Ыؤ@�Ӥ@��@���ƥ��ť��
	_listener1->onTouchBegan = CC_CALLBACK_2(MainMenu::onTouchBegan, this);		//�[�JĲ�I�}�l�ƥ�
	_listener1->onTouchMoved = CC_CALLBACK_2(MainMenu::onTouchMoved, this);		//�[�JĲ�I���ʨƥ�
	_listener1->onTouchEnded = CC_CALLBACK_2(MainMenu::onTouchEnded, this);		//�[�JĲ�I���}�ƥ�

	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(_listener1, this);	//�[�J��Ыت��ƥ��ť��
	this->schedule(CC_SCHEDULE_SELECTOR(MainMenu::doStep));
	return true;
}

void MainMenu::doStep(float dt)
{
	int velocityIterations = 8;	// �t�׭��N����
	int positionIterations = 1; // ��m���N���� ���N���Ƥ@��]�w��8~10 �V���V�u����Ĳv�V�t
								// Instruct the world to perform a single step of simulation.
								// It is generally best to keep the time step and iterations fixed.
	_b2World->Step(dt, velocityIterations, positionIterations);

	// ���o _b2World ���Ҧ��� body �i��B�z
	// �̥D�n�O�ھڥثe�B�⪺���G�A��s���ݦb body �� sprite ����m
	for (b2Body* body = _b2World->GetBodyList(); body;)
	{
		// �H�U�O�H Body ���]�t Sprite ��ܬ���
		if (body->GetUserData() != NULL)
		{
			Sprite *spriteData = (Sprite*)body->GetUserData();
			spriteData->setPosition(body->GetPosition().x*PTM_RATIO, body->GetPosition().y*PTM_RATIO);
			spriteData->setRotation(-1 * CC_RADIANS_TO_DEGREES(body->GetAngle()));
		}
				// �]�X�ù��~���N������q b2World ������
				if (body->GetType() == b2BodyType::b2_dynamicBody) {
					float x = body->GetPosition().x * PTM_RATIO;
					float y = body->GetPosition().y * PTM_RATIO;
					if (x > _visibleSize.width || x < 0 || y >  _visibleSize.height || y < 0) {
						if (body->GetUserData() != NULL) {
							Sprite* spriteData = (Sprite *)body->GetUserData();
							this->removeChild(spriteData);
						}
						b2Body* nextbody = body->GetNext(); // ���o�U�@�� body
						_b2World->DestroyBody(body); // ����ثe�� body
						body = nextbody;  // �� body ���V��~���o���U�@�� body
		#ifdef CREATED_REMOVED
						g_totRemoved2++;
						CCLOG("Removing %4d Particles", g_totRemoved2);
		#endif
					}
					else body = body->GetNext(); //�_�h�N�~���s�U�@��Body
				}
				else body = body->GetNext(); //�_�h�N�~���s�U�@��Body
	}

	//----------------------------------------------------------------------------
	//for Start & End
	if (!_waterBody->IsAwake() && !_contactListener._bWin)_fStartCount += dt; //���y�R��B�C���|������ : �}�l�p��
	if (_fStartCount > 3.0f) {	//�R��W�L�T��A���m��m
		_fStartCount -= 3.0f;

		_startWater->setVisible(true);
		_Bush_live->setVisible(false);

		_waterBody->SetType(b2_staticBody);
		_waterBody->SetTransform(b2Vec2(_startPt.x / PTM_RATIO, _startPt.y / PTM_RATIO), _waterBody->GetAngle());
		_waterBody->SetType(b2_dynamicBody); //BUG�ץ� : �����]�w��m�|�d��A�ഫType��Body���򸨤U
	}
	if (_contactListener._bWin) {		//�L��
		_startWater->setVisible(false);
		_Bush_live->setVisible(true);
		_fcount += dt;
	}
	if (_fcount > 1.0f) {				//�W�L3�� ���m
		_contactListener._bWin = false;
		_fcount = 0;
	}

#ifdef WATER_PARTICLE
	// ���͹L������
	if (_contactListener._bCreateSpark) {
		_contactListener._bCreateSpark = false;	//���ͧ�����
		for (int i = 0; i < _contactListener._NumOfSparks; i++) {
			// �إ� Spark Sprite �ûP�ثe�����鵲�X
			auto sparkSprite = Sprite::createWithSpriteFrameName("pen.png");
			sparkSprite->setColor(Color3B(0, 146 - rand() % 20, 200 - rand() % 20));
			sparkSprite->setBlendFunc(BlendFunc::ADDITIVE);
			this->addChild(sparkSprite, 2);
			//���ͤp������
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

			//���O�q
			RectBody->ApplyForce(b2Vec2(rand() % 51 - 25, 50 + rand() % 30), _contactListener._createLoc, true);
		}
	}
#endif
}

void MainMenu::setupStatic()
{
	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody; // �]�w�o�� Body �� �R�A��
	bodyDef.userData = NULL;
	// �b b2World �����͸� Body, �öǦ^���ͪ� b2Body ���󪺫���
	// ���ͤ@���A�N�i�H���᭱�Ҧ��� Shape �ϥ�
	b2Body *body = _b2World->CreateBody(&bodyDef);

	b2FixtureDef fixtureDef; // ���� Fixture

							 // Ū���Ҧ� wall_ �}�Y���ϥ� ���O�R�A����
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

		// �����R�A��ɩһݭn�� boxShape
		b2PolygonShape boxShape;
		boxShape.SetAsBox((size.width*0.5f - 2.0f)*scale / PTM_RATIO, (size.height*0.5f - 2.0f)*scale / PTM_RATIO);

		b2FixtureDef fixtureDef;
		fixtureDef.shape = &boxShape;
		staticBody->CreateFixture(&fixtureDef);
	}
}

void MainMenu::setStartEndpoint()
{
	// ���o�ó]�w���I Bush_dry �ϥܬ��i�R�A����j
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
	fixtureDef.isSensor = true; // �]�w�� Sensor
	staticBody->CreateFixture(&fixtureDef);

	//----------------------------------------------------
	// ���o�ó]�w�_�I water ���i�ʺA����j
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

bool MainMenu::onTouchBegan(cocos2d::Touch *pTouch, cocos2d::Event *pEvent)//Ĳ�I�}�l�ƥ�
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

void  MainMenu::onTouchMoved(cocos2d::Touch *pTouch, cocos2d::Event *pEvent) //Ĳ�I���ʨƥ�
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

void  MainMenu::onTouchEnded(cocos2d::Touch *pTouch, cocos2d::Event *pEvent) //Ĳ�I�����ƥ� 
{
	Point touchLoc = pTouch->getLocation();
	if (_settingbtn->onTouchEnded(touchLoc) || _okaybtn->onTouchEnded(touchLoc)) { //�]�w&OK btn
		//����3�ӿﶵ
		_settingbtn->setVisible(settingVisible); 
		_startbtn->setVisible(settingVisible);
		_exitbtn->setVisible(settingVisible);

		//��ܳ]�w����
		_okaybtn->setVisible(!settingVisible);
		_easybtn->setVisible(!settingVisible);
		_normalbtn->setVisible(!settingVisible);
		_hardbtn->setVisible(!settingVisible);
		_finalbtn->setVisible(!settingVisible);
		settingVisible = !settingVisible;
		_settingWindow->setVisible(settingVisible);
		_black_bg->setVisible(settingVisible);
	}
	if (_startbtn->onTouchEnded(touchLoc)) { //�}�l�s
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
			auto scene01 = JointLevel::createScene(); //������l����
			Director::getInstance()->replaceScene(TransitionFade::create(1, scene01, Color3B(255, 255, 255)));
		}
	}
	if (_exitbtn->onTouchEnded(touchLoc)) { //���}�s
		unscheduleAllCallbacks();  // �N�Ҧ����U�b Schedule �����{�ǳ�����
		Director::getInstance()->end(); // �פ� Director ������
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
	_NumOfSparks = 20;	//�Q�o���l��
}

// �u�n�O��� body �� fixtures �I���A�N�|�I�s�o�Ө禡
void CContactListenerMainMenu::BeginContact(b2Contact* contact)
{
	b2Body* BodyA = contact->GetFixtureA()->GetBody();
	b2Body* BodyB = contact->GetFixtureB()->GetBody();

	//�L������
	if (BodyA->GetUserData() == _EndSprite) { //A:End, B:Start
		auto shape = BodyB->GetFixtureList()->GetBody()->GetUserData();
		if (shape == _StartSprite) {
			_bWin = true;
			_bCreateSpark = true;
			_createLoc = BodyA->GetWorldCenter() + b2Vec2(0, -38.f / PTM_RATIO); //���o�Q�o�I
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

//�I������
void CContactListenerMainMenu::EndContact(b2Contact* contact)
{

}