#ifndef __GRAVITYLEVEL_H__
#define __GRAVITYLEVEL_H__

//#define BOX2D_DEBUG 1

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "SimpleAudioEngine.h"
#include "cocostudio/CocoStudio.h"
#include "Box2D/Box2D.h"
#include "Common/C3SButton.h"
#include "CarLevel.h" //�U�@��
#include "MainMenu.h"

#ifdef BOX2D_DEBUG
#include "Common/GLES-Render.h"
#include "Common/GB2ShapeCache-x.h"
#endif

#define PTM_RATIO 32.0f
#define RepeatCreateBallTime 3
#define AccelerateMaxNum 2
#define AccelerateRatio 1.5f

#define SPARKS_NUMBER 20 //���l��
#define LINE_LENGTH 200 //�i�e�u������
#define CUTTING_NUM 2	//�i�Ũ�����

//�I����ť���O
class  CContactListenerLevel3 : public b2ContactListener
{
public:
	cocos2d::Node *_newNode;		//�ϵe�`�I

	//cutting
	bool _bDeleteJoint;				//�O�_�n�R��Joint
	cocos2d::Node *_cutNode;		//�Ũ��`�I
	b2Body *_ropeBody;
	b2Body *_lineBody;

	//gravity
	bool _bGravityUp;				//���O�V�W�O�_�}��
	bool _bGravityDown;				//���O�V�U�O�_�}��
	cocos2d::Sprite *_gravityUpSprite;
	cocos2d::Sprite *_gravityDownSprite;

	//start & end
	bool _bWin;						//�q�L���d
	bool _bCreateSpark;				//���ͤ���
	//int  _NumOfSparks;				//�Q�o���l��
	b2Vec2 _createLoc;				//�Q�o�I
	cocos2d::Sprite *_StartSprite;	//water
	cocos2d::Sprite *_EndSprite;	//bush

	 CContactListenerLevel3();
	//�I���}�l
	virtual void BeginContact(b2Contact* contact);
	//�I������
	virtual void EndContact(b2Contact* contact);
};

class GravityLevel : public cocos2d::Layer
{
public:
	~GravityLevel();
	// there's no 'id' in cpp, so we recommend returning the class instance pointer
	static cocos2d::Scene* createScene();
	Node *_csbRoot;

	//�I���ƥ�
	 CContactListenerLevel3 _contactListener;

	//ui
	bool _bBtnPressed;	//�O�_���b���s�W
	cocos2d::ui::LoadingBar *_penBlackBar;	//�����q��
	C3SButton *_retrybtn;
	C3SButton *_homebtn;
	C3SButton *_blackpenbtn;
	C3SButton *_cutbtn;

	//drawing
	cocos2d::Node *_newNode;					//�ϵe�`�I
	cocos2d::Point _pt[LINE_LENGTH];			//�O���I�y��
	b2Vec2 _b2vec[LINE_LENGTH];					//�O���I�y��
	b2Vec2 _b2polyVec[3];						//�O���e�h����I�y��(���Ѧ��h�T��)
	cocos2d::DrawNode *_draw[LINE_LENGTH - 1];	//�����u�q
	b2Body *_b2Linebody;						//�������z�@�ɽu�q
	bool _bDrawing;								//�O�_�e�Ϥ�
	int _iFree, _iInUsed;						//�����ҥ��I��
	int _istartPt;								//�����}�Y�I�����ޭ�
	void DrawLine(cocos2d::Point prePt, cocos2d::Point Pt, cocos2d::Color4F color, int num);

	//�e�ϯS��
	int _iflareNum;
	float _fflareCount;
	cocos2d::Point _tp;	//�ƹ��I
	cocos2d::Sprite *_flare[SPARKS_NUMBER];	//�x�s���l
	b2Body *_b2flare[SPARKS_NUMBER];

	// for Start and End
	float _fStartCount;				//���Ʊq�Ť���y�p��
	float _fWinCount;				//�������d�p��
	b2Body *_waterBody;				//���y���zBody
	cocos2d::Point _startPt;		//�_�I
	cocos2d::Sprite *_startWater;
	cocos2d::Sprite *_Bush_dry;
	cocos2d::Sprite *_Bush_live;
	cocos2d::Sprite *_sparks[SPARKS_NUMBER];	//�x�s���l

	// for cutting
	int _icutNum;					//�Ũ�����
	bool _bCutting;					//�O�_�e�Ϥ�
	b2RopeJoint *_MainRopeJoint;	//�D�s��÷
	cocos2d::Node *_cutNode;		//�Ũ��`�I
	cocos2d::Point _cutPt[2];		//�O���Ũ����u����I�y��
	cocos2d::Point _preCutPt;		//�s����ܥ�
	cocos2d::Sprite *_cut1;			//UI
	cocos2d::Sprite *_cut2;			//UI
	cocos2d::DrawNode *_drawCut[CUTTING_NUM];	//�����u�q
	void DrawCuttingLine(cocos2d::Point firstPt, cocos2d::Point Pt, cocos2d::Color4F color, int num);
	int _iCutLineNum;						//�Ũ��u�q��
	cocos2d::Point _tp_start;				//start touchLoc(�e�u��)
	DrawNode *_CutDrawLine;					//�Ũ��u
	DrawNode *_destroyCutDrawLine;			//�x�s�ò����e�@���u

	// for gravity point
	cocos2d::Sprite *_gravityUp;
	cocos2d::Sprite *_gravityUp_on;
	cocos2d::Sprite *_gravityDown;
	cocos2d::Sprite *_gravityDown_on;

	// for Box2D
	b2World* _b2World;
	cocos2d::Size _visibleSize;
	b2Body *_bottomBody; // ������ edgeShape

	//�禡
	void createStaticBoundary(); //���
	void setupStatic();			//�@���R�A����
	void setStartEndpoint();	//���y & ���I
	
	void setupSeesaw1();			//���ߪO1�]�w
	void setupSeesaw2();			//���ߪO2�]�w
	void setupGravityPt();			//���O�ഫ�I
	void setupRopeJoint();			//÷�l

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
	CREATE_FUNC(GravityLevel);
};

#endif