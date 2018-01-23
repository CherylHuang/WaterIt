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
    
	// �إ� Box2D world
	_b2World = nullptr;
	b2Vec2 Gravity = b2Vec2(0.0f, -9.8f);		//���O��V
	bool AllowSleep = true;			//���\�ε�
	_b2World = new b2World(Gravity);	//�Ыإ@��
	_b2World->SetAllowSleeping(AllowSleep);	//�]�w���󤹳\�ε�

	// Create Scene with csb file
	_csbRoot = CSLoader::createNode("FixtureCollision.csb");
#ifndef BOX2D_DEBUG
	// �]�w��ܭI���ϥ�
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
	//�]�wDebugDraw
	_b2World->SetDebugDraw(_DebugDraw);
	//���ø�s���O
	uint32 flags = 0;
	flags += GLESDebugDraw::e_shapeBit;						//ø�s�Ϊ�
	flags += GLESDebugDraw::e_pairBit;						
	flags += GLESDebugDraw::e_jointBit;
	flags += GLESDebugDraw::e_centerOfMassBit;
	flags += GLESDebugDraw::e_aabbBit;
	//�]�wø�s����
	_DebugDraw->SetFlags(flags);
#endif

	_b2World->SetContactListener(&_contactListener);

	_listener1 = EventListenerTouchOneByOne::create();	//�Ыؤ@�Ӥ@��@���ƥ��ť��
	_listener1->onTouchBegan = CC_CALLBACK_2(FixtureCollision::onTouchBegan, this);		//�[�JĲ�I�}�l�ƥ�
	_listener1->onTouchMoved = CC_CALLBACK_2(FixtureCollision::onTouchMoved, this);		//�[�JĲ�I���ʨƥ�
	_listener1->onTouchEnded = CC_CALLBACK_2(FixtureCollision::onTouchEnded, this);		//�[�JĲ�I���}�ƥ�

	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(_listener1, this);	//�[�J��Ыت��ƥ��ť��
	this->schedule(CC_SCHEDULE_SELECTOR(FixtureCollision::doStep));

    return true;
}

void FixtureCollision::setupDesnity()
{
	Point pntLoc = _csbRoot->getPosition();

	b2BodyDef bodyDef;
	bodyDef.userData = NULL;
	// �b b2World �����͸� Body, �öǦ^���ͪ� b2Body ���󪺫���
	// ���ͤ@���A�N�i�H���᭱�Ҧ��� Shape �ϥ�
	b2FixtureDef fixtureDef; // ���� Fixture
	char tmp[20] = "";

	// ���ͤT�����R�A����һݭn�� triShape
	// �������ߪO���y���T����
	b2Body *seesawBasedbody;
	b2PolygonShape triShape;
	fixtureDef.shape = &triShape;
	fixtureDef.density = 1.0f;
	fixtureDef.friction = 0.2f;
	fixtureDef.restitution = 0.8f;
	for (size_t i = 1; i <= 1; i++)
	{
		// ���ͩһݭn�� Sprite file name int plist 
		// ���B���o�����O�۹�� csbRoot �Ҧb��m���۹�y��
		// �b�p�� edgeShape ���۹����y�ЮɡA�����i���ഫ
		sprintf(tmp, "triangle1_%02d", i);
		auto triSprite = (Sprite *)_csbRoot->getChildByName(tmp);
		Size ts = triSprite->getContentSize();
		Point loc = triSprite->getPosition();
		float scaleX = triSprite->getScaleX();	// �� X �b��j
		float scaleY = triSprite->getScaleY();	// �� Y �b��j

		Point lep[3], wep[3];	// triShape ���T�ӳ��I, 0 ���I�B 1 ���U�B 2 �k�U
		lep[0].x = 0;  lep[0].y = (ts.height - 2) / 2.0f;
		lep[1].x = -(ts.width - 2) / 2.0f; lep[1].y = -(ts.height - 2) / 2.0f;
		lep[2].x = (ts.width - 2) / 2.0f; lep[2].y = -(ts.height - 2) / 2.0f;

		// Step1: ��CHECK ���L����A������h�i����I���p��
		cocos2d::Mat4 modelMatrix, rotMatrix;
		modelMatrix.m[0] = scaleX;  // ���]�w X �b���Y��
		modelMatrix.m[5] = scaleY;  // ���]�w Y �b���Y��
		for (size_t j = 0; j < 3; j++) 
		{   // �ǤJ�Y��P���઺ local space ���y�Эp��
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

	// �������ߪO�A�����ʺA�A�C�@�ӰʺA����b b2World ���������إ߹���
	// �]�w�o�� Body �� �ʺA���A�æb _b2World ���إ߹���A
	bodyDef.type = b2_dynamicBody;
	b2PolygonShape seesawBoardShape;
	fixtureDef.shape = &seesawBoardShape;
	fixtureDef.density = 5.0f;
	fixtureDef.friction = 0.1f;
	fixtureDef.restitution = 0.1f;
	// ���o�N�����ߪO���ϥܡA�ó]�w�� sprite
	auto boardSprite = (Sprite *)_csbRoot->getChildByName("seesawBoard");
	bodyDef.userData = boardSprite;
	Size ts = boardSprite->getContentSize();
	Point loc = boardSprite->getPosition();
	float scaleX = boardSprite->getScaleX();	// ��x�ιϥ� X �b�Y���
	float scaleY = boardSprite->getScaleY();	// ��x�ιϥ� Y �b�Y���

	// �]�w�O�l�Ҧb����m�A�]���O�ϥ� joint �i�H���γ]�w��m
//	bodyDef.position.Set(loc.x/ PTM_RATIO, loc.y/ PTM_RATIO); 
	b2Body *seesawBoardbody = _b2World->CreateBody(&bodyDef); // �b b2World ���إ߹���

	// ��X seesawBoard ���Y��᪺�e��, 4 ���w�d���e�סA�����L���Ϥ����|
	float bw = (ts.width -4)* scaleX; 	
	float bh = (ts.height -4)* scaleY;

	// �]�w���骺�d��O�@�� BOX �]�i�H�Y�񦨯x�Ρ^
	seesawBoardShape.SetAsBox(bw*0.5f / PTM_RATIO, bh*0.5f / PTM_RATIO);
	seesawBoardbody->CreateFixture(&fixtureDef);

	// �إ߻P�򩳤T���Ϊ� Joint �s��
	b2RevoluteJointDef seesawJoint;
	seesawJoint.bodyA = seesawBasedbody;
	seesawJoint.localAnchorA.Set(0, 1.2f);
	seesawJoint.bodyB = seesawBoardbody;
	seesawJoint.localAnchorB.Set(0,0);
	_b2World->CreateJoint(&seesawJoint);

	// Ū���ëإߨ����y������
	// ball1 with density=10
	bodyDef.type = b2_dynamicBody;
	b2CircleShape circleShape;
	fixtureDef.shape = &circleShape;
	fixtureDef.density = 10.0f;
	fixtureDef.friction = 0.2f;
	fixtureDef.restitution = 0.15f;

	auto ballSprite = (Sprite *)_csbRoot->getChildByName("densityball1");
	bodyDef.userData = ballSprite; // body �P Sprite �s��

	Point ballloc = ballSprite->getPosition();
	Size ballsize = ballSprite->getContentSize();
	float ballScale = ballSprite->getScale();
	// �ھ� Sprite �Ҧb��m�]�w��Φb b2World ������m
	bodyDef.position.Set(ballloc.x/ PTM_RATIO, ballloc.y / PTM_RATIO);
	b2Body* ball1 = _b2World->CreateBody(&bodyDef);
	// �]�w��Ϊ��b�|
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
	// �إ߲��ͯx�εe�ت����s
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
	b2FixtureDef fixtureDef; // ���� Fixture
	b2PolygonShape rectShape;
	fixtureDef.shape = &rectShape;

	char tmp[20] = "";
	// �]�w�T�Ӥ��P�C��N��T�ӸI�����ժ��s��
	for (int i = 1; i <= 3; i++)
	{
		// ���o�T�ӳ]�w�I���L�o�����R�A����ϥ�
		sprintf(tmp, "filter1_%02d", i);
		auto rectSprite = (Sprite *)_csbRoot->getChildByName(tmp);
		bodyDef.userData = rectSprite;
		rectSprite->setColor(filterColor[(i-1)]);	// �ϥ� filterColor �w�g�إߪ��C��
		Size ts = rectSprite->getContentSize();
		Point loc = rectSprite->getPosition();
		float scaleX = rectSprite->getScaleX();	// Ū���x�εe�ئ��� X �b�Y��
		float scaleY = rectSprite->getScaleY();	// Ū���x�εe�ئ��� Y �b�Y��

		bodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO); // �]�w�O�l�Ҧb����m
		rectbody = _b2World->CreateBody(&bodyDef); // �b b2World ���إ߹���
		float bw = (ts.width - 4)* scaleX;
		float bh = (ts.height - 4)* scaleY;
		// �]�w���骺�d��O�@�� BOX �]�i�H�Y�񦨯x�Ρ^
		rectShape.SetAsBox(bw*0.5f / PTM_RATIO, bh*0.5f / PTM_RATIO);
		fixtureDef.filter.categoryBits = 1 << i;
		rectbody->CreateFixture(&fixtureDef);
	}
}

void FixtureCollision::setupSensorAndCollision()
{
	// �]�m�����i�H����y���O���ϥ�
	_light1 = CLight::create();
	_light1->setLightInfo("orange05.png","orange02.png",Point(822,682));
	_light1->setScale(0.75f);
	_light1->setLightStatus(true);
	this->addChild(_light1,5);
	//�N��i�H����y�A�u�n���U���Ͳy�����s�A�o�Ӫ��A�N�]�w�� false
	_bReleasingBall = true; 

	// �]�m�i�H���Ͳy�����s
	auto btnSprite = _csbRoot->getChildByName("releasingBall_Btn");
	_ballBtn = CButton::create();
	_ballBtn->setButtonInfo("dnarrow.png", "dnarrowon.png", btnSprite->getPosition());
	_ballBtn->setScale(btnSprite->getScale());
	this->addChild(_ballBtn, 5);
	btnSprite->setVisible(false);

	// �H�������ҩ�m����i�ϥܡA�W�٤��O�O sensor1 �P sensor2
	// �b��� sensor ����m�إ��R�A����A���i��I���A���ϥθI���ƥ�ӷP���y�O�_�g�L
	// ��y���U�g�L sensor1 �ɡA�֭p�[�t������
	// ����y���}�P���ϮɡA���@�ӤϦV�O�A���䩹�W�u�X
	// �]�w�@���ܼơA���y�O�ϦV�u�X�ɡA�����g�L  sensor1 ���I���ƥ�
	// ��y�u�n�g�L sensor2�A�N�� _bReleasingBall ��true �N��i�H����U�@�Ӳy

	// �]�w����R�A������P����
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
		SensorFixtureDef.isSensor = true;	// �]�w�� Sensor
		SensorFixtureDef.density = 9999+i; // �G�N�]�w���o�ӭȡA��K�IĲ�ɭԪ��P�_
		SensorBody->CreateFixture(&SensorFixtureDef);
	}

	// �إ� _collisionSprite �H�i�ܸI���o�ͮɪ��B��
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
	bodyDef.userData = _collisionSprite; // body �P Sprite �s��

	Point ballloc = _collisionSprite->getPosition();
	Size ballsize = _collisionSprite->getContentSize();
	float ballScale = _collisionSprite->getScale();
	// �ھ� Sprite �Ҧb��m�]�w��Φb b2World ������m
	bodyDef.position.Set(ballloc.x / PTM_RATIO, ballloc.y / PTM_RATIO);
	b2Body* ball1 = _b2World->CreateBody(&bodyDef);
	// �]�w��Ϊ��b�|
	circleShape.m_radius = ballsize.width *ballScale * 0.5f / PTM_RATIO;
	ball1->CreateFixture(&fixtureDef);
	
	_contactListener.setCollisionTarget(*_collisionSprite);
	_tdelayTime = 0; // �Ĥ@���@�w�i�H�Q�o
	_bSparking = true; // �i�H�Q�o
}


void FixtureCollision::doStep(float dt)
{
	int velocityIterations = 8;	// �t�׭��N����
	int positionIterations = 1; // ��m���N���� ���N���Ƥ@��]�w��8~10 �V���V�u����Ĳv�V�t
	// Instruct the world to perform a single step of simulation.
	// It is generally best to keep the time step and iterations fixed.
	_b2World->Step(dt, velocityIterations, positionIterations);

	// ���o _b2World ���Ҧ��� body �i��B�z
	// �̥D�n�O�ھڥثe�B�⪺���G�A��s���ݦb body �� sprite ����m
	for (b2Body* body = _b2World->GetBodyList(); body; )
	{
		// �H�U�O�H Body ���]�t Sprite ��ܬ���
		if (body->GetUserData() != NULL)
		{
			Sprite *spriteData = (Sprite*)body->GetUserData();
			spriteData->setPosition(body->GetPosition().x*PTM_RATIO, body->GetPosition().y*PTM_RATIO);
			spriteData->setRotation(-1 * CC_RADIANS_TO_DEGREES(body->GetAngle()));
		}
		else {
			if (!_bReleasingBall) {  // �ثe�y������s����ϥ�
				if (body->GetFixtureList()->GetDensity() == 10002.0f) { // �N�� sensor2 ���y�g�L�A���s�}��y���s�\��
					body->GetFixtureList()->SetDensity(10001.0f);
					_bReleasingBall = true;
					_light1->setLightStatus(_bReleasingBall);
				}
			}			
		}
		// �]�X�ù��~���N������q b2World ������
		if ( body->GetType() == b2BodyType::b2_dynamicBody ) {
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
				g_totRemoved++;
				CCLOG("Removing %4d Particles", g_totRemoved);
#endif
			}
			else body = body->GetNext(); //�_�h�N�~���s�U�@��Body
		}
		else body = body->GetNext(); //�_�h�N�~���s�U�@��Body
	}

	// ���ͤ���
	if (_contactListener._bCreateSpark ) {
		_contactListener._bCreateSpark = false;	//���ͧ�����
		// �P�_���𪺮ɶ��O�_����
		if ( _bSparking ) { //�i�H�Q�o�A��{�o�����Q�o
			_tdelayTime = 0; // �ɶ����s�]�w�A
			_bSparking = false; // �}�l�p��
			for (int i = 0; i < _contactListener._NumOfSparks; i++) {
				// �إ� Spark Sprite �ûP�ثe�����鵲�X
				auto sparkSprite = Sprite::createWithSpriteFrameName("spark.png");
				sparkSprite->setColor(Color3B(rand() % 256, rand() % 256, rand() % 156));
				sparkSprite->setBlendFunc(BlendFunc::ADDITIVE);
				this->addChild(sparkSprite, 5);
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
				RectFixtureDef.density = 1.0f;
				RectFixtureDef.isSensor = true;
				b2Fixture*RectFixture = RectBody->CreateFixture(&RectFixtureDef);

				//���O�q
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
			_tdelayTime = 0; // �k�s
			_bSparking = true; // �i�i��U�@�����Q�o
		}
	}
}

bool FixtureCollision::onTouchBegan(cocos2d::Touch *pTouch, cocos2d::Event *pEvent)//Ĳ�I�}�l�ƥ�
{
	Point touchLoc = pTouch->getLocation();
	_rectButton->touchesBegin(touchLoc);
	_ballBtn->touchesBegin(touchLoc);
	return true;
}

void  FixtureCollision::onTouchMoved(cocos2d::Touch *pTouch, cocos2d::Event *pEvent) //Ĳ�I���ʨƥ�
{
	Point touchLoc = pTouch->getLocation();
	_rectButton->touchesMoved(touchLoc);
	_ballBtn->touchesBegin(touchLoc);

}

void  FixtureCollision::onTouchEnded(cocos2d::Touch *pTouch, cocos2d::Event *pEvent) //Ĳ�I�����ƥ� 
{
	Point touchLoc = pTouch->getLocation();
	if (_rectButton->touchesEnded(touchLoc) && _iNumofRect < 9 ) { // ����@�Ӫ����
		// �b (518,680) ���ͤ@�Ӫ���ΡA�M�ᵹ�@���C��
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

		// �Ҧ� BOX2D ���� filter.categoryBits �w�]���O 1
		fixtureDef.filter.maskBits = 1 << (_iNumofRect % 3 + 1) | 1;
		body->CreateFixture(&fixtureDef);

		_iNumofRect++;
	}

	// ����s�T�w�Q���U�A�ӥB���\����y���ɭԡA�~���X�@���y
	if (_ballBtn->touchesEnded(touchLoc) && _bReleasingBall ) {
		_bReleasingBall = false; // ����y�g�L sensor2 ��~�|�A���}��
		_light1->setLightStatus(_bReleasingBall);

		// �H����ܤ@���y�A�q (1247,386) �B����ۥѸ��U �A�j�p�Y��50%
		auto ballSprite = Sprite::createWithSpriteFrameName(g_CircleObject[rand() % MAX_CIRCLE_OBJECTS]);
		ballSprite->setScale(0.5f);
		//	ballSprite->setPosition(touchLoc);
		this->addChild(ballSprite, 2);

		// �إߤ@��²�檺�ʺA�y��
		b2BodyDef bodyDef;	// ���H���c b2BodyDef �ŧi�@�� Body ���ܼ�
		bodyDef.type = b2_dynamicBody; // �]�w���ʺA����
		bodyDef.userData = ballSprite;	// �]�w Sprite ���ʺA���骺��ܹϥ�
		bodyDef.position.Set(1247.0f / PTM_RATIO, 386.0f / PTM_RATIO);
		// �H bodyDef �b b2World  ���إ߹���öǦ^�ӹ��骺����
		b2Body *ballBody = _b2World->CreateBody(&bodyDef);

		// �]�w�Ӫ��骺�~��
		b2CircleShape ballShape;	//  �ŧi���骺�~�������ܼơA���B�O��Ϊ���
		Size ballsize = ballSprite->getContentSize();	// �ھ� Sprite �ϧΪ��j�p�ӳ]�w��Ϊ��b�|
		ballShape.m_radius = 0.5f*(ballsize.width - 4) *0.5f / PTM_RATIO;
		// �H b2FixtureDef  ���c�ŧi���鵲�c�ܼơA�ó]�w���骺�������z�Y��
		b2FixtureDef fixtureDef;
		fixtureDef.shape = &ballShape;			// ���w���骺�~�������
		fixtureDef.restitution = 0.75f;			// �]�w�u�ʫY��
		fixtureDef.density = 1.0f;				// �]�w�K��
		fixtureDef.friction = 0.15f;			// �]�w�����Y��
		ballBody->CreateFixture(&fixtureDef);	// �b Body �W���ͳo�ӭ��骺�]�w
	}
}

void FixtureCollision::setStaticWalls()
{
	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody; // �]�w�o�� Body �� �R�A��
	bodyDef.userData = NULL;
	// �b b2World �����͸� Body, �öǦ^���ͪ� b2Body ���󪺫���
	// ���ͤ@���A�N�i�H���᭱�Ҧ��� Shape �ϥ�
	b2Body *staticbody = _b2World->CreateBody(&bodyDef);

	// �����R�A��ɩһݭn�� EdgeShape�A�W�ٳ��O wall ���}�Y
	b2EdgeShape edgeShape;
	b2FixtureDef fixtureDef; // ���� Fixture
	fixtureDef.shape = &edgeShape;
	fixtureDef.friction = 0.1f;
	char tmp[20] = "";
	Point pntLoc = _csbRoot->getPosition();

	for (size_t i = 1; i <= 50; i++)
	{
		// ���ͩһݭn�� Sprite file name int plist 
		// ���B���o�����O�۹�� csbRoot �Ҧb��m���۹�y��
		// �b�p�� edgeShape ���۹����y�ЮɡA�����i���ഫ
		sprintf(tmp, "wall1_%02d", i);
		auto edgeSprite = (Sprite *)_csbRoot->getChildByName(tmp);
		Size ts = edgeSprite->getContentSize();
		Point loc = edgeSprite->getPosition();
		float angle = edgeSprite->getRotation();
		float scale = edgeSprite->getScaleX();	// �������u�q�ϥܰ��]���u���� X �b��j

		Point lep1, lep2, wep1, wep2; // EdgeShape ����Ӻ��I
		lep1.y = 0; lep1.x = -(ts.width - 4) / 2.0f;
		lep2.y = 0; lep2.x = (ts.width - 4) / 2.0f;

		// �Ҧ����u�q�ϥܳ��O�O�����������I�� (0,0)�A
		// �ھ��Y��B���ಣ�ͩһݭn���x�}
		// �ھڼe�׭p��X��Ӻ��I���y�СA�M��e�W�}�x�}
		// �M��i�����A
		// Step1: ��CHECK ���L����A������h�i����I���p��
		cocos2d::Mat4 modelMatrix, rotMatrix;
		modelMatrix.m[0] = scale;  // ���]�w X �b���Y��
		cocos2d::Mat4::createRotationZ(angle*M_PI / 180.0f, &rotMatrix);
		modelMatrix.multiply(rotMatrix);
		modelMatrix.m[3] = pntLoc.x + loc.x; //�]�w Translation�A�ۤv���[�W���˪�
		modelMatrix.m[7] = pntLoc.y + loc.y; //�]�w Translation�A�ۤv���[�W���˪�

											 // ���ͨ�Ӻ��I
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
	// ������ Body, �]�w�������Ѽ�

	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody; // �]�w�o�� Body �� �R�A��
	bodyDef.userData = NULL;
	// �b b2World �����͸� Body, �öǦ^���ͪ� b2Body ���󪺫���
	// ���ͤ@���A�N�i�H���᭱�Ҧ��� Shape �ϥ�
	b2Body *body = _b2World->CreateBody(&bodyDef);

	// �����R�A��ɩһݭn�� EdgeShape
	b2EdgeShape edgeShape;
	b2FixtureDef edgeFixtureDef; // ���� Fixture
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
//��gø�s��k
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
// �u�n�O��� body �� fixtures �I���A�N�|�I�s�o�Ө禡
//
void CContactListener::BeginContact(b2Contact* contact)
{
	b2Body* BodyA = contact->GetFixtureA()->GetBody();
	b2Body* BodyB = contact->GetFixtureB()->GetBody();

	// check �O�_�����U���y�g�L sensor1 �A�u�n�g�L�N�ߨ����L�u�X�h
	if (BodyA->GetFixtureList()->GetDensity() == 10000.0f ) { // �N�� sensor1
		BodyB->ApplyLinearImpulse(b2Vec2(0, 50 + rand() % 101), BodyB->GetWorldCenter(), true);
		_bApplyImpulse = true;
	}
	else if (BodyB->GetFixtureList()->GetDensity() == 10000.0f) {// �N�� sensor1
		BodyA->ApplyLinearImpulse(b2Vec2(0, 50+rand()%101), BodyB->GetWorldCenter(), true);
		_bApplyImpulse = true;
	}

	if ( BodyA->GetUserData() == _targetSprite ) {
		float lengthV = BodyB->GetLinearVelocity().Length();
		if (lengthV >= 4.25f) { // ��Ĳ�ɪ��t�׶W�L�@�w���Ȥ~�Q�X����
			_bCreateSpark = true;
			_createLoc = BodyA->GetWorldCenter() + b2Vec2(0, -30 / PTM_RATIO);
		}
	}
	else if ( BodyB->GetUserData() == _targetSprite) {
		float lengthV = BodyB->GetLinearVelocity().Length();
		if (lengthV >= 4.25f) { // ��Ĳ�ɪ��t�׶W�L�@�w���Ȥ~�Q�X����
			_bCreateSpark = true;
			_createLoc = BodyB->GetWorldCenter() + b2Vec2(0, -30 / PTM_RATIO);
		}
	}
}

//�I������
void CContactListener::EndContact(b2Contact* contact)
{
	b2Body* BodyA = contact->GetFixtureA()->GetBody();
	b2Body* BodyB = contact->GetFixtureB()->GetBody();

	if (BodyA->GetFixtureList()->GetDensity() == 10001.0f && _bApplyImpulse ) { // �N�� sensor2
		BodyA->GetFixtureList()->SetDensity(10002);
		_bApplyImpulse = false;
	}
	else if (BodyB->GetFixtureList()->GetDensity() == 10001.0f && _bApplyImpulse ) {	// �N�� sensor2
		BodyB->GetFixtureList()->SetDensity(10002);
		_bApplyImpulse = false;
	}
}