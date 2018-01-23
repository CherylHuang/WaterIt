#ifndef __FIXTURECOLLISION_SCENE_H__
#define __FIXTURECOLLISION_SCENE_H__

//#define BOX2D_DEBUG 1

#include "cocos2d.h"
#include "Box2D/Box2D.h"
#include "Common/CButton.h"
#include "Common/CLight.h"

#ifdef BOX2D_DEBUG
#include "Common/GLES-Render.h"
#include "Common/GB2ShapeCache-x.h"
#endif

#define PTM_RATIO 32.0f
#define RepeatCreateBallTime 3
#define AccelerateMaxNum 2
#define AccelerateRatio 1.5f


//  Box2D���I�������� b2ContactListener ����A�n���o�I���ƥ󥲶����~�Ӧ����O

class CContactListener : public b2ContactListener
{
public:
	cocos2d::Sprite *_targetSprite; // �Ω�P�_�O�_
	bool _bCreateSpark;		//���ͤ���
	bool _bApplyImpulse;	// �����������ĤO
	b2Vec2 _createLoc;
	int  _NumOfSparks;
	CContactListener();
	//�I���}�l
	virtual void BeginContact(b2Contact* contact);
	//�I������
	virtual void EndContact(b2Contact* contact);
	void setCollisionTarget(cocos2d::Sprite &targetSprite);
};


class FixtureCollision : public cocos2d::Layer
{
public:

	~FixtureCollision();
    // there's no 'id' in cpp, so we recommend returning the class instance pointer
    static cocos2d::Scene* createScene();
	Node *_csbRoot;

	// for Box2D
	b2World* _b2World;
	cocos2d::Label *_titleLabel;
	cocos2d::Size _visibleSize;

	// For FrictionAndFilter Example
	CButton *_rectButton;
	int _iNumofRect;

	// For Sensor And Collision Example
	CLight *_light1;
	bool _bReleasingBall;
	CButton *_ballBtn;
	CContactListener _contactListener;
	cocos2d::Sprite *_collisionSprite;
	cocos2d::BlendFunc blendFunc;
	float _tdelayTime; // �Ω���᪺���͡A���n�ƥ�i�J�Ӧh�ӾɭP�@�U���͹L�h������
	bool  _bSparking;  // true: �i�H�Q�X����Afalse: ����	

	// Box2D Examples
	void setStaticWalls();
	void setupDesnity();
	void setupFrictionAndFilter();
	void setupSensorAndCollision();
	void createStaticBoundary();


#ifdef BOX2D_DEBUG
	//DebugDraw
	GLESDebugDraw* _DebugDraw;
	virtual void draw(cocos2d::Renderer* renderer, const cocos2d::Mat4& transform, uint32_t flags);
#endif

    // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
    virtual bool init();
	void doStep(float dt);

	cocos2d::EventListenerTouchOneByOne *_listener1;
	bool onTouchBegan(cocos2d::Touch *pTouch, cocos2d::Event *pEvent); //Ĳ�I�}�l�ƥ�
	void onTouchMoved(cocos2d::Touch *pTouch, cocos2d::Event *pEvent); //Ĳ�I���ʨƥ�
	void onTouchEnded(cocos2d::Touch *pTouch, cocos2d::Event *pEvent); //Ĳ�I�����ƥ� 

    // implement the "static create()" method manually
    CREATE_FUNC(FixtureCollision);
};

#endif // __FixtureCollision_SCENE_H__
