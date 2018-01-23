#ifndef __JOINTLEVEL_H__
#define __JOINTLEVEL_H__

//#define BOX2D_DEBUG 1

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "SimpleAudioEngine.h"
#include "cocostudio/CocoStudio.h"
#include "Box2D/Box2D.h"
#include "Common/C3SButton.h"
#include "GearLevel.h" //下一關
#include "MainMenu.h"

#ifdef BOX2D_DEBUG
#include "Common/GLES-Render.h"
#include "Common/GB2ShapeCache-x.h"
#endif

#define PTM_RATIO 32.0f
#define RepeatCreateBallTime 3
#define AccelerateMaxNum 2
#define AccelerateRatio 1.5f

#define SPARKS_NUMBER 20 //分子數
#define LINE_LENGTH 200 //可畫線的長度

//碰撞聆聽類別
class CContactListenerLevel1 : public b2ContactListener
{
public:
	// dot joint
	cocos2d::Sprite *_targetSprite;
	cocos2d::Node *_newNode;		//圖畫節點
	bool _bRevoluteJoint;			//偵測碰撞，執行連結

	//start & end
	bool _bWin;						//通過關卡
	bool _bCreateSpark;				//產生水花
	//int  _NumOfSparks;				//噴發分子數
	b2Vec2 _createLoc;				//噴發點
	cocos2d::Sprite *_StartSprite;	//water
	cocos2d::Sprite *_EndSprite;	//bush

	CContactListenerLevel1();
	//碰撞開始
	virtual void BeginContact(b2Contact* contact);
	//碰撞結束
	virtual void EndContact(b2Contact* contact);
	void setCollisionTarget(cocos2d::Sprite &targetSprite);
};

class JointLevel : public cocos2d::Layer
{
public:
	~JointLevel();
	// there's no 'id' in cpp, so we recommend returning the class instance pointer
	static cocos2d::Scene* createScene();
	Node *_csbRoot;

	//碰撞事件
	CContactListenerLevel1 _contactListener;
	b2Body *_dotBody;
	bool _bJointCount; //延遲計時
	float _fcount; //延遲判斷時間

	//ui
	bool _bBtnPressed;	//是否按在按鈕上
	cocos2d::ui::LoadingBar *_penBlackBar;	//墨水量條
	C3SButton *_retrybtn;
	C3SButton *_homebtn;

	//drawing
	cocos2d::Node *_newNode;					//圖畫節點
	cocos2d::Point _pt[LINE_LENGTH];			//記錄點座標
	b2Vec2 _b2vec[LINE_LENGTH];					//記錄點座標
	b2Vec2 _b2polyVec[3];						//記錄畫多邊形點座標(分解成多三角)
	cocos2d::DrawNode *_draw[LINE_LENGTH - 1];	//紀錄線段
	b2Body *_b2Linebody;						//紀錄物理世界線段
	bool _bDrawing;								//是否畫圖中
	int _iFree, _iInUsed;						//紀錄所用點數
	int _istartPt;								//紀錄開頭點的索引值
	void DrawLine(cocos2d::Point prePt, cocos2d::Point Pt, cocos2d::Color4F color, int num);
	
	//畫圖特效
	int _iflareNum;
	float _fflareCount;
	cocos2d::Point _tp;	//滑鼠點
	cocos2d::Sprite *_flare[SPARKS_NUMBER];	//儲存分子
	b2Body *_b2flare[SPARKS_NUMBER];

	// for Start and End
	float _fStartCount;				//重複從空中放球計時
	float _fWinCount;				//跳轉關卡計時
	b2Body *_waterBody;				//水球物理Body
	cocos2d::Point _startPt;		//起點
	cocos2d::Sprite *_startWater;
	cocos2d::Sprite *_Bush_dry;
	cocos2d::Sprite *_Bush_live;
	cocos2d::Sprite *_sparks[SPARKS_NUMBER];	//儲存分子

	// for Box2D
	b2World* _b2World;
	cocos2d::Size _visibleSize;

	// for MouseJoint
	b2Body *_bottomBody; // 底部的 edgeShape
	b2MouseJoint* _MouseJoint;
	bool _bTouchOn;

	// Box2D Examples
	void createStaticBoundary();

	void setupMouseJoint();

	//函式
	void setupStatic();			//一般靜態物件
	void setJointDot();			//連結點
	void setStartEndpoint();	//水球 & 終點

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
	CREATE_FUNC(JointLevel);
};

#endif