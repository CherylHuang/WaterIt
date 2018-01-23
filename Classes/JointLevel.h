#ifndef __JOINTLEVEL_H__
#define __JOINTLEVEL_H__

//#define BOX2D_DEBUG 1

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "SimpleAudioEngine.h"
#include "cocostudio/CocoStudio.h"
#include "Box2D/Box2D.h"
#include "Common/C3SButton.h"
#include "GearLevel.h" //�U�@��
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

//�I����ť���O
class CContactListenerLevel1 : public b2ContactListener
{
public:
	// dot joint
	cocos2d::Sprite *_targetSprite;
	cocos2d::Node *_newNode;		//�ϵe�`�I
	bool _bRevoluteJoint;			//�����I���A����s��

	//start & end
	bool _bWin;						//�q�L���d
	bool _bCreateSpark;				//���ͤ���
	//int  _NumOfSparks;				//�Q�o���l��
	b2Vec2 _createLoc;				//�Q�o�I
	cocos2d::Sprite *_StartSprite;	//water
	cocos2d::Sprite *_EndSprite;	//bush

	CContactListenerLevel1();
	//�I���}�l
	virtual void BeginContact(b2Contact* contact);
	//�I������
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

	//�I���ƥ�
	CContactListenerLevel1 _contactListener;
	b2Body *_dotBody;
	bool _bJointCount; //����p��
	float _fcount; //����P�_�ɶ�

	//ui
	bool _bBtnPressed;	//�O�_���b���s�W
	cocos2d::ui::LoadingBar *_penBlackBar;	//�����q��
	C3SButton *_retrybtn;
	C3SButton *_homebtn;

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

	// for Box2D
	b2World* _b2World;
	cocos2d::Size _visibleSize;

	// for MouseJoint
	b2Body *_bottomBody; // ������ edgeShape
	b2MouseJoint* _MouseJoint;
	bool _bTouchOn;

	// Box2D Examples
	void createStaticBoundary();

	void setupMouseJoint();

	//�禡
	void setupStatic();			//�@���R�A����
	void setJointDot();			//�s���I
	void setStartEndpoint();	//���y & ���I

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
	CREATE_FUNC(JointLevel);
};

#endif