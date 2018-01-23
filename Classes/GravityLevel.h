#ifndef __GRAVITYLEVEL_H__
#define __GRAVITYLEVEL_H__

//#define BOX2D_DEBUG 1

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "SimpleAudioEngine.h"
#include "cocostudio/CocoStudio.h"
#include "Box2D/Box2D.h"
#include "Common/C3SButton.h"
#include "CarLevel.h" //下一關
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
#define CUTTING_NUM 2	//可剪取次數

//碰撞聆聽類別
class  CContactListenerLevel3 : public b2ContactListener
{
public:
	cocos2d::Node *_newNode;		//圖畫節點

	//cutting
	bool _bDeleteJoint;				//是否要刪除Joint
	cocos2d::Node *_cutNode;		//剪取節點
	b2Body *_ropeBody;
	b2Body *_lineBody;

	//gravity
	bool _bGravityUp;				//重力向上是否開啟
	bool _bGravityDown;				//重力向下是否開啟
	cocos2d::Sprite *_gravityUpSprite;
	cocos2d::Sprite *_gravityDownSprite;

	//start & end
	bool _bWin;						//通過關卡
	bool _bCreateSpark;				//產生水花
	//int  _NumOfSparks;				//噴發分子數
	b2Vec2 _createLoc;				//噴發點
	cocos2d::Sprite *_StartSprite;	//water
	cocos2d::Sprite *_EndSprite;	//bush

	 CContactListenerLevel3();
	//碰撞開始
	virtual void BeginContact(b2Contact* contact);
	//碰撞結束
	virtual void EndContact(b2Contact* contact);
};

class GravityLevel : public cocos2d::Layer
{
public:
	~GravityLevel();
	// there's no 'id' in cpp, so we recommend returning the class instance pointer
	static cocos2d::Scene* createScene();
	Node *_csbRoot;

	//碰撞事件
	 CContactListenerLevel3 _contactListener;

	//ui
	bool _bBtnPressed;	//是否按在按鈕上
	cocos2d::ui::LoadingBar *_penBlackBar;	//墨水量條
	C3SButton *_retrybtn;
	C3SButton *_homebtn;
	C3SButton *_blackpenbtn;
	C3SButton *_cutbtn;

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

	// for cutting
	int _icutNum;					//剪取次數
	bool _bCutting;					//是否畫圖中
	b2RopeJoint *_MainRopeJoint;	//主連接繩
	cocos2d::Node *_cutNode;		//剪取節點
	cocos2d::Point _cutPt[2];		//記錄剪取直線兩端點座標
	cocos2d::Point _preCutPt;		//連續顯示用
	cocos2d::Sprite *_cut1;			//UI
	cocos2d::Sprite *_cut2;			//UI
	cocos2d::DrawNode *_drawCut[CUTTING_NUM];	//紀錄線段
	void DrawCuttingLine(cocos2d::Point firstPt, cocos2d::Point Pt, cocos2d::Color4F color, int num);
	int _iCutLineNum;						//剪取線段數
	cocos2d::Point _tp_start;				//start touchLoc(畫線用)
	DrawNode *_CutDrawLine;					//剪取線
	DrawNode *_destroyCutDrawLine;			//儲存並移除前一條線

	// for gravity point
	cocos2d::Sprite *_gravityUp;
	cocos2d::Sprite *_gravityUp_on;
	cocos2d::Sprite *_gravityDown;
	cocos2d::Sprite *_gravityDown_on;

	// for Box2D
	b2World* _b2World;
	cocos2d::Size _visibleSize;
	b2Body *_bottomBody; // 底部的 edgeShape

	//函式
	void createStaticBoundary(); //邊界
	void setupStatic();			//一般靜態物件
	void setStartEndpoint();	//水球 & 終點
	
	void setupSeesaw1();			//蹺蹺板1設定
	void setupSeesaw2();			//蹺蹺板2設定
	void setupGravityPt();			//重力轉換點
	void setupRopeJoint();			//繩子

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
	CREATE_FUNC(GravityLevel);
};

#endif