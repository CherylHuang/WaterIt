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
	//�ŤM
	_bCutting = false;
	_icutNum = 0;
	_iCutLineNum = 0;
	//----------------------------------------------------------
	//�e��
	_iFree = _iFree_red = _iFree_blue = LINE_LENGTH;
	_iInUsed = _iInUsed_red = _iInUsed_blue = 0;
	_bDrawing = false;
	//----------------------------------------------------------
	// �إ� Box2D world
	_b2World = nullptr;
	b2Vec2 Gravity = b2Vec2(0.0f, -9.8f);		//���O��V
	bool AllowSleep = true;			//���\�ε�
	_b2World = new b2World(Gravity);	//�Ыإ@��
	_b2World->SetAllowSleeping(AllowSleep);	//�]�w���󤹳\�ε�

											// Create Scene with csb file
	_csbRoot = CSLoader::createNode("CarLevel.csb");
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
	_penBlackBar = dynamic_cast<LoadingBar*>(_csbRoot->getChildByName("Ink_black")->getChildByName("penBlack_bar")); //�����q��
	_penBlackBar->setPercent(100);
	_penRedBar = dynamic_cast<LoadingBar*>(_csbRoot->getChildByName("Ink_red")->getChildByName("penRed_bar")); //�����q��
	_penRedBar->setPercent(100);
	_penBlueBar = dynamic_cast<LoadingBar*>(_csbRoot->getChildByName("Ink_blue")->getChildByName("penBlue_bar")); //�����q��
	_penBlueBar->setPercent(100);
	_cut1 = dynamic_cast<Sprite*>(_csbRoot->getChildByName("cut")->getChildByName("cut_1"));
	//_cut2 = dynamic_cast<Sprite*>(_csbRoot->getChildByName("cut")->getChildByName("cut_2"));

	_triSensor = dynamic_cast<Sprite*>(_csbRoot->getChildByName("tri_line"));	//�P���I
	_rectSensor = dynamic_cast<Sprite*>(_csbRoot->getChildByName("rect_line"));
	_jeepSensor = dynamic_cast<Sprite*>(_csbRoot->getChildByName("jeep_line"));
	_blackWood0 = dynamic_cast<Sprite*>(_csbRoot->getChildByName("wall_10"));	//���z�׶¤��Y
	_blackWood1 = dynamic_cast<Sprite*>(_csbRoot->getChildByName("wall_10_0"));	//�׶¤��Y
	_blackWood2 = dynamic_cast<Sprite*>(_csbRoot->getChildByName("wall_11"));	//�׶¤��Y
	_blackWood1->setVisible(false); //����
	_blackWood2->setVisible(false);
	_redWood0 = dynamic_cast<Sprite*>(_csbRoot->getChildByName("wall_12"));		//���z�׬����Y
	_redWood1 = dynamic_cast<Sprite*>(_csbRoot->getChildByName("wall_12_0"));	//�׬����Y
	_redWood2 = dynamic_cast<Sprite*>(_csbRoot->getChildByName("wall_13"));		//�����Y
	_redWood1->setVisible(false); //����
	_redWood2->setVisible(false);
	_blueWood = dynamic_cast<Sprite*>(_csbRoot->getChildByName("wall_9"));	//�Ť��Y

	_jeepSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("jeep"));	//���l
	_contactListener._jeepSprite = _jeepSprite;	//�ǤJBox2d world

	//----------------------------------------------------------
	//Sensor
	_contactListener._triSensor = _triSensor;	//�ǤJBox2d�@��
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

	for (int i = 0; i < SPARKS_NUMBER; i++) {		//�x�s�ɤl�Ϥ�
		_sparks[i] = Sprite::createWithSpriteFrameName("pen.png");
		_sparks[i]->setColor(Color3B(0, 146 - rand() % 20, 200 - rand() % 20));
		_sparks[i]->setBlendFunc(BlendFunc::ADDITIVE);
		this->addChild(_sparks[i], 5);
		_sparks[i]->setVisible(false);
	}
	//----------------------------------------------------------
	//�e�ϮĪG
	for (int i = 0; i < SPARKS_NUMBER; i++) {		//�x�s�ɤl�Ϥ�
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
	auto BtnPos = dynamic_cast<Button*>(_csbRoot->getChildByName("btn_shape")); //�Ϊ��s
	Point loc = BtnPos->getPosition();
	BtnPos->setVisible(false);
	_shapebtn = CSwitchButton::create();
	_shapebtn->setButtonInfo("btn_shape_0.png", "btn_shape_1.png", "btn_shape_2.png", loc);
	_shapebtn->setScale(0.4f);
	this->addChild(_shapebtn, 2);

	//Type 1
	BtnPos = dynamic_cast<Button*>(_csbRoot->getChildByName("btn_retry")); //���s�C���s
	loc = BtnPos->getPosition();
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
	BtnPos = dynamic_cast<Button*>(_csbRoot->getChildByName("Ink_black")->getChildByName("penBlack_icon")); //�µ��s
	loc = _csbRoot->getChildByName("Ink_black")->convertToWorldSpace(BtnPos->getPosition());
	BtnPos->setVisible(false);
	_blackpenbtn = C3SButton::create();
	_blackpenbtn->setButtonInfo(SELECT_BTN, "penBlack_icon.png", "penBlack_icon_on.png", "penBlack_icon_on.png", loc);
	_blackpenbtn->setScale(0.5f);
	_blackpenbtn->setVisible(true);
	this->addChild(_blackpenbtn, 3);
	_blackpenbtn->setEnable(false);	//��l�Ψ� �]���ϥΤ�

	BtnPos = dynamic_cast<Button*>(_csbRoot->getChildByName("Ink_red")->getChildByName("penRed_icon")); //�����s
	loc = _csbRoot->getChildByName("Ink_red")->convertToWorldSpace(BtnPos->getPosition());
	BtnPos->setVisible(false);
	_redpenbtn = C3SButton::create();
	_redpenbtn->setButtonInfo(SELECT_BTN, "penRed_icon.png", "penRed_icon_on.png", "penRed_icon_on.png", loc);
	_redpenbtn->setScale(0.5f);
	_redpenbtn->setVisible(true);
	this->addChild(_redpenbtn, 3);

	BtnPos = dynamic_cast<Button*>(_csbRoot->getChildByName("Ink_blue")->getChildByName("penBlue_icon")); //�ŵ��s
	loc = _csbRoot->getChildByName("Ink_blue")->convertToWorldSpace(BtnPos->getPosition());
	BtnPos->setVisible(false);
	_bluepenbtn = C3SButton::create();
	_bluepenbtn->setButtonInfo(SELECT_BTN, "penBlue_icon.png", "penBlue_icon_on.png", "penBlue_icon_on.png", loc);
	_bluepenbtn->setScale(0.5f);
	_bluepenbtn->setVisible(true);
	this->addChild(_bluepenbtn, 3);

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
	createStaticBoundary();		//���
	setupStatic();				//�R�A����
	setupJeep();				//���l
	setupRopeJoint();			//÷�l
	setupSensorPt();			//�P���I

	//------------------------------------------------------------------------
	_iflareNum = 0;
	_fflareCount = 0;
	for (int i = 0; i < SPARKS_NUMBER; i++) {
		//_flare[i]->setVisible(true);
		//���ͤp������
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

		//���O�q
		_b2flare[i]->ApplyForce(b2Vec2(rand() % 26 - 13, 10 + rand() % 10), b2Vec2(_Bush_dry->getPositionX() / PTM_RATIO, _Bush_dry->getPositionY() / PTM_RATIO), true);
	}


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
	_listener1->onTouchBegan = CC_CALLBACK_2(CarLevel::onTouchBegan, this);		//�[�JĲ�I�}�l�ƥ�
	_listener1->onTouchMoved = CC_CALLBACK_2(CarLevel::onTouchMoved, this);		//�[�JĲ�I���ʨƥ�
	_listener1->onTouchEnded = CC_CALLBACK_2(CarLevel::onTouchEnded, this);		//�[�JĲ�I���}�ƥ�

	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(_listener1, this);	//�[�J��Ыت��ƥ��ť��
	this->schedule(CC_SCHEDULE_SELECTOR(CarLevel::doStep));

	return true;
}

void CarLevel::setupStatic()
{
	// Ū���Ҧ� wall_ �}�Y���ϥ� ���O�R�A����
	char tmp[20] = "";

	for (size_t i = 1; i <= 13; i++)
	{
		sprintf(tmp, "wall_%d", i);
		auto rectSprite = (Sprite *)_csbRoot->getChildByName(tmp);

		Size ts = rectSprite->getContentSize();
		Point loc = rectSprite->getPosition();
		float angle = rectSprite->getRotation();
		float scaleX = rectSprite->getScaleX();	// �������u�q�ϥܰ��]���u���� X �b��j
		float scaleY = rectSprite->getScaleY();	// �������u�q�ϥܰ��]���u���� X �b��j

		//------------------------------------------
		b2BodyDef staticBodyDef;
		staticBodyDef.type = b2_staticBody;
		staticBodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
		staticBodyDef.userData = rectSprite;
		b2Body* staticBody = _b2World->CreateBody(&staticBodyDef);
		//------------------------------------------

												// rectShape ���|�Ӻ��I, 0 �k�W�B 1 ���W�B 2 ���U 3 �k�U
		Point lep[4], wep[4];
		lep[0].x = (ts.width - 4) / 2.0f;;  lep[0].y = (ts.height - 4) / 2.0f;
		lep[1].x = -(ts.width - 4) / 2.0f;; lep[1].y = (ts.height - 4) / 2.0f;
		lep[2].x = -(ts.width - 4) / 2.0f;; lep[2].y = -(ts.height - 4) / 2.0f;
		lep[3].x = (ts.width - 4) / 2.0f;;  lep[3].y = -(ts.height - 4) / 2.0f;

		// �Ҧ����u�q�ϥܳ��O�O�����������I�� (0,0)�A
		// �ھ��Y��B���ಣ�ͩһݭn���x�}
		// �ھڼe�׭p��X��Ӻ��I���y�СA�M��e�W�}�x�}
		// �M��i�����A
		// Step1: ��CHECK ���L����A������h�i����I���p��
		cocos2d::Mat4 modelMatrix, rotMatrix;
		modelMatrix.m[0] = scaleX;  // ���]�w X �b���Y��
		modelMatrix.m[5] = scaleY;  // ���]�w Y �b���Y��
		cocos2d::Mat4::createRotationZ(angle*M_PI / 180.0f, &rotMatrix);
		modelMatrix.multiply(rotMatrix);
		//modelMatrix.m[3] = pntLoc.x + loc.x; //�]�w Translation�A�ۤv���[�W���˪�
		//modelMatrix.m[7] = pntLoc.y + loc.y; //�]�w Translation�A�ۤv���[�W���˪�
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


		// �����R�A��ɩһݭn�� boxShape
		b2PolygonShape boxShape;
		boxShape.Set(vecs, 4);

		b2FixtureDef fixtureDef;
		fixtureDef.shape = &boxShape;
		staticBody->CreateFixture(&fixtureDef);

		//���o���z���Y---------------------------------------------
		if (i == 10 || i == 11 || i == 12 || i == 13) {
			auto fixtrue = staticBody->GetFixtureList();
			fixtrue->SetSensor(true);	//�]���P����(�]������)
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

void CarLevel::setupRopeJoint()
{
	// ���o�ó]�w stickRope �ϥܬ��i�R�A����j
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

	//���o jeep ��T
	Point locTail = _jeepSprite->getPosition();
	Size sizeTail = _jeepSprite->getContentSize();
	auto scaleTail = _jeepSprite->getScale();

	//����÷�l���`
	b2RopeJointDef JointDef;
	JointDef.bodyA = ropeHeadBody;
	JointDef.bodyB = _CarBody;
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
	revJoint.bodyB = _CarBody;	//�P���l�s��
	revJoint.localAnchorB.Set(-67.f / PTM_RATIO, 0);
	_b2World->CreateJoint(&revJoint);
}

void CarLevel::setupJeep()
{
	// ���o�ó]�w jeep �ϥܬ��i�ʺA����j
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
	// ���o�ó]�w wheel1 ���i�ʺA����j
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
	// ���o�ó]�w wheel2 ���i�ʺA����j
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
	//�إ� Joint
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
	// ���o�ó]�w tri_line �ϥܬ��i�R�A����j
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
	fixtureDef.isSensor = true; // �]�w�� Sensor
	staticBody->CreateFixture(&fixtureDef);

	//-----------------------------------------------
	// ���o�ó]�w rect_line �ϥܬ��i�R�A����j
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
	fixtureDef2.isSensor = true; // �]�w�� Sensor
	staticBody2->CreateFixture(&fixtureDef2);

	//-----------------------------------------------
	// ���o�ó]�w jeep_line �ϥܬ��i�R�A����j
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
	fixtureDef3.isSensor = true; // �]�w�� Sensor
	staticBody3->CreateFixture(&fixtureDef3);
}

//�¤��Y
void CarLevel::showBlackWood()
{
	//��ܹϮ�
	_blackWood1->setVisible(true);
	_blackWood2->setVisible(true);
	//----------------------------------------
	_b2BlackBody1->GetFixtureList()->SetSensor(false); //�]���i����
	_b2BlackBody2->GetFixtureList()->SetSensor(false);
}
void CarLevel::destroyBlackWood()
{
	//���ùϮ�
	_blackWood1->setVisible(false);
	_blackWood2->setVisible(false);
	//----------------------------------------
	_b2BlackBody1->GetFixtureList()->SetSensor(true); //�]�����i����
	_b2BlackBody2->GetFixtureList()->SetSensor(true);
}
//�����Y
void CarLevel::showRedWood()
{
	//��ܹϮ�
	_redWood1->setVisible(true);
	_redWood2->setVisible(true);
	//----------------------------------------
	_b2RedBody1->GetFixtureList()->SetSensor(false); //�]���i����
	_b2RedBody2->GetFixtureList()->SetSensor(false);
}
void CarLevel::destroyRedWood()
{
	//���ùϮ�
	_redWood1->setVisible(false);
	_redWood2->setVisible(false);
	//----------------------------------------
	_b2RedBody1->GetFixtureList()->SetSensor(true); //�]�����i����
	_b2RedBody2->GetFixtureList()->SetSensor(true);
}
//�Ť��Y
void CarLevel::showBlueWood()
{
	//��ܹϮ�
	_blueWood->setVisible(true);
	//----------------------------------------
	_b2BlueBody->GetFixtureList()->SetSensor(false); //�]���i����
}
void CarLevel::destroyBlueWood()
{
	//���ùϮ�
	_blueWood->setVisible(false);
	//----------------------------------------
	_b2BlueBody->GetFixtureList()->SetSensor(true); //�]�����i����
}

void CarLevel::DrawLine(cocos2d::Point prePt, cocos2d::Point Pt, cocos2d::Color4F color, int num)
{
	//�e�X�u�q
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
	//�e�X�Ũ��u�q
	_drawCut[num] = DrawNode::create();
	_drawCut[num]->drawLine(firstPt, Pt, color);
	//_drawCut[num]->clear();
	_cutNode->addChild(_drawCut[num], 5);
}

void CarLevel::doStep(float dt)
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
	if (_fWinCount > 4.0f) { //����^�D��
		_fWinCount = -100.f; //�קK���ư���
		auto Scene = MainMenu::createScene();
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
			if (jointToDestroy->GetBodyA() == _contactListener._ropeBody
				/*|| jointToDestroy->GetBodyB() == _contactListener._ropeBody*/) _b2World->DestroyJoint(jointToDestroy);
			if (jointToDestroy == _MainRopeJoint)_b2World->DestroyJoint(_MainRopeJoint);	//���_�D÷
		}
		for (b2Body* b = _b2World->GetBodyList(); b; ) //�Yrope�I�� �����u�q���W��body(�קK���ƸI��) -> �@���u����@��
		{
			b2Body* bodyToDestroy = b;
			b = b->GetNext();
			if(bodyToDestroy == _contactListener._lineBody)_b2World->DestroyBody(_contactListener._lineBody);	//�����u�qBody
		}
	}

	//----------------------------------------------------------------------------
	// for sensor
	//�T���ηP����
	if (_contactListener._bTriSensor) showBlackWood();		//�Y�IĲ ��ܶ¤��Y
	else destroyBlackWood();
	//�x�ηP����
	if (_contactListener._bRectSensor) showRedWood();		//�Y�IĲ ��ܬ����Y
	else destroyRedWood();
	//���l�P����
	if (_contactListener._bJeepSensor) destroyBlueWood();	//�Y�IĲ �����Ť��Y
	else showBlueWood();

	// ---------------------------------------------------------------------------
	// ����ø�ϮĪG
	if (_bDrawing) {	//�e�Ϥ�
		if (!_blackpenbtn->getEnable() && _iFree > 0) {			//�µ��ϥΤ�
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
					b2Vec2(_tp.x / PTM_RATIO, _tp.y / PTM_RATIO), true);//���O�q
			//}
			//_iflareNum += 4;///////////////////	
				_iflareNum++;
				if (_iflareNum == SPARKS_NUMBER)_iflareNum = 0;
			}
		}
		else if (!_redpenbtn->getEnable() && _iFree_red > 0) {	//�����ϥΤ�
			_fflareCount += dt;
			if (_fflareCount > 0.1f) {
				_fflareCount -= 0.1f;
				_flare[_iflareNum]->setColor(Color3B::RED);
				_flare[_iflareNum]->setVisible(true);
				_b2flare[_iflareNum]->SetType(b2_staticBody);
				_b2flare[_iflareNum]->SetTransform(b2Vec2(_tp.x / PTM_RATIO, _tp.y / PTM_RATIO), _b2flare[_iflareNum]->GetAngle());
				_b2flare[_iflareNum]->SetType(b2_dynamicBody);
				_b2flare[_iflareNum]->ApplyForce(b2Vec2(rand() % 5 - 3, 1 + rand() % 3),
					b2Vec2(_tp.x / PTM_RATIO, _tp.y / PTM_RATIO), true);//���O�q
				_iflareNum++;
				if (_iflareNum == SPARKS_NUMBER)_iflareNum = 0;
			}
		}
		else if (!_bluepenbtn->getEnable() && _iFree_blue > 0) {	//�ŵ��ϥΤ�
			_fflareCount += dt;
			if (_fflareCount > 0.1f) {
				_fflareCount -= 0.1f;
				_flare[_iflareNum]->setColor(Color3B::BLUE);
				_flare[_iflareNum]->setVisible(true);
				_b2flare[_iflareNum]->SetType(b2_staticBody);
				_b2flare[_iflareNum]->SetTransform(b2Vec2(_tp.x / PTM_RATIO, _tp.y / PTM_RATIO), _b2flare[_iflareNum]->GetAngle());
				_b2flare[_iflareNum]->SetType(b2_dynamicBody);
				_b2flare[_iflareNum]->ApplyForce(b2Vec2(rand() % 5 - 3, 1 + rand() % 3),
					b2Vec2(_tp.x / PTM_RATIO, _tp.y / PTM_RATIO), true);//���O�q
				_iflareNum++;
				if (_iflareNum == SPARKS_NUMBER)_iflareNum = 0;
			}
		}
	}
	else {			//���e��
		_fflareCount = 0;	//���m�p��
		_iflareNum = 0;		//���m�p��
		for (int i = 0; i < SPARKS_NUMBER; i++) {	//�w��^�k
			_flare[i]->setVisible(false);
			_b2flare[i]->SetType(b2_staticBody);
			_b2flare[i]->SetTransform(b2Vec2(_Bush_dry->getPositionX() / PTM_RATIO, _Bush_dry->getPositionY() / PTM_RATIO), _b2flare[i]->GetAngle());
			_b2flare[i]->SetType(b2_dynamicBody);
		}
	}
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
	//----------------------------------------------------------------------------
	//���l�B�n
	if (_shapebtn->getStatus()) { //�Ϊ������}��
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
	else {						//�Ϊ���������
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

bool CarLevel::onTouchBegan(cocos2d::Touch *pTouch, cocos2d::Event *pEvent)//Ĳ�I�}�l�ƥ�
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
	if (!_cutbtn->getEnable()) {	//�ŤM���s�I��~�i�e��
		if (!_bBtnPressed)_bCutting = true;	//���b���s�W�~�i�e��
		if (_bCutting && _icutNum < CUTTING_NUM) {
			_cutNode = Node::create(); //�s�W�����e����`�I
			this->addChild(_cutNode, 5);
			_contactListener._cutNode = _cutNode; //�ǤJBox2d world
			_cutPt[0] = _cutPt[1] = touchLoc;	//�Ĥ@�I
		}
	}
	//--------------------------------------
	//For drawing
	else {							//�e�����s�I��~�i�e��
		if (!_bBtnPressed)_bDrawing = true;	//���b���s�W�~�i�e��
		if (_bDrawing) {
			if (!_blackpenbtn->getEnable() && _iFree > 0) {		//�µ��ϥΤ�
				_blackNode = Node::create(); //�s�W�����e����`�I
				this->addChild(_blackNode, 5);
				_contactListener._blackNode = _blackNode; //�ǤJBox2d world
				_istartPt = _iInUsed;	//�����Ĥ@�I
				_pt[_iInUsed] = touchLoc;
				_iInUsed++; _iFree--;
			}
			else if (!_redpenbtn->getEnable() && _iFree_red > 0) {	//�����ϥΤ�
				_redNode = Node::create(); //�s�W�����e����`�I
				this->addChild(_redNode, 5);
				_contactListener._redNode = _redNode; //�ǤJBox2d world
				_istartPt_red = _iInUsed_red;	//�����Ĥ@�I
				_pt_red[_iInUsed_red] = touchLoc;
				_iInUsed_red++; _iFree_red--;
			}
			else if (!_bluepenbtn->getEnable() && _iFree_blue > 0) {	//�ŵ��ϥΤ�
				_blueNode = Node::create(); //�s�W�����e����`�I
				this->addChild(_blueNode, 5);
				//_contactListener._blueNode = _blueNode; //�ǤJBox2d world
				_istartPt_blue = _iInUsed_blue;	//�����Ĥ@�I
				_pt_blue[_iInUsed_blue] = touchLoc;
				_iInUsed_blue++; _iFree_blue--;
			}
		}
	}

	return true;
}

void  CarLevel::onTouchMoved(cocos2d::Touch *pTouch, cocos2d::Event *pEvent) //Ĳ�I���ʨƥ�
{
	Point touchLoc = pTouch->getLocation();
	_tp = touchLoc;
	//-------------------------------------------------------------------
	//For cutting
	if (!_cutbtn->getEnable()) {	//�ŤM���s�I��~�i�e��
		if (_bCutting && _icutNum < CUTTING_NUM) {
			_cutPt[1] = touchLoc;
		}
	}
	//--------------------------------------
	//For drawing
	else {							//�e�����s�I��~�i�e��
		if (_bDrawing) {
			if (!_blackpenbtn->getEnable() && _iFree > 0) {		//�µ��ϥΤ�
				DrawLine(_pt[_iInUsed - 1], touchLoc, Color4F::BLACK, _iInUsed - 1); //�e�u�禡
				_pt[_iInUsed] = touchLoc;
				_iInUsed++; _iFree--;
				_penBlackBar->setPercent((float)_iFree / (float)LINE_LENGTH * 100.f); //�����q
			}
			else if (!_redpenbtn->getEnable() && _iFree_red > 0) {	//�����ϥΤ�
				DrawLine(_pt_red[_iInUsed_red - 1], touchLoc, Color4F::RED, _iInUsed_red - 1); //�e�u�禡
				_pt_red[_iInUsed_red] = touchLoc;
				_iInUsed_red++; _iFree_red--;
				_penRedBar->setPercent((float)_iFree_red / (float)LINE_LENGTH * 100.f); //�����q
			}
			else if (!_bluepenbtn->getEnable() && _iFree_blue > 0) {	//�ŵ��ϥΤ�
				DrawLine(_pt_blue[_iInUsed_blue - 1], touchLoc, Color4F::BLUE, _iInUsed_blue - 1); //�e�u�禡
				_pt_blue[_iInUsed_blue] = touchLoc;
				_iInUsed_blue++; _iFree_blue--;
				_penBlueBar->setPercent((float)_iFree_blue / (float)LINE_LENGTH * 100.f); //�����q
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

void  CarLevel::onTouchEnded(cocos2d::Touch *pTouch, cocos2d::Event *pEvent) //Ĳ�I�����ƥ� 
{
	Point touchLoc = pTouch->getLocation();
	_tp = touchLoc;
	//-------------------------------------------------------------------
	//For cutting
	if (!_cutbtn->getEnable()) {		//�ŤM���s�I��~�i�e��
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
			edgeShape.Set(b2Vec2(_cutPt[0].x / PTM_RATIO, _cutPt[0].y / PTM_RATIO),
				b2Vec2(_cutPt[1].x / PTM_RATIO, _cutPt[1].y / PTM_RATIO)); //�Ũ��u�q
			body->CreateFixture(&edgeFixtureDef);

			_icutNum++;				//�w�Ũ�����+1
			_cut1->setVisible(false);

			_bCutting = false;
		}
	}
	//--------------------------------------
	//for drawing
	else {								//�e�����s�I��~�i�e��
		if (_bDrawing) {
			if (!_blackpenbtn->getEnable()) {		//�µ��ϥΤ�
				//�إ� ��ø�i�ʺA����j
				Point loc = _blackNode->getPosition();
				Size size = _blackNode->getContentSize();
				b2BodyDef bodyDef;				//body�u���ͤ@��
				bodyDef.type = b2_dynamicBody;
				bodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
				bodyDef.userData = _blackNode;
				_b2Linebody = _b2World->CreateBody(&bodyDef);

				//��ø�i�u���j----------------------------------------------

				for (int i = _istartPt; _draw[i] != nullptr && i < LINE_LENGTH - 1; i++) { //���o�ó]�w �u�q ���i�ʺA����j
					b2EdgeShape edgeShape;
					b2FixtureDef edgeFixtureDef; // ���� Fixture
					edgeFixtureDef.shape = &edgeShape;
					edgeShape.Set(b2Vec2(_pt[i].x / PTM_RATIO, _pt[i].y / PTM_RATIO),
						b2Vec2(_pt[i + 1].x / PTM_RATIO, _pt[i + 1].y / PTM_RATIO)); //�e�Ͻu�q

					//�]�w�B�n
					if (_shapebtn->getStatus()) edgeFixtureDef.filter.categoryBits = 2;
					else edgeFixtureDef.filter.categoryBits = 1;

					_b2Linebody->CreateFixture(&edgeFixtureDef);
				}
			}
			else if (!_redpenbtn->getEnable()) {	//�����ϥΤ�
				//�إ� ��ø�i�ʺA����j
				Point loc = _redNode->getPosition();
				Size size = _redNode->getContentSize();
				b2BodyDef bodyDef;				//body�u���ͤ@��
				bodyDef.type = b2_dynamicBody;
				bodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
				bodyDef.userData = _redNode;
				_b2Linebody_red = _b2World->CreateBody(&bodyDef);

				//��ø�i�u���j----------------------------------------------

				for (int i = _istartPt_red; _draw_red[i] != nullptr && i < LINE_LENGTH - 1; i++) { //���o�ó]�w �u�q ���i�ʺA����j
					b2EdgeShape edgeShape;
					b2FixtureDef edgeFixtureDef; // ���� Fixture
					edgeFixtureDef.shape = &edgeShape;
					edgeShape.Set(b2Vec2(_pt_red[i].x / PTM_RATIO, _pt_red[i].y / PTM_RATIO),
						b2Vec2(_pt_red[i + 1].x / PTM_RATIO, _pt_red[i + 1].y / PTM_RATIO)); //�e�Ͻu�q

					//�]�w�B�n
					if (_shapebtn->getStatus()) edgeFixtureDef.filter.categoryBits = 4;
					else edgeFixtureDef.filter.categoryBits = 1;

					_b2Linebody_red->CreateFixture(&edgeFixtureDef);
				}
			}
			else if (!_bluepenbtn->getEnable()) {	//�ŵ��ϥΤ�
				//�إ� ��ø�i�ʺA����j
				Point loc = _blueNode->getPosition();
				Size size = _blueNode->getContentSize();
				b2BodyDef bodyDef;				//body�u���ͤ@��
				bodyDef.type = b2_dynamicBody;
				bodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
				bodyDef.userData = _blueNode;
				_b2Linebody_blue = _b2World->CreateBody(&bodyDef);

				//��ø�i�u���j----------------------------------------------

				for (int i = _istartPt_blue; _draw_blue[i] != nullptr && i < LINE_LENGTH - 1; i++) { //���o�ó]�w �u�q ���i�ʺA����j
					b2EdgeShape edgeShape;
					b2FixtureDef edgeFixtureDef; // ���� Fixture
					edgeFixtureDef.shape = &edgeShape;
					edgeShape.Set(b2Vec2(_pt_blue[i].x / PTM_RATIO, _pt_blue[i].y / PTM_RATIO),
						b2Vec2(_pt_blue[i + 1].x / PTM_RATIO, _pt_blue[i + 1].y / PTM_RATIO)); //�e�Ͻu�q

					//�]�w�B�n
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
	if (_retrybtn->onTouchEnded(touchLoc)) {		//���s�C���s
		auto Scene = CarLevel::createScene();
		Director::getInstance()->replaceScene(TransitionFade::create(1, Scene, Color3B(255, 255, 255)));
	}
	if (_homebtn->onTouchEnded(touchLoc)) {			//�^�����s
		auto mainScene = MainMenu::createScene();
		Director::getInstance()->replaceScene(TransitionFade::create(1, mainScene, Color3B(255, 255, 255)));
	}
	if (_blackpenbtn->onTouchEnded(touchLoc)) {		//�µ��s
		_redpenbtn->setEnable(true);
		_bluepenbtn->setEnable(true);
		_cutbtn->setEnable(true);
	}
	if (_redpenbtn->onTouchEnded(touchLoc)) {		//�����s
		_blackpenbtn->setEnable(true);
		_bluepenbtn->setEnable(true);
		_cutbtn->setEnable(true);
	}
	if (_bluepenbtn->onTouchEnded(touchLoc)) {		//�ŵ��s
		_blackpenbtn->setEnable(true);
		_redpenbtn->setEnable(true);
		_cutbtn->setEnable(true);
	}
	if (_cutbtn->onTouchEnded(touchLoc)) {			//�ŤM�s
		_redpenbtn->setEnable(true);
		_bluepenbtn->setEnable(true);
		_blackpenbtn->setEnable(true);
	}
	_shapebtn->touchesEnded(touchLoc);				//�Ϊ���
}

void CarLevel::createStaticBoundary()
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
//��gø�s��k
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
	//_NumOfSparks = 20;	//�Q�o���l��

	_bDeleteJoint = false;

	_bTriSensor = false;
	_bRectSensor = false;
	_bJeepSensor = false;
}

// �u�n�O��� body �� fixtures �I���A�N�|�I�s�o�Ө禡
void  CContactListenerLevel5::BeginContact(b2Contact* contact)
{
	b2Body* BodyA = contact->GetFixtureA()->GetBody();
	b2Body* BodyB = contact->GetFixtureB()->GetBody();
	//----------------------------------------------------------------------
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
	// cut �I����
	if (BodyA->GetFixtureList()->GetDensity() == 0.01f) { //A:rope, B:cutline
		auto shape = BodyB->GetFixtureList()->GetBody()->GetUserData();
		if (shape == _cutNode) {
			_bDeleteJoint = true;
			_ropeBody = BodyA;	//�Ǧ^rope
			_lineBody = BodyB;	//�Ǧ^line
		}
	}
	else if (BodyB->GetFixtureList()->GetDensity() == 0.01f) { //A:cutline, B:rope
		auto shape = BodyA->GetFixtureList()->GetBody()->GetUserData();
		if (shape == _cutNode) {
			_bDeleteJoint = true;
			_ropeBody = BodyB;	//�Ǧ^rope
			_lineBody = BodyA;	//�Ǧ^line
		}
	}
	//----------------------------------------------------------------------
	// Sensor ����
	if (BodyA->GetUserData() == _triSensor) { //A:�T�������I, B:�¦ⵧ��
		auto shape = BodyB->GetFixtureList()->GetBody()->GetUserData();
		if (shape == _blackNode) {
			_bTriSensor = true;
		}
	}
	else if (BodyB->GetUserData() == _triSensor) { //A:�¦ⵧ��, B:�T�������I
		auto shape = BodyA->GetFixtureList()->GetBody()->GetUserData();
		if (shape == _blackNode) {
			_bTriSensor = true;
		}
	}
	//--------------
	if (BodyA->GetUserData() == _rectSensor) { //A:�x�ΰ����I, B:���ⵧ��
		auto shape = BodyB->GetFixtureList()->GetBody()->GetUserData();
		if (shape == _redNode) {
			_bRectSensor = true;
		}
	}
	else if (BodyB->GetUserData() == _rectSensor) { //A:���ⵧ��, B:�x�ΰ����I
		auto shape = BodyA->GetFixtureList()->GetBody()->GetUserData();
		if (shape == _redNode) {
			_bRectSensor = true;
		}
	}
	//--------------
	if (BodyA->GetUserData() == _jeepSensor) { //A:���l�����I, B:���l
		auto shape = BodyB->GetFixtureList()->GetBody()->GetUserData();
		if (shape == _jeepSprite) {
			_bJeepSensor = true;
		}
	}
	else if (BodyB->GetUserData() == _jeepSensor) { //A:���l, B:���l�����I
		auto shape = BodyA->GetFixtureList()->GetBody()->GetUserData();
		if (shape == _jeepSprite) {
			_bJeepSensor = true;
		}
	}
}

//�I������
void  CContactListenerLevel5::EndContact(b2Contact* contact)
{
	b2Body* BodyA = contact->GetFixtureA()->GetBody();
	b2Body* BodyB = contact->GetFixtureB()->GetBody();

	// Sensor ����
	if (BodyA->GetUserData() == _triSensor) { //A:�T�������I, B:�¦ⵧ��
		auto shape = BodyB->GetFixtureList()->GetBody()->GetUserData();
		if (shape == _blackNode) {
			_bTriSensor = false;
		}
	}
	else if (BodyB->GetUserData() == _triSensor) { //A:�¦ⵧ��, B:�T�������I
		auto shape = BodyA->GetFixtureList()->GetBody()->GetUserData();
		if (shape == _blackNode) {
			_bTriSensor = false;
		}
	}
	//--------------
	if (BodyA->GetUserData() == _rectSensor) { //A:�x�ΰ����I, B:���ⵧ��
		auto shape = BodyB->GetFixtureList()->GetBody()->GetUserData();
		if (shape == _redNode) {
			_bRectSensor = false;
		}
	}
	else if (BodyB->GetUserData() == _rectSensor) { //A:���ⵧ��, B:�x�ΰ����I
		auto shape = BodyA->GetFixtureList()->GetBody()->GetUserData();
		if (shape == _redNode) {
			_bRectSensor = false;
		}
	}
	//--------------
	if (BodyA->GetUserData() == _jeepSensor) { //A:���l�����I, B:���l
		auto shape = BodyB->GetFixtureList()->GetBody()->GetUserData();
		if (shape == _jeepSprite) {
			_bJeepSensor = false;
		}
	}
	else if (BodyB->GetUserData() == _jeepSensor) { //A:���l, B:���l�����I
		auto shape = BodyA->GetFixtureList()->GetBody()->GetUserData();
		if (shape == _jeepSprite) {
			_bJeepSensor = false;
		}
	}
}