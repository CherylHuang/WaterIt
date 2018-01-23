#ifndef __GEAR_H__
#define __GEAR_H__

//#define BOX2D_DEBUG 1

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "SimpleAudioEngine.h"
#include "cocostudio/CocoStudio.h"
#include "Box2D/Box2D.h"
#include "Common/C3SButton.h"
#include "GravityLevel.h" //�U�@��
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
#define CUTTING_NUM 4	//�i�Ũ�����

//�I����ť���O
class  CContactListenerLevel4 : public b2ContactListener
{
public:
	//cutting
	bool _bDeleteJoint;				//�O�_�n�R��Joint
	bool _bDeleteRope1;				//����rope1
	bool _bDeleteRope2;				//����rope2
	bool _bDeleteRope3;				//����rope3
	bool _bDeleteRope4;				//����rope4
	cocos2d::Node *_cutNode;		//�Ũ��`�I
	b2Body *_ropeBody;
	b2Body *_lineBody;

	//transition door
	bool _bMove;					//�O�_�ǰe
	cocos2d::Sprite *_DoorIn;		//door in

	//start & end
	bool _bWin;						//�q�L���d
	bool _bCreateSpark;				//���ͤ���
	//int  _NumOfSparks;				//�Q�o���l��
	b2Vec2 _createLoc;				//�Q�o�I
	cocos2d::Sprite *_StartSprite;	//water
	cocos2d::Sprite *_EndSprite;	//bush

	 CContactListenerLevel4();
	//�I���}�l
	virtual void BeginContact(b2Contact* contact);
	//�I������
	virtual void EndContact(b2Contact* contact);
};

class GearLevel : public cocos2d::Layer
{
public:
	~GearLevel();
	// there's no 'id' in cpp, so we recommend returning the class instance pointer
	static cocos2d::Scene* createScene();
	Node *_csbRoot;

	//�I���ƥ�
	 CContactListenerLevel4 _contactListener;

	//ui
	bool _bBtnPressed;	//�O�_���b���s�W
	C3SButton *_retrybtn;
	C3SButton *_homebtn;
	C3SButton *_cutbtn;

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
	cocos2d::Node *_cutNode;		//�Ũ��`�I
	cocos2d::Point _cutPt[2];		//�O���Ũ����u����I�y��
	cocos2d::Point _preCutPt;		//�s����ܥ�
	cocos2d::DrawNode *_drawCut[CUTTING_NUM];	//�����u�q
	void DrawCuttingLine(cocos2d::Point firstPt, cocos2d::Point Pt, cocos2d::Color4F color, int num);
	b2RopeJoint *_MainRopeJoint;	//�D�s��÷1
	b2RopeJoint *_MainRope2Joint;	//�D�s��÷2
	b2RopeJoint *_MainRope3Joint;	//�D�s��÷3
	b2RopeJoint *_MainRope4Joint;	//�D�s��÷4
	b2Body *_cactusBody;			//����
	b2Body *_cactus2Body;			//����
	int _iCutLineNum;						//�Ũ��u�q��
	cocos2d::Point _tp;						//touchLoc(�e�u��)
	cocos2d::Point _tp_start;				//start touchLoc(�e�u��)
	DrawNode *_CutDrawLine;					//�Ũ��u
	DrawNode *_destroyCutDrawLine;			//�x�s�ò����e�@���u

	//for pully joint + �ƽ�÷
	b2Body *_Board;					//�D��O���zBody
	b2Body *_leftBoard;				//����O���zBody
	b2Body *_rightBoard;			//�k��O���zBody
	cocos2d::Sprite *_gear1;		//�Ĥ@�ƽ�
	cocos2d::Sprite *_cactus1;		//����1
	cocos2d::Sprite *_gear2;		//�ĤG�ƽ�
	cocos2d::Sprite *_cactus2;		//����2
	cocos2d::Sprite *_board;		//��O
	cocos2d::Sprite *_board_left;	//����O
	cocos2d::Sprite *_board_right;	//�k��O
	int _iLineNum;					//�e�u�ƶq
	DrawNode *_pulleyline1;							//�ƽ��u
	DrawNode *_destroyLine1;						//�x�s�ò����e�@���u
	DrawNode *_pulleyline2, *_destroyLine2;			//�ƽ��u
	DrawNode *_pulleyline3, *_destroyLine3;			//�ƽ��u
	DrawNode *_pulleyline4, *_destroyLine4;			//�ƽ��u

	// for gears
	cocos2d::Sprite *_fakeTurn1;	//�]������
	cocos2d::Sprite *_fakeTurn2;
	b2Body *_b2fakeTurn1, *_b2fakeTurn2;	//����
	b2Body *_groundbody, *_groundbody2;		//�s���ƽ� �����R�A����

	//for transition door
	cocos2d::Sprite *_DoorIn;		//transDoor_in
	cocos2d::Sprite *_DoorOut;		//transDoor_out

	// for Box2D
	b2World* _b2World;
	cocos2d::Size _visibleSize;
	b2Body *_bottomBody; // ������ edgeShape

	//�禡
	void createStaticBoundary(); //���
	void setupStatic();			//�@���R�A����
	void setStartEndpoint();	//���y & ���I

	void setupWoodBoard();			//��O�]�w
	void setupRopeJoint();			//÷�l1
	void setupRope2Joint();			//÷�l2
	void setupRope3Joint();			//÷�l3
	void setupRope4Joint();			//÷�l4
	void setupPulleyJoint();		//�ƽ�
	void setupMoveDoor();			//�ǰe��
	void setupGearStatic();			//�ƽ������R�A����]�w
	void setupGear();				//����


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
	CREATE_FUNC(GearLevel);
};

#endif