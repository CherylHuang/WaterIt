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


//  Box2D的碰撞偵測由 b2ContactListener 控制，要取得碰撞事件必須先繼承此類別

class CContactListener : public b2ContactListener
{
public:
	cocos2d::Sprite *_targetSprite; // 用於判斷是否
	bool _bCreateSpark;		//產生火花
	bool _bApplyImpulse;	// 產生瞬間的衝力
	b2Vec2 _createLoc;
	int  _NumOfSparks;
	CContactListener();
	//碰撞開始
	virtual void BeginContact(b2Contact* contact);
	//碰撞結束
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
	float _tdelayTime; // 用於火花的產生，不要事件進入太多而導致一下產生過多的火花
	bool  _bSparking;  // true: 可以噴出火花，false: 不行	

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
	bool onTouchBegan(cocos2d::Touch *pTouch, cocos2d::Event *pEvent); //觸碰開始事件
	void onTouchMoved(cocos2d::Touch *pTouch, cocos2d::Event *pEvent); //觸碰移動事件
	void onTouchEnded(cocos2d::Touch *pTouch, cocos2d::Event *pEvent); //觸碰結束事件 

    // implement the "static create()" method manually
    CREATE_FUNC(FixtureCollision);
};

#endif // __FixtureCollision_SCENE_H__
