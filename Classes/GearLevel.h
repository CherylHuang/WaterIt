#ifndef __GEAR_H__
#define __GEAR_H__

//#define BOX2D_DEBUG 1

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "SimpleAudioEngine.h"
#include "cocostudio/CocoStudio.h"
#include "Box2D/Box2D.h"
#include "Common/C3SButton.h"
#include "GravityLevel.h" //下一關
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
#define CUTTING_NUM 4	//可剪取次數

//碰撞聆聽類別
class  CContactListenerLevel4 : public b2ContactListener
{
public:
	//cutting
	bool _bDeleteJoint;				//是否要刪除Joint
	bool _bDeleteRope1;				//切到rope1
	bool _bDeleteRope2;				//切到rope2
	bool _bDeleteRope3;				//切到rope3
	bool _bDeleteRope4;				//切到rope4
	cocos2d::Node *_cutNode;		//剪取節點
	b2Body *_ropeBody;
	b2Body *_lineBody;

	//transition door
	bool _bMove;					//是否傳送
	cocos2d::Sprite *_DoorIn;		//door in

	//start & end
	bool _bWin;						//通過關卡
	bool _bCreateSpark;				//產生水花
	//int  _NumOfSparks;				//噴發分子數
	b2Vec2 _createLoc;				//噴發點
	cocos2d::Sprite *_StartSprite;	//water
	cocos2d::Sprite *_EndSprite;	//bush

	 CContactListenerLevel4();
	//碰撞開始
	virtual void BeginContact(b2Contact* contact);
	//碰撞結束
	virtual void EndContact(b2Contact* contact);
};

class GearLevel : public cocos2d::Layer
{
public:
	~GearLevel();
	// there's no 'id' in cpp, so we recommend returning the class instance pointer
	static cocos2d::Scene* createScene();
	Node *_csbRoot;

	//碰撞事件
	 CContactListenerLevel4 _contactListener;

	//ui
	bool _bBtnPressed;	//是否按在按鈕上
	C3SButton *_retrybtn;
	C3SButton *_homebtn;
	C3SButton *_cutbtn;

	// for Start and End
	float _fStartCount;				//重複從空中放球計時
	float _fWinCount;				//跳轉關卡計時
	b2Body *_waterBody;				//水球物理Body
	cocos2d::Point _startPt;		//起點
	cocos2d::Sprite *_startWater;
	cocos2d::Sprite *_Bush_dry;
	cocos2d::Sprite *_Bush_live;
	cocos2d::Sprite *_sparks[SPARKS_NUMBER];	//儲存分子

	// for cutting
	int _icutNum;					//剪取次數
	bool _bCutting;					//是否畫圖中
	cocos2d::Node *_cutNode;		//剪取節點
	cocos2d::Point _cutPt[2];		//記錄剪取直線兩端點座標
	cocos2d::Point _preCutPt;		//連續顯示用
	cocos2d::DrawNode *_drawCut[CUTTING_NUM];	//紀錄線段
	void DrawCuttingLine(cocos2d::Point firstPt, cocos2d::Point Pt, cocos2d::Color4F color, int num);
	b2RopeJoint *_MainRopeJoint;	//主連接繩1
	b2RopeJoint *_MainRope2Joint;	//主連接繩2
	b2RopeJoint *_MainRope3Joint;	//主連接繩3
	b2RopeJoint *_MainRope4Joint;	//主連接繩4
	b2Body *_cactusBody;			//重物
	b2Body *_cactus2Body;			//重物
	int _iCutLineNum;						//剪取線段數
	cocos2d::Point _tp;						//touchLoc(畫線用)
	cocos2d::Point _tp_start;				//start touchLoc(畫線用)
	DrawNode *_CutDrawLine;					//剪取線
	DrawNode *_destroyCutDrawLine;			//儲存並移除前一條線

	//for pully joint + 滑輪繩
	b2Body *_Board;					//主木板物理Body
	b2Body *_leftBoard;				//左木板物理Body
	b2Body *_rightBoard;			//右木板物理Body
	cocos2d::Sprite *_gear1;		//第一滑輪
	cocos2d::Sprite *_cactus1;		//重物1
	cocos2d::Sprite *_gear2;		//第二滑輪
	cocos2d::Sprite *_cactus2;		//重物2
	cocos2d::Sprite *_board;		//木板
	cocos2d::Sprite *_board_left;	//左木板
	cocos2d::Sprite *_board_right;	//右木板
	int _iLineNum;					//畫線數量
	DrawNode *_pulleyline1;							//滑輪線
	DrawNode *_destroyLine1;						//儲存並移除前一條線
	DrawNode *_pulleyline2, *_destroyLine2;			//滑輪線
	DrawNode *_pulleyline3, *_destroyLine3;			//滑輪線
	DrawNode *_pulleyline4, *_destroyLine4;			//滑輪線

	// for gears
	cocos2d::Sprite *_fakeTurn1;	//設為重物
	cocos2d::Sprite *_fakeTurn2;
	b2Body *_b2fakeTurn1, *_b2fakeTurn2;	//重物
	b2Body *_groundbody, *_groundbody2;		//連結滑輪 底部靜態物體

	//for transition door
	cocos2d::Sprite *_DoorIn;		//transDoor_in
	cocos2d::Sprite *_DoorOut;		//transDoor_out

	// for Box2D
	b2World* _b2World;
	cocos2d::Size _visibleSize;
	b2Body *_bottomBody; // 底部的 edgeShape

	//函式
	void createStaticBoundary(); //邊界
	void setupStatic();			//一般靜態物件
	void setStartEndpoint();	//水球 & 終點

	void setupWoodBoard();			//木板設定
	void setupRopeJoint();			//繩子1
	void setupRope2Joint();			//繩子2
	void setupRope3Joint();			//繩子3
	void setupRope4Joint();			//繩子4
	void setupPulleyJoint();		//滑輪
	void setupMoveDoor();			//傳送門
	void setupGearStatic();			//滑輪齒輪靜態物體設定
	void setupGear();				//齒輪


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
	CREATE_FUNC(GearLevel);
};

#endif