#ifndef __CARLEVEL_H__
#define __CARLEVEL_H__

//#define BOX2D_DEBUG 1

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "SimpleAudioEngine.h"
#include "cocostudio/CocoStudio.h"
#include "Box2D/Box2D.h"
#include "Common/C3SButton.h"
#include "Common/CSwitchButton.h"
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
#define LINE_LENGTH 100 //�i�e�u������
#define CUTTING_NUM 1	//�i�Ũ�����

//�I����ť���O
class  CContactListenerLevel5 : public b2ContactListener
{
public:
	cocos2d::Node *_blackNode;		//�ϵe�`�I
	cocos2d::Node *_redNode;
	//cocos2d::Node *_blueNode;

	//sensor
	bool _bTriSensor;				//�O�_�I��
	bool _bRectSensor;
	bool _bJeepSensor;
	cocos2d::Sprite *_triSensor;	//triangle
	cocos2d::Sprite *_rectSensor;	//rectangle
	cocos2d::Sprite *_jeepSensor;	//jeep
	cocos2d::Sprite *_jeepSprite;	//���l�Ϥ�

	//cutting
	bool _bDeleteJoint;				//�O�_�n�R��Joint
	cocos2d::Node *_cutNode;		//�Ũ��`�I
	b2Body *_ropeBody;
	b2Body *_lineBody;

	//start & end
	bool _bWin;						//�q�L���d
	bool _bCreateSpark;				//���ͤ���
	//int  _NumOfSparks;				//�Q�o���l��
	b2Vec2 _createLoc;				//�Q�o�I
	cocos2d::Sprite *_StartSprite;	//water
	cocos2d::Sprite *_EndSprite;	//bush

	 CContactListenerLevel5();
	//�I���}�l
	virtual void BeginContact(b2Contact* contact);
	//�I������
	virtual void EndContact(b2Contact* contact);
};

class CarLevel : public cocos2d::Layer
{
public:
	~CarLevel();
	// there's no 'id' in cpp, so we recommend returning the class instance pointer
	static cocos2d::Scene* createScene();
	Node *_csbRoot;

	//�I���ƥ�
	 CContactListenerLevel5 _contactListener;

	//ui ����
	bool _bBtnPressed;	//�O�_���b���s�W
	C3SButton *_retrybtn;
	C3SButton *_homebtn;
	CSwitchButton *_shapebtn;	//�Ϊ��������s
	cocos2d::ui::LoadingBar *_penBlackBar;	//�µ������q��
	cocos2d::ui::LoadingBar *_penRedBar;	//���������q��
	cocos2d::ui::LoadingBar *_penBlueBar;	//�ŵ������q��
	C3SButton *_blackpenbtn;	//�µ� ���s
	C3SButton *_redpenbtn;		//���� ���s
	C3SButton *_bluepenbtn;		//�ŵ� ���s
	C3SButton *_cutbtn;			//�ŤM ���s

	//drawing
	bool _bDrawing;									//�O�_�e�Ϥ�
	//black
	cocos2d::Node *_blackNode;						//�ϵe�`�I
	cocos2d::Point _pt[LINE_LENGTH];				//�O���I�y��
	cocos2d::DrawNode *_draw[LINE_LENGTH - 1];		//�����u�q
	b2Body *_b2Linebody;							//�������z�@�ɽu�q
	int _iFree, _iInUsed;							//�����ҥ��I��
	int _istartPt;									//�����}�Y�I�����ޭ�
	//red
	cocos2d::Node *_redNode;						//�ϵe�`�I
	cocos2d::Point _pt_red[LINE_LENGTH];			//�O���I�y��
	cocos2d::DrawNode *_draw_red[LINE_LENGTH - 1];	//�����u�q
	b2Body *_b2Linebody_red;						//�������z�@�ɽu�q
	int _iFree_red, _iInUsed_red;					//�����ҥ��I��
	int _istartPt_red;								//�����}�Y�I�����ޭ�
	//blue
	cocos2d::Node *_blueNode;						//�ϵe�`�I
	cocos2d::Point _pt_blue[LINE_LENGTH];			//�O���I�y��
	cocos2d::DrawNode *_draw_blue[LINE_LENGTH - 1];	//�����u�q
	b2Body *_b2Linebody_blue;						//�������z�@�ɽu�q
	int _iFree_blue, _iInUsed_blue;					//�����ҥ��I��
	int _istartPt_blue;								//�����}�Y�I�����ޭ�
	void DrawLine(cocos2d::Point prePt, cocos2d::Point Pt, cocos2d::Color4F color, int num);	//�e�Ϩ禡

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
	//cocos2d::Sprite *_cut2;		//UI
	cocos2d::DrawNode *_drawCut[CUTTING_NUM];	//�����u�q
	void DrawCuttingLine(cocos2d::Point firstPt, cocos2d::Point Pt, cocos2d::Color4F color, int num);
	int _iCutLineNum;						//�Ũ��u�q��
	cocos2d::Point _tp_start;				//start touchLoc(�e�u��)
	DrawNode *_CutDrawLine;					//�Ũ��u
	DrawNode *_destroyCutDrawLine;			//�x�s�ò����e�@���u

	// for sensor
	cocos2d::Sprite *_triSensor;	//�T���P���I
	cocos2d::Sprite *_rectSensor;	//�x�ηP���I
	cocos2d::Sprite *_jeepSensor;	//���l�P���I
	cocos2d::Sprite *_blackWood0, *_blackWood1, *_blackWood2;	//�¦���Y
	cocos2d::Sprite *_redWood0, *_redWood1, *_redWood2;			//������Y
	cocos2d::Sprite *_blueWood;									//�Ŧ���Y
	b2Body *_b2BlackBody1, *_b2BlackBody2;	//���z���Y
	b2Body *_b2RedBody1, *_b2RedBody2;
	b2Body *_b2BlueBody;

	// for jeep
	cocos2d::Sprite *_jeepSprite;
	b2Body *_CarBody;
	b2Body *_wheel1Body;
	b2Body *_wheel2Body;

	// for Box2D
	b2World* _b2World;
	cocos2d::Size _visibleSize;
	b2Body *_bottomBody; // ������ edgeShape

	//�禡
	void createStaticBoundary(); //���
	void setupStatic();			//�@���R�A����
	void setStartEndpoint();	//���y & ���I
	
	void setupRopeJoint();			//÷�l
	void setupJeep();				//���l ///////������
	void setupSensorPt();			//�Ϊ��P���I
	void showBlackWood();			//��ܶ¤��Y
	void destroyBlackWood();		//�����¤��Y
	void showRedWood();				//��ܬ����Y
	void destroyRedWood();			//���������Y
	void showBlueWood();			//����Ť��Y
	void destroyBlueWood();			//�����Ť��Y

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
	CREATE_FUNC(CarLevel);
};

#endif