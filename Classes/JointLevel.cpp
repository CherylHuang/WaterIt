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
	//�e��
	_iFree = LINE_LENGTH;
	_iInUsed = 0;
	_bDrawing = false;
	//----------------------------------------------------------
	//Joint�I�I��
	_bJointCount = false;
	_fcount = 0;
	//----------------------------------------------------------
	// �إ� Box2D world
	_b2World = nullptr;
	b2Vec2 Gravity = b2Vec2(0.0f, -9.8f);		//���O��V
	bool AllowSleep = true;			//���\�ε�
	_b2World = new b2World(Gravity);	//�Ыإ@��
	_b2World->SetAllowSleeping(AllowSleep);	//�]�w���󤹳\�ε�

											// Create Scene with csb file
	_csbRoot = CSLoader::createNode("JointLevel.csb");
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

	//----------------------------------------------------------
	//Start & End
	_fStartCount = 0;
	_contactListener._StartSprite = _startWater;
	_contactListener._EndSprite = _Bush_dry;
	_startPt = _startWater->getPosition();
	_fWinCount = 0;

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

	//----------------------------------------------------------
	setStartEndpoint();			//�_���I
	createStaticBoundary();		//���
	setupStatic();				//�R�A����
	setJointDot();				//��ø����Joint�I

	setupMouseJoint();

	//------------------------------------------------------------------------
	//ø�ϯS�Ŀ���
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
	_listener1->onTouchBegan = CC_CALLBACK_2(JointLevel::onTouchBegan, this);		//�[�JĲ�I�}�l�ƥ�
	_listener1->onTouchMoved = CC_CALLBACK_2(JointLevel::onTouchMoved, this);		//�[�JĲ�I���ʨƥ�
	_listener1->onTouchEnded = CC_CALLBACK_2(JointLevel::onTouchEnded, this);		//�[�JĲ�I���}�ƥ�

	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(_listener1, this);	//�[�J��Ыت��ƥ��ť��
	this->schedule(CC_SCHEDULE_SELECTOR(JointLevel::doStep));

	return true;
}

void JointLevel::setupStatic()
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

		// �����R�A��ɩһݭn�� boxShape
		b2PolygonShape boxShape;
		boxShape.SetAsBox((size.width*0.5f - 2.0f)*scale / PTM_RATIO, (size.height*0.5f - 2.0f)*scale / PTM_RATIO);

		b2FixtureDef fixtureDef;
		fixtureDef.shape = &boxShape;
		staticBody->CreateFixture(&fixtureDef);
	}
}

void JointLevel::setJointDot()
{
	// ���o�ó]�w JointDot �ϥܬ��i�R�A����j
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
	fixtureDef.isSensor = true; // �]�w�� Sensor
	_dotBody->CreateFixture(&fixtureDef);

	_contactListener.setCollisionTarget(*DotSprite);
}
void JointLevel::setStartEndpoint()
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

void JointLevel::setupMouseJoint()
{
	// ���o�ó]�w frame01 �e�عϥܬ��ʺA����
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
	//�e�X�u�q
	_draw[num] = DrawNode::create();
	_draw[num]->drawLine(prePt, Pt, color);
	_newNode->addChild(_draw[num], 5);
	//CCLOG("%d", num);
}

void JointLevel::doStep(float dt)
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
			spriteData->setPosition(body->GetPosition().x*PTM_RATIO, body->GetPosition().y*PTM_RATIO);
			spriteData->setRotation(-1 * CC_RADIANS_TO_DEGREES(body->GetAngle()));
		}
//		// �]�X�ù��~���N������q b2World ������
//		if (body->GetType() == b2BodyType::b2_dynamicBody) {
//			float x = body->GetPosition().x * PTM_RATIO;
//			float y = body->GetPosition().y * PTM_RATIO;
//			if (x > _visibleSize.width || x < 0 || y >  _visibleSize.height || y < 0) {
//				if (body->GetUserData() != NULL) {
//					Sprite* spriteData = (Sprite *)body->GetUserData();
//					this->removeChild(spriteData);
//				}
//				b2Body* nextbody = body->GetNext(); // ���o�U�@�� body
//				_b2World->DestroyBody(body); // ����ثe�� body
//				body = nextbody;  // �� body ���V��~���o���U�@�� body
//#ifdef CREATED_REMOVED
//				g_totRemoved2++;
//				CCLOG("Removing %4d Particles", g_totRemoved2);
//#endif
//			}
//			else body = body->GetNext(); //�_�h�N�~���s�U�@��Body
//		}
//		else body = body->GetNext(); //�_�h�N�~���s�U�@��Body
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
		auto Scene = GearLevel::createScene();
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
	// for drawing
	if (_bJointCount) _fcount += dt;
	if (_b2Linebody != nullptr) {
		if (_contactListener._bRevoluteJoint && _fcount > 0.01f) {		//���M�s���I�I���A���ͳs��
			_bJointCount = false; //�����p��
			_fcount = 0; //���]�p��

			b2RevoluteJointDef revoluteJoint;		// �إ߱��� Joint �s��
			revoluteJoint.bodyA = _dotBody;
			revoluteJoint.localAnchorA.Set(0, 0);
			revoluteJoint.bodyB = _b2Linebody;
			auto pos = _b2Linebody->GetPosition();
			revoluteJoint.localAnchorB.Set(pos.x + 640.f / PTM_RATIO, pos.y + 360.f / PTM_RATIO);
			_b2World->CreateJoint(&revoluteJoint);
		}
		else if (!(_contactListener._bRevoluteJoint) && _fcount > 0.01f) {	//�S���M�s���I���A����egde����
			_bJointCount = false;
			_fcount = 0;

			for (b2Fixture* f = _b2Linebody->GetFixtureList(); f; ) //�����ثebody���W�Ҧ�fixture
			{
				b2Fixture* fixtureToDestroy = f;
				f = f->GetNext();
				_b2Linebody->DestroyFixture(fixtureToDestroy);
			}

			//��ø�i�u���j----------------------------------------------

			for (int i = _istartPt; _draw[i] != nullptr && i < LINE_LENGTH - 1; i++) { //���o�ó]�w �u�q ���i�ʺA����j
				b2EdgeShape edgeShape;
				b2FixtureDef edgeFixtureDef; // ���� Fixture
				edgeFixtureDef.shape = &edgeShape;
				edgeShape.Set(b2Vec2(_pt[i].x / PTM_RATIO, _pt[i].y / PTM_RATIO), b2Vec2(_pt[i + 1].x / PTM_RATIO, _pt[i + 1].y / PTM_RATIO)); //�e�Ͻu�q
				_b2Linebody->CreateFixture(&edgeFixtureDef);
			}
		}
	}
	// ---------------------------------------------------------------------------
	// ����ø�ϮĪG
	if (_bDrawing) {	//�e�Ϥ�
		if (_iFree > 0) {			//�µ��ϥΤ�
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

}

bool JointLevel::onTouchBegan(cocos2d::Touch *pTouch, cocos2d::Event *pEvent)//Ĳ�I�}�l�ƥ�
{
	Point touchLoc = pTouch->getLocation();
	_tp = touchLoc;
	//------------------------------------------------------------
	// For Mouse Joint 
	for (b2Body* body = _b2World->GetBodyList(); body; body = body->GetNext())
	{
		if (body->GetUserData() == NULL) continue; // �R�A���餣�B�z
												   // �P�_�I����m�O�_���b�ʺA����@�w���d��
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
			_MouseJoint = (b2MouseJoint*)_b2World->CreateJoint(&mouseJointDef); // �s�W Mouse Joint
			body->SetAwake(true);
			break;
		}
	}
	//-------------------------------------------------------------------
	//For button
	if (_retrybtn->onTouchBegan(touchLoc) || _homebtn->onTouchBegan(touchLoc)) _bBtnPressed = true;

	//-------------------------------------------------------------------
	//For drawing
	if (!_bTouchOn && !_bBtnPressed)_bDrawing = true;	//���b���s�W�~�i�e��
	if (_bDrawing && _iFree > 0) {
		_newNode = Node::create(); //�s�W�����e����`�I
		this->addChild(_newNode, 5);
		_contactListener._newNode = _newNode; //�ǤJBox2d world

		_pt[_iInUsed] = touchLoc;
		_b2vec[_iInUsed].x = touchLoc.x / PTM_RATIO;
		_b2vec[_iInUsed].y = touchLoc.y / PTM_RATIO;
		_istartPt = _iInUsed;
		_iInUsed++; _iFree--;
	}

	return true;
}

void  JointLevel::onTouchMoved(cocos2d::Touch *pTouch, cocos2d::Event *pEvent) //Ĳ�I���ʨƥ�
{
	Point touchLoc = pTouch->getLocation();
	_tp = touchLoc;
	//------------------------------------------------------------
	if (_bDrawing && _iFree > 0) {
		DrawLine(_pt[_iInUsed - 1], touchLoc, Color4F::BLACK, _iInUsed - 1); //�e�u�禡
		_b2vec[_iInUsed].x = touchLoc.x / PTM_RATIO;
		_b2vec[_iInUsed].y = touchLoc.y / PTM_RATIO;
		_pt[_iInUsed] = touchLoc;
		_iInUsed++; _iFree--;
		_penBlackBar->setPercent((float)_iFree / (float)LINE_LENGTH * 100.f); //�����q
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

void  JointLevel::onTouchEnded(cocos2d::Touch *pTouch, cocos2d::Event *pEvent) //Ĳ�I�����ƥ� 
{
	Point touchLoc = pTouch->getLocation();
	_tp = touchLoc;
	//----------------------------------------------------------------
	if (_bDrawing) {

		//�إ� ��ø�i�ʺA����j
		Point loc = _newNode->getPosition();
		Size size = _newNode->getContentSize();
		b2BodyDef bodyDef;				//body�u���ͤ@��
		bodyDef.type = b2_dynamicBody;
		bodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
		bodyDef.userData = _newNode;
		_b2Linebody = _b2World->CreateBody(&bodyDef);

		//��ø�i�h��Ρj--------------------------------------------
		_b2polyVec[0] = _b2vec[_istartPt]; //�T���γ��I
		for (int i = _istartPt + 1; _draw[i] != nullptr && i < LINE_LENGTH - 1; i++) { //���o�ó]�w �u�q ���i�ʺA����j
			b2PolygonShape polyShape;
			b2FixtureDef fixtureDef; // ���� Fixture
			fixtureDef.shape = &polyShape;

			_b2polyVec[1] = _b2vec[i]; //�C�@�u�q�P�_�I���ͤT����
			_b2polyVec[2] = _b2vec[i + 1];

			polyShape.Set(_b2polyVec, 3); //polygon�����p�T����
			_b2Linebody->CreateFixture(&fixtureDef); //�[�J�Dbody
		}
		_bJointCount = true; //����p��
		_bDrawing = false;
	}
	_contactListener._bRevoluteJoint = false;	//���s�����I��

	//---------------------------------------------------------------
	//Button
	_bBtnPressed = false;
	if (_retrybtn->onTouchEnded(touchLoc)) {		//���s�C���s
		auto Scene = JointLevel::createScene();
		Director::getInstance()->replaceScene(TransitionFade::create(1, Scene, Color3B(255, 255, 255)));
	}
	if (_homebtn->onTouchEnded(touchLoc)) {			//�^�����s
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
	//_NumOfSparks = 20;	//�Q�o���l��
}
void CContactListenerLevel1::setCollisionTarget(cocos2d::Sprite &targetSprite)
{
	_targetSprite = &targetSprite;
}

// �u�n�O��� body �� fixtures �I���A�N�|�I�s�o�Ө禡
void CContactListenerLevel1::BeginContact(b2Contact* contact)
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
	//Joint�I����
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

//�I������
void CContactListenerLevel1::EndContact(b2Contact* contact)
{

}