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
	//�ŤM
	_bCutting = false;
	_icutNum = 0;
	_iCutLineNum = 0;
	//----------------------------------------------------------
	// �إ� Box2D world
	_b2World = nullptr;
	b2Vec2 Gravity = b2Vec2(0.0f, -9.8f);		//���O��V
	//b2Vec2 Gravity = b2Vec2(0.0f, 9.8f);		//���O��V
	bool AllowSleep = true;			//���\�ε�
	_b2World = new b2World(Gravity);	//�Ыإ@��
	_b2World->SetAllowSleeping(AllowSleep);	//�]�w���󤹳\�ε�

											// Create Scene with csb file
	_csbRoot = CSLoader::createNode("GearLevel.csb");
#ifndef BOX2D_DEBUG
	// �]�w��ܭI���ϥ�
	auto bgSprite = _csbRoot->getChildByName("bg");
	bgSprite->setVisible(true);

#endif
	addChild(_csbRoot, 1);

	//Ū������
	_startWater = dynamic_cast<Sprite*>(_csbRoot->getChildByName("water"));
	_Bush_dry = dynamic_cast<Sprite*>(_csbRoot->getChildByName("Bush_dry"));
	_Bush_live = dynamic_cast<Sprite*>(_csbRoot->getChildByName("Bush_live"));

	_DoorIn = dynamic_cast<Sprite*>(_csbRoot->getChildByName("trainsDoor_in"));
	_contactListener._DoorIn = _DoorIn; //�ǵ��I������
	_DoorOut = dynamic_cast<Sprite*>(_csbRoot->getChildByName("trainsDoor_out"));

	//�ƽ��u��
	_cactus1 = dynamic_cast<Sprite*>(_csbRoot->getChildByName("cactus"));
	_cactus2 = dynamic_cast<Sprite*>(_csbRoot->getChildByName("cactus_2"));
	_gear1 = dynamic_cast<Sprite*>(_csbRoot->getChildByName("gear_1")); this->addChild(_gear1, 3);
	_gear2 = dynamic_cast<Sprite*>(_csbRoot->getChildByName("gear_2")); this->addChild(_gear2, 3);
	_board = dynamic_cast<Sprite*>(_csbRoot->getChildByName("seesaw01_1")); this->addChild(_board, 2);
	_board_left = dynamic_cast<Sprite*>(_csbRoot->getChildByName("seesaw01_2")); this->addChild(_board_left, 3);
	_board_right = dynamic_cast<Sprite*>(_csbRoot->getChildByName("seesaw01_3")); this->addChild(_board_right, 3);
	_iLineNum = 0;

	//��ʽ��l��
	_fakeTurn1 = dynamic_cast<Sprite*>(_csbRoot->getChildByName("bubble_1"));
	_fakeTurn2 = dynamic_cast<Sprite*>(_csbRoot->getChildByName("bubble_2"));

	//----------------------------------------------------------
	//Start & End
	_fStartCount = 0;
	_contactListener._StartSprite = _startWater;
	_contactListener._EndSprite = _Bush_dry;
	_startPt = _startWater->getPosition();

	for (int i = 0; i < SPARKS_NUMBER; i++) {		//�x�s�ɤl�Ϥ�
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
	auto BtnPos = dynamic_cast<Button*>(_csbRoot->getChildByName("btn_retry")); //���s�C���s
	Point loc = BtnPos->getPosition();
	BtnPos->setVisible(false);
	_retrybtn = C3SButton::create();
	_retrybtn->setButtonInfo(NORMAL_BTN, "btn_retry_0.png", "btn_retry_2.png", "btn_retry_1.png", loc);
	_retrybtn->setScale(0.4f);
	_retrybtn->setVisible(true);
	this->addChild(_retrybtn, 3);

	BtnPos = dynamic_cast<Button*>(_csbRoot->getChildByName("btn_home")); //�^�����s
	loc = BtnPos->getPosition();
	BtnPos->setVisible(false);
	_homebtn = C3SButton::create();
	_homebtn->setButtonInfo(NORMAL_BTN, "btn_home_0.png", "btn_home_2.png", "btn_home_1.png", loc);
	_homebtn->setScale(0.4f);
	_homebtn->setVisible(true);
	this->addChild(_homebtn, 3);

	//Type 2
	BtnPos = dynamic_cast<Button*>(_csbRoot->getChildByName("cut")->getChildByName("cut_icon")); //�ŤM�s
	loc = _csbRoot->getChildByName("cut")->convertToWorldSpace(BtnPos->getPosition());
	BtnPos->setVisible(false);
	_cutbtn = C3SButton::create();
	_cutbtn->setButtonInfo(SELECT_BTN, "cut_icon.png", "cut_icon_on.png", "cut_icon_on.png", loc);
	_cutbtn->setScale(0.5f);
	_cutbtn->setVisible(true);
	this->addChild(_cutbtn, 3);

	//----------------------------------------------------------
	setStartEndpoint();			//�_���I
	//createStaticBoundary();	//���
	setupStatic();				//�R�A����
	setupRopeJoint();			//÷�l1
	setupRope2Joint();			//÷�l2
	setupRope3Joint();			//÷�l3
	setupRope4Joint();			//÷�l4
	setupWoodBoard();			//��O�]�w
	setupPulleyJoint();			//�ƽ�
	setupMoveDoor();			//�ǰe��
	setupGearStatic();			//�ƽ����� �R�A���~�]�w
	setupGear();				//����


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

	_b2World->SetContactListener(&_contactListener); //�I����ť��

	_listener1 = EventListenerTouchOneByOne::create();	//�Ыؤ@�Ӥ@��@���ƥ��ť��
	_listener1->onTouchBegan = CC_CALLBACK_2(GearLevel::onTouchBegan, this);		//�[�JĲ�I�}�l�ƥ�
	_listener1->onTouchMoved = CC_CALLBACK_2(GearLevel::onTouchMoved, this);		//�[�JĲ�I���ʨƥ�
	_listener1->onTouchEnded = CC_CALLBACK_2(GearLevel::onTouchEnded, this);		//�[�JĲ�I���}�ƥ�

	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(_listener1, this);	//�[�J��Ыت��ƥ��ť��
	this->schedule(CC_SCHEDULE_SELECTOR(GearLevel::doStep));

	return true;
}

void GearLevel::setupStatic()
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
		auto frameSprite = (Sprite *)_csbRoot->getChildByName(tmp);

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

void GearLevel::setStartEndpoint()
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

//��O
void GearLevel::setupWoodBoard()
{
	// ���o�ó]�w seesaw01 ���i�ʺA����j
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
	boxShape2.SetAsBox((size.width*0.5f*scale - 2.f) / PTM_RATIO, (size.height*0.5f*scale - 10.f) / PTM_RATIO); //���׭ץ�

	b2FixtureDef fixtureDef3;
	fixtureDef3.shape = &boxShape2;
	fixtureDef3.density = 2.0f;
	seesawBody2->CreateFixture(&fixtureDef3);
	_leftBoard = seesawBody2; //���������O
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
	boxShape3.SetAsBox((size.width*0.5f*scale - 2.f) / PTM_RATIO, (size.height*0.5f*scale - 10.f) / PTM_RATIO); //���׭ץ�

	b2FixtureDef fixtureDef4;
	fixtureDef4.shape = &boxShape3;
	fixtureDef4.density = 2.0f;
	seesawBody3->CreateFixture(&fixtureDef4);
	_rightBoard = seesawBody3;

	//----------------------------------------------------
	// �إ� �k��Joint �s��
	b2WeldJointDef WeldJointDef;
	WeldJointDef.Initialize(seesawBody, seesawBody2, seesawBody->GetPosition() + b2Vec2(-100 / PTM_RATIO, 0));
	_b2World->CreateJoint(&WeldJointDef); // �ϥιw�]�Ȳk��

	// �إ� �k��Joint �s��
	b2WeldJointDef JointDef;
	JointDef.Initialize(seesawBody, seesawBody3, seesawBody->GetPosition() + b2Vec2(100 / PTM_RATIO, 0));
	_b2World->CreateJoint(&JointDef); // �ϥιw�]�Ȳk��
}
//��1÷�l
void GearLevel::setupRopeJoint()
{
	// ���o�ó]�w stickRope �ϥܬ��i�R�A����j
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

	//���o�ó]�w cactus ���i�ʺA����j
	auto circleSprite = _csbRoot->getChildByName("cactus");
	Point locTail = circleSprite->getPosition();
	Size sizeTail = circleSprite->getContentSize();
	auto scaleTail = circleSprite->getScale();

	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(locTail.x / PTM_RATIO, locTail.y / PTM_RATIO);
	bodyDef.userData = circleSprite;
	_cactusBody = _b2World->CreateBody(&bodyDef);
	b2CircleShape circleShape;
	circleShape.m_radius = (sizeTail.width - 4)*0.5f*scaleTail*0.85f / PTM_RATIO; //0.85 : �V���ץ�(�Y�p)
	fixtureDef.shape = &circleShape;
	fixtureDef.density = 7.f;  fixtureDef.friction = 0.25f; fixtureDef.restitution = 0.25f;
	_cactusBody->CreateFixture(&fixtureDef);


	//����÷�l���`
	b2RopeJointDef JointDef;
	JointDef.bodyA = ropeHeadBody;
	JointDef.bodyB = _cactusBody;
	JointDef.localAnchorA = b2Vec2(0, 0);
	JointDef.localAnchorB = b2Vec2(0, 0);
	JointDef.maxLength = (locHead.y - locTail.y) / PTM_RATIO;
	JointDef.collideConnected = true;
	_MainRopeJoint = (b2RopeJoint*)_b2World->CreateJoint(&JointDef);

	// �����A�H�u�q�۳s�A
	char tmp[20] = "";
	Sprite *ropeSprite[10];
	Point loc[10];
	Size  size[11];
	b2Body* ropeBody[10];

	bodyDef.type = b2_dynamicBody;
	// �]���O÷�l�ҥH���q���n�ӭ�
	fixtureDef.density = 0.01f;  fixtureDef.friction = 1.0f; fixtureDef.restitution = 0.0f;
	fixtureDef.shape = &boxShape;
	// ���ͤ@�t�C��÷�l�q�� rope01_01 ~ rope01_15�A�P�ɱ��_��
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
	// �Q�� RevoluteJoint �N�u�q�����s���b�@�_
	// ���s�� ropeHeadBody �P  ropeBody[0]

	float locAnchor = 0.5f*(size[0].height - 10) / PTM_RATIO;
	b2RevoluteJointDef revJoint;
	revJoint.bodyA = ropeHeadBody;		//Head�P�u�q�s��
	revJoint.localAnchorA.Set(0, -0.5f);
	revJoint.bodyB = ropeBody[0];
	revJoint.localAnchorB.Set(0, locAnchor);
	_b2World->CreateJoint(&revJoint);
	for (int i = 0; i < 9; i++) {		//�����u�q�s��
		revJoint.bodyA = ropeBody[i];
		revJoint.localAnchorA.Set(0, -locAnchor);
		revJoint.bodyB = ropeBody[i + 1];
		revJoint.localAnchorB.Set(0, locAnchor);
		_b2World->CreateJoint(&revJoint);
	}
	revJoint.bodyA = ropeBody[9];		//Tail�P�u�q�s��
	revJoint.localAnchorA.Set(0, -locAnchor);
	revJoint.bodyB = _cactusBody;
	revJoint.localAnchorB.Set(-0.5f, 1.f);
	_b2World->CreateJoint(&revJoint);
}
//��2÷�l
void GearLevel::setupRope2Joint()
{
	// ���o�ó]�w stickRope �ϥܬ��i�R�A����j
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

	//���o�ó]�w cactus ���i�ʺA����j
	auto circleSprite = _csbRoot->getChildByName("cactus");
	Point locTail = circleSprite->getPosition();
	Size sizeTail = circleSprite->getContentSize();
	auto scaleTail = circleSprite->getScale();

	//����÷�l���`
	b2RopeJointDef JointDef;
	JointDef.bodyA = ropeHeadBody;
	JointDef.bodyB = _cactusBody;
	JointDef.localAnchorA = b2Vec2(0, 0);
	JointDef.localAnchorB = b2Vec2(0, 0);
	JointDef.maxLength = (locHead.y - locTail.y) / PTM_RATIO;
	JointDef.collideConnected = true;
	_MainRope2Joint = (b2RopeJoint*)_b2World->CreateJoint(&JointDef);

	// �����A�H�u�q�۳s�A
	char tmp[20] = "";
	Sprite *ropeSprite[10];
	Point loc[10];
	Size  size[11];
	b2Body* ropeBody[10];

	bodyDef.type = b2_dynamicBody;
	// �]���O÷�l�ҥH���q���n�ӭ�
	fixtureDef.density = 0.012f;  fixtureDef.friction = 1.0f; fixtureDef.restitution = 0.0f;
	fixtureDef.shape = &boxShape;
	// ���ͤ@�t�C��÷�l�q�� rope01_01 ~ rope01_15�A�P�ɱ��_��
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
	// �Q�� RevoluteJoint �N�u�q�����s���b�@�_
	// ���s�� ropeHeadBody �P  ropeBody[0]

	float locAnchor = 0.5f*(size[0].height - 10) / PTM_RATIO;
	b2RevoluteJointDef revJoint;
	revJoint.bodyA = ropeHeadBody;		//Head�P�u�q�s��
	revJoint.localAnchorA.Set(0, -0.5f);
	revJoint.bodyB = ropeBody[0];
	revJoint.localAnchorB.Set(0, locAnchor);
	_b2World->CreateJoint(&revJoint);
	for (int i = 0; i < 9; i++) {		//�����u�q�s��
		revJoint.bodyA = ropeBody[i];
		revJoint.localAnchorA.Set(0, -locAnchor);
		revJoint.bodyB = ropeBody[i + 1];
		revJoint.localAnchorB.Set(0, locAnchor);
		_b2World->CreateJoint(&revJoint);
	}
	revJoint.bodyA = ropeBody[9];		//Tail�P�u�q�s��
	revJoint.localAnchorA.Set(0, -locAnchor);
	revJoint.bodyB = _cactusBody;
	revJoint.localAnchorB.Set(0.5f, 1.f);
	_b2World->CreateJoint(&revJoint);
}
//�k2÷�l
void GearLevel::setupRope3Joint()
{
	// ���o�ó]�w stickRope �ϥܬ��i�R�A����j
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

	//���o�ó]�w cactus ���i�ʺA����j
	auto circleSprite = _csbRoot->getChildByName("cactus_2");
	Point locTail = circleSprite->getPosition();
	Size sizeTail = circleSprite->getContentSize();
	auto scaleTail = circleSprite->getScale();

	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(locTail.x / PTM_RATIO, locTail.y / PTM_RATIO);
	bodyDef.userData = circleSprite;
	_cactus2Body = _b2World->CreateBody(&bodyDef);
	b2CircleShape circleShape;
	circleShape.m_radius = (sizeTail.width - 4)*0.5f*scaleTail*0.85f / PTM_RATIO; //0.85 : �V���ץ�(�Y�p)
	fixtureDef.shape = &circleShape;
	fixtureDef.density = 7.f;  fixtureDef.friction = 0.25f; fixtureDef.restitution = 0.25f;
	_cactus2Body->CreateFixture(&fixtureDef);


	//����÷�l���`
	b2RopeJointDef JointDef;
	JointDef.bodyA = ropeHeadBody;
	JointDef.bodyB = _cactus2Body;
	JointDef.localAnchorA = b2Vec2(0, 0);
	JointDef.localAnchorB = b2Vec2(0, 0);
	JointDef.maxLength = (locHead.y - locTail.y) / PTM_RATIO;
	JointDef.collideConnected = true;
	_MainRope3Joint = (b2RopeJoint*)_b2World->CreateJoint(&JointDef);

	// �����A�H�u�q�۳s�A
	char tmp[20] = "";
	Sprite *ropeSprite[10];
	Point loc[10];
	Size  size[11];
	b2Body* ropeBody[10];

	bodyDef.type = b2_dynamicBody;
	// �]���O÷�l�ҥH���q���n�ӭ�
	fixtureDef.density = 0.013f;  fixtureDef.friction = 1.0f; fixtureDef.restitution = 0.0f;
	fixtureDef.shape = &boxShape;
	// ���ͤ@�t�C��÷�l�q�� rope01_01 ~ rope01_15�A�P�ɱ��_��
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
	// �Q�� RevoluteJoint �N�u�q�����s���b�@�_
	// ���s�� ropeHeadBody �P  ropeBody[0]

	float locAnchor = 0.5f*(size[0].height - 10) / PTM_RATIO;
	b2RevoluteJointDef revJoint;
	revJoint.bodyA = ropeHeadBody;		//Head�P�u�q�s��
	revJoint.localAnchorA.Set(0, -0.5f);
	revJoint.bodyB = ropeBody[0];
	revJoint.localAnchorB.Set(0, locAnchor);
	_b2World->CreateJoint(&revJoint);
	for (int i = 0; i < 9; i++) {		//�����u�q�s��
		revJoint.bodyA = ropeBody[i];
		revJoint.localAnchorA.Set(0, -locAnchor);
		revJoint.bodyB = ropeBody[i + 1];
		revJoint.localAnchorB.Set(0, locAnchor);
		_b2World->CreateJoint(&revJoint);
	}
	revJoint.bodyA = ropeBody[9];		//Tail�P�u�q�s��
	revJoint.localAnchorA.Set(0, -locAnchor);
	revJoint.bodyB = _cactus2Body;
	revJoint.localAnchorB.Set(-0.5f, 1.f);
	_b2World->CreateJoint(&revJoint);
}
//�k1÷�l
void GearLevel::setupRope4Joint()
{
	// ���o�ó]�w stickRope �ϥܬ��i�R�A����j
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

	//���o�ó]�w cactus ���i�ʺA����j
	auto circleSprite = _csbRoot->getChildByName("cactus_2");
	Point locTail = circleSprite->getPosition();
	Size sizeTail = circleSprite->getContentSize();
	auto scaleTail = circleSprite->getScale();

	//����÷�l���`
	b2RopeJointDef JointDef;
	JointDef.bodyA = ropeHeadBody;
	JointDef.bodyB = _cactus2Body;
	JointDef.localAnchorA = b2Vec2(0, 0);
	JointDef.localAnchorB = b2Vec2(0, 0);
	JointDef.maxLength = (locHead.y - locTail.y) / PTM_RATIO;
	JointDef.collideConnected = true;
	_MainRope4Joint = (b2RopeJoint*)_b2World->CreateJoint(&JointDef);

	// �����A�H�u�q�۳s�A
	char tmp[20] = "";
	Sprite *ropeSprite[10];
	Point loc[10];
	Size  size[11];
	b2Body* ropeBody[10];

	bodyDef.type = b2_dynamicBody;
	// �]���O÷�l�ҥH���q���n�ӭ�
	fixtureDef.density = 0.014f;  fixtureDef.friction = 1.0f; fixtureDef.restitution = 0.0f;
	fixtureDef.shape = &boxShape;
	// ���ͤ@�t�C��÷�l�q�� rope01_01 ~ rope01_15�A�P�ɱ��_��
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
	// �Q�� RevoluteJoint �N�u�q�����s���b�@�_
	// ���s�� ropeHeadBody �P  ropeBody[0]

	float locAnchor = 0.5f*(size[0].height - 10) / PTM_RATIO;
	b2RevoluteJointDef revJoint;
	revJoint.bodyA = ropeHeadBody;		//Head�P�u�q�s��
	revJoint.localAnchorA.Set(0, -0.5f);
	revJoint.bodyB = ropeBody[0];
	revJoint.localAnchorB.Set(0, locAnchor);
	_b2World->CreateJoint(&revJoint);
	for (int i = 0; i < 9; i++) {		//�����u�q�s��
		revJoint.bodyA = ropeBody[i];
		revJoint.localAnchorA.Set(0, -locAnchor);
		revJoint.bodyB = ropeBody[i + 1];
		revJoint.localAnchorB.Set(0, locAnchor);
		_b2World->CreateJoint(&revJoint);
	}
	revJoint.bodyA = ropeBody[9];		//Tail�P�u�q�s��
	revJoint.localAnchorA.Set(0, -locAnchor);
	revJoint.bodyB = _cactus2Body;
	revJoint.localAnchorB.Set(0.5f, 1.f);
	_b2World->CreateJoint(&revJoint);
}
//�ƽ�
void GearLevel::setupPulleyJoint()
{
	// �����ƽ���m
	auto Sprite = _csbRoot->getChildByName("gear_1");
	Point loc = Sprite->getPosition();

	//���ͷƽ����`
	b2PulleyJointDef JointDef;
	JointDef.Initialize(_cactusBody, _Board,
		b2Vec2((loc.x - 48.f) / PTM_RATIO, loc.y / PTM_RATIO), //������m
		b2Vec2((loc.x + 48.f) / PTM_RATIO, loc.y / PTM_RATIO),
		_cactusBody->GetWorldCenter(),
		b2Vec2(_Board->GetWorldCenter().x - (100.f / PTM_RATIO), _Board->GetWorldCenter().y),
		1);
	_b2World->CreateJoint(&JointDef);

	//---------------------------------------------
	// �k���ƽ���m
	Sprite = _csbRoot->getChildByName("gear_2");
	loc = Sprite->getPosition();

	//���ͷƽ����`
	JointDef.Initialize(_Board, _cactus2Body,
		b2Vec2((loc.x - 48.f) / PTM_RATIO, loc.y / PTM_RATIO), //������m
		b2Vec2((loc.x + 48.f) / PTM_RATIO, loc.y / PTM_RATIO),
		b2Vec2(_Board->GetWorldCenter().x + (100.f / PTM_RATIO), _Board->GetWorldCenter().y),
		_cactus2Body->GetWorldCenter(),
		1);
	_b2World->CreateJoint(&JointDef);
}
//�ǰe��
void GearLevel::setupMoveDoor()
{
	// ���o�ó]�w�i�J�I trainsDoor_in �ϥܬ��i�R�A����j�P����
	Point loc = _DoorIn->getPosition();
	Size size = _DoorIn->getContentSize();
	auto scale = _DoorIn->getScale();
	b2BodyDef doorInBodyDef;
	doorInBodyDef.type = b2_staticBody;
	doorInBodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	doorInBodyDef.userData = _DoorIn;
	b2Body* doorInBody = _b2World->CreateBody(&doorInBodyDef);

	b2CircleShape circleShape;
	circleShape.m_radius = size.width*0.5f*scale*0.85f / PTM_RATIO; //0.85f : �ץ�

	b2FixtureDef doorInfixtureDef;
	doorInfixtureDef.shape = &circleShape;
	doorInfixtureDef.isSensor = true; // �]�w�� Sensor
	doorInBody->CreateFixture(&doorInfixtureDef);
}
//�ƽ�����
void GearLevel::setupGearStatic()
{
	// ���o�ó]�w bubble ���i�R�A����j
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
	// ���o�ó]�w bubble ���i�R�A����j
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
	// ���o�ó]�w gear �I���R�A����
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
	// ���o�ó]�w gear �I���R�A����
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
	// ���o�ó]�w�_�I gear1 ���i�ʺA����j
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
	// ���o�ó]�w�_�I gear2 ���i�ʺA����j
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
	//�إ� Joint
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
	//�e�X�Ũ��u�q
	_drawCut[num] = DrawNode::create();
	_drawCut[num]->drawLine(firstPt, Pt, color);
	//_drawCut[num]->clear();
	_cutNode->addChild(_drawCut[num], 5);
}

void GearLevel::doStep(float dt)
{
	int velocityIterations = 8;	// �t�׭��N����
	int positionIterations = 1; // ��m���N���� ���N���Ƥ@��]�w��8~10 �V���V�u����Ĳv�V�t
								// Instruct the world to perform a single step of simulation.
								// It is generally best to keep the time step and iterations fixed.
	_b2World->Step(dt, velocityIterations, positionIterations);

	// ���o _b2World ���Ҧ��� body �i��B�z
	// �̥D�n�O�ھڥثe�B�⪺���G�A��s���ݦb body �� sprite ����m
	for (b2Body* body = _b2World->GetBodyList(); body; body = body->GetNext())
	{
		// �H�U�O�H Body ���]�t Sprite ��ܬ���
		if (body->GetUserData() != NULL)
		{
			Sprite *spriteData = (Sprite*)body->GetUserData();
			if (spriteData != NULL) {
				spriteData->setPosition(body->GetPosition().x*PTM_RATIO, body->GetPosition().y*PTM_RATIO);
				spriteData->setRotation(-1 * CC_RADIANS_TO_DEGREES(body->GetAngle()));
			}
		}
		//// �]�X�ù��~���N������q b2World ������
		//if (body->GetType() == b2BodyType::b2_dynamicBody) {
		//	float x = body->GetPosition().x * PTM_RATIO;
		//	float y = body->GetPosition().y * PTM_RATIO;
		//	if (x > _visibleSize.width || x < 0 || y >  _visibleSize.height || y < 0) {
		//		if (body->GetUserData() != NULL) {
		//			Sprite* spriteData = (Sprite *)body->GetUserData();
		//			this->removeChild(spriteData);
		//		}
		//		b2Body* nextbody = body->GetNext(); // ���o�U�@�� body
		//		_b2World->DestroyBody(body); // ����ثe�� body
		//		body = nextbody;  // �� body ���V��~���o���U�@�� body
		//	}
		//	else body = body->GetNext(); //�_�h�N�~���s�U�@��Body
		//}
		//else body = body->GetNext(); //�_�h�N�~���s�U�@��Body
	}

	//----------------------------------------------------------------------------
	//for Start & End
	if(!_waterBody->IsAwake() && !_contactListener._bWin)_fStartCount += dt; //���y�R��B�C���|������ : �}�l�p��
	if (_fStartCount > 3.0f) {	//�R��W�L�T��A���m��m
		_fStartCount -= 3.0f;
		_waterBody->SetType(b2_staticBody);
		_waterBody->SetTransform(b2Vec2(_startPt.x / PTM_RATIO, _startPt.y / PTM_RATIO), _waterBody->GetAngle());
		_waterBody->SetType(b2_dynamicBody); //BUG�ץ� : �����]�w��m�|�d��A�ഫType��Body���򸨤U
	}
	if (_contactListener._bWin) {		//�L��
		_startWater->setVisible(false);
		_Bush_dry->setVisible(false);
		_Bush_live->setVisible(true);
		_fWinCount += dt;
	}
	if (_fWinCount > 4.0f) { //����U�@��
		_fWinCount = -100.f; //�קK���ư���
		auto Scene = GravityLevel::createScene();
		Director::getInstance()->replaceScene(TransitionFade::create(1, Scene, Color3B(255, 255, 255)));
	}
	// ���͹L������------------------------------------
	if (_contactListener._bCreateSpark) {
		_contactListener._bCreateSpark = false;	//���ͧ�����
		for (int i = 0; i < SPARKS_NUMBER; i++) {
			_sparks[i]->setVisible(true);
			//���ͤp������
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

			//���O�q
			RectBody->ApplyForce(b2Vec2(rand() % 51 - 25, 50 + rand() % 30), _contactListener._createLoc, true);
		}
	}

	//----------------------------------------------------------------------------
	// for cutting
	if (_contactListener._bDeleteJoint) { //delete joint
		_contactListener._bDeleteJoint = false;
		for (b2Joint* j = _b2World->GetJointList(); j; ) //�Yrope�I�� �����䨭�W��joint
		{
			b2Joint* jointToDestroy = j;
			j = j->GetNext();
			if (jointToDestroy->GetBodyA() == _contactListener._ropeBody) _b2World->DestroyJoint(jointToDestroy);	//���_÷�l
			if (jointToDestroy == _MainRopeJoint && _contactListener._bDeleteRope1) {
				_b2fakeTurn1->SetType(b2_dynamicBody);
				_b2World->DestroyJoint(_MainRopeJoint);	//���_�D÷
			}
			if (jointToDestroy == _MainRope2Joint && _contactListener._bDeleteRope2) {
				_b2fakeTurn1->SetType(b2_dynamicBody);
				_b2World->DestroyJoint(_MainRope2Joint);	//���_�D÷
			}
			if (jointToDestroy == _MainRope3Joint && _contactListener._bDeleteRope3) {
				_b2fakeTurn2->SetType(b2_dynamicBody);
				_b2World->DestroyJoint(_MainRope3Joint);	//���_�D÷
			}
			if (jointToDestroy == _MainRope4Joint && _contactListener._bDeleteRope4) {
				_b2fakeTurn2->SetType(b2_dynamicBody);
				_b2World->DestroyJoint(_MainRope4Joint);	//���_�D÷
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
		_waterBody->ApplyLinearImpulse(b2Vec2(0, 100 + rand() % 51), _waterBody->GetWorldCenter(), true); //�V�W���O
	}

	//----------------------------------------------------------------------------
	//ø�s�ƽ�÷
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
	//ø�s�Ũ��u
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

bool GearLevel::onTouchBegan(cocos2d::Touch *pTouch, cocos2d::Event *pEvent)//Ĳ�I�}�l�ƥ�
{
	Point touchLoc = pTouch->getLocation();
	_tp_start = touchLoc;
	_tp = touchLoc;
	//-------------------------------------------------------------------
	//For button
	if (_retrybtn->onTouchBegan(touchLoc) || _homebtn->onTouchBegan(touchLoc)) _bBtnPressed = true;

	//-------------------------------------------------------------------
	//for cutting
	if (!_bBtnPressed && _icutNum < CUTTING_NUM)_bCutting = true;	//���b���s�W �٦��Ѿl�e�Ϧ��Ƥ~�i�e��
	if (_bCutting) {
		_cutNode = Node::create(); //�s�W�����e����`�I
		this->addChild(_cutNode, 5);
		_contactListener._cutNode = _cutNode; //�ǤJBox2d world
		_cutPt[0] = _cutPt[1] = touchLoc;	//�Ĥ@�I
	}
	//-------------------------------------------------------------------

	return true;
}

void  GearLevel::onTouchMoved(cocos2d::Touch *pTouch, cocos2d::Event *pEvent) //Ĳ�I���ʨƥ�
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

void  GearLevel::onTouchEnded(cocos2d::Touch *pTouch, cocos2d::Event *pEvent) //Ĳ�I�����ƥ� 
{
	Point touchLoc = pTouch->getLocation();
	_tp = touchLoc;
	//-------------------------------------------------------------------
	//For cutting
	if (_bCutting && _icutNum < CUTTING_NUM) {
		DrawCuttingLine(_cutPt[0], _cutPt[1], Color4F::GREEN, _icutNum); //�e�u�禡

		//�إ� �i�R�A����j
		Point cutloc = _cutNode->getPosition();
		b2BodyDef cutbodyDef;				//body�u���ͤ@��
		cutbodyDef.type = b2_staticBody;
		cutbodyDef.position.Set(cutloc.x / PTM_RATIO, cutloc.y / PTM_RATIO);
		cutbodyDef.userData = _cutNode;
		b2Body *body = _b2World->CreateBody(&cutbodyDef);

		b2EdgeShape edgeShape;
		b2FixtureDef edgeFixtureDef; // ���� Fixture
		edgeFixtureDef.shape = &edgeShape;
		edgeFixtureDef.isSensor = true;
		edgeShape.Set(b2Vec2(_cutPt[0].x / PTM_RATIO, _cutPt[0].y / PTM_RATIO), b2Vec2(_cutPt[1].x / PTM_RATIO, _cutPt[1].y / PTM_RATIO)); //�Ũ��u�q
		body->CreateFixture(&edgeFixtureDef);

		_icutNum++;				//�w�Ũ�����+1
		char tmp[10] = "";
		sprintf(tmp, "cut_%d", CUTTING_NUM - _icutNum + 1); //���o�Ũ�UI
		auto cut_ui = (Sprite *)_csbRoot->getChildByName("cut")->getChildByName(tmp);
		cut_ui->setVisible(false); //����

		_bCutting = false;
	}
	//---------------------------------------------------------------
	//Button
	_bBtnPressed = false;
	if (_retrybtn->onTouchEnded(touchLoc)) {		//���s�C���s
		auto Scene = GearLevel::createScene();
		Director::getInstance()->replaceScene(TransitionFade::create(1, Scene, Color3B(255, 255, 255)));
	}
	if (_homebtn->onTouchEnded(touchLoc)) {			//�^�����s
		auto mainScene = MainMenu::createScene();
		Director::getInstance()->replaceScene(TransitionFade::create(1, mainScene, Color3B(255, 255, 255)));
	}

}

void GearLevel::createStaticBoundary()
{
	// ������ Body, �]�w�������Ѽ�

	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody; // �]�w�o�� Body �� �R�A��
	bodyDef.userData = NULL;
	// �b b2World �����͸� Body, �öǦ^���ͪ� b2Body ���󪺫���
	// ���ͤ@���A�N�i�H���᭱�Ҧ��� Shape �ϥ�
	b2Body *body = _b2World->CreateBody(&bodyDef);

	_bottomBody = body;
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
	//_NumOfSparks = 20;	//�Q�o���l��

	_bDeleteJoint = false;
	_bDeleteRope1 = false;
	_bDeleteRope2 = false;

	_bMove = false; //�ǰe��
}

// �u�n�O��� body �� fixtures �I���A�N�|�I�s�o�Ө禡
void  CContactListenerLevel4::BeginContact(b2Contact* contact)
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
	//----------------------------------------------------------------------
	//�ǰe������
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
	// rope cut �I����
	auto shapeA = BodyA->GetFixtureList()->GetBody()->GetUserData();
	auto shapeB = BodyB->GetFixtureList()->GetBody()->GetUserData();
	if (shapeA == _cutNode) { // A:cutline
		if (BodyB->GetFixtureList()->GetDensity() == 0.01f) { // B:rope1
			_bDeleteJoint = true;
			_bDeleteRope1 = true;
			_lineBody = BodyA;	//�Ǧ^rope
			_ropeBody = BodyB;	//�Ǧ^line
		}
		else if (BodyB->GetFixtureList()->GetDensity() == 0.012f) { // B:rope2
			_bDeleteJoint = true;
			_bDeleteRope2 = true;
			_lineBody = BodyA;	//�Ǧ^rope
			_ropeBody = BodyB;	//�Ǧ^line
		}
		else if (BodyB->GetFixtureList()->GetDensity() == 0.013f) { // B:rope3
			_bDeleteJoint = true;
			_bDeleteRope3 = true;
			_lineBody = BodyA;	//�Ǧ^rope
			_ropeBody = BodyB;	//�Ǧ^line
		}
		else if (BodyB->GetFixtureList()->GetDensity() == 0.014f) { // B:rope4
			_bDeleteJoint = true;
			_bDeleteRope4 = true;
			_lineBody = BodyA;	//�Ǧ^rope
			_ropeBody = BodyB;	//�Ǧ^line
		}
	}
	else if (shapeB == _cutNode) { // B:cutline
		if (BodyA->GetFixtureList()->GetDensity() == 0.01f) { // A:rope1
			_bDeleteJoint = true;
			_bDeleteRope1 = true;
			_ropeBody = BodyA;	//�Ǧ^rope
			_lineBody = BodyB;	//�Ǧ^line
		}
		else if (BodyA->GetFixtureList()->GetDensity() == 0.012f) { // A:rope2
			_bDeleteJoint = true;
			_bDeleteRope2 = true;
			_ropeBody = BodyA;	//�Ǧ^rope
			_lineBody = BodyB;	//�Ǧ^line
		}
		else if (BodyA->GetFixtureList()->GetDensity() == 0.013f) { // A:rope3
			_bDeleteJoint = true;
			_bDeleteRope3 = true;
			_ropeBody = BodyA;	//�Ǧ^rope
			_lineBody = BodyB;	//�Ǧ^line
		}
		else if (BodyA->GetFixtureList()->GetDensity() == 0.014f) { // A:rope4
			_bDeleteJoint = true;
			_bDeleteRope4 = true;
			_ropeBody = BodyA;	//�Ǧ^rope
			_lineBody = BodyB;	//�Ǧ^line
		}
	}

}

//�I������
void  CContactListenerLevel4::EndContact(b2Contact* contact)
{

}