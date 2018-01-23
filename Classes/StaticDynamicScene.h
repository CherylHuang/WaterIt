#ifndef __STATICDYNAMIC_SCENE_H__
#define __STATICDYNAMIC_SCENE_H__

//#define BOX2D_DEBUG 1
#define SET_GRAVITY_BUTTON 1

#include "cocos2d.h"
#include "Box2D/Box2D.h"
#include "Common/CButton.h"

#ifdef BOX2D_DEBUG
#include "Common/GLES-Render.h"
#include "Common/GB2ShapeCache-x.h"
#endif

#define PTM_RATIO 32.0f
#define RepeatCreateBallTime 3
#define AccelerateMaxNum 2
#define AccelerateRatio 1.5f

class StaticDynamic : public cocos2d::Layer
{
public:

	~StaticDynamic();
    // there's no 'id' in cpp, so we recommend returning the class instance pointer
    static cocos2d::Scene* createScene();

	// for Box2D
	b2World* _b2World;
	cocos2d::Label *_titleLabel;
	cocos2d::Size _visibleSize;

	b2BodyDef _BallBodyDef;
	b2CircleShape _BallShape;
	b2FixtureDef _BallFixtureDef;

	// Box2D Examples
	void readBlocksCSBFile(const char *);
	void readSceneFile(const char *);
	void createStaticBoundary();
	void setGravityButton();

#ifdef SET_GRAVITY_BUTTON
	// 四個重力方向的按鈕
	CButton *_gravityBtn[4]; // 0 往下、1 往左、2 往上、3 往右
#endif

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
    CREATE_FUNC(StaticDynamic);
};

#endif // __StaticDynamic_SCENE_H__
