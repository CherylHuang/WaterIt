#pragma once
#ifndef __MAINMENU_SCENE_H__
#define __MAINMENU_SCENE_H__

#define WATER_PARTICLE 1

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "SimpleAudioEngine.h"
#include "cocostudio/CocoStudio.h"
#include "Common/C3SButton.h"
#include "Box2D/Box2D.h"

//�I����ť���O
class CContactListenerMainMenu : public b2ContactListener
{
public:
	//start & end
	bool _bWin;						//�q�L���d
	bool _bCreateSpark;				//���ͤ���
	int  _NumOfSparks;				//�Q�o���l��
	b2Vec2 _createLoc;				//�Q�o�I
	cocos2d::Sprite *_StartSprite;	//water
	cocos2d::Sprite *_EndSprite;	//bush

	CContactListenerMainMenu();
	//�I���}�l
	virtual void BeginContact(b2Contact* contact);
	//�I������
	virtual void EndContact(b2Contact* contact);
};

class MainMenu : public cocos2d::Layer
{
private:
	bool settingVisible;
	cocos2d::Sprite *_black_bg; //�¹��B��
	cocos2d::Sprite *_settingWindow;
	C3SButton *_startbtn;
	C3SButton *_settingbtn;
	C3SButton *_exitbtn;
	C3SButton *_okaybtn;
	
	//level btn
	C3SButton *_easybtn;
	C3SButton *_normalbtn;
	C3SButton *_hardbtn;
	C3SButton *_finalbtn;

public:
	~MainMenu();
	static cocos2d::Scene* createScene();
	virtual bool init();
	void doStep(float dt);
	static int level; //����������d
	Node *_rootNode;

	// for Box2D
	b2World* _b2World;
	cocos2d::Size _visibleSize;

	//�I���ƥ�
	CContactListenerMainMenu _contactListener;

	// for Start and End
	float _fStartCount;				//���Ʊq�Ť���y�p��
	b2Body *_waterBody;				//���y���zBody
	cocos2d::Point _startPt;		//�_�I
	cocos2d::Sprite *_startWater;
	cocos2d::Sprite *_Bush_dry;
	cocos2d::Sprite *_Bush_live;
	float _fcount;					//���s�}�l�p��

	//�禡
	void setupStatic();			//�@���R�A����
	void setStartEndpoint();	//���y & ���I

	//Ĳ�I
	cocos2d::EventListenerTouchOneByOne *_listener1;
	bool onTouchBegan(cocos2d::Touch *pTouch, cocos2d::Event *pEvent); //Ĳ�I�}�l�ƥ�
	void onTouchMoved(cocos2d::Touch *pTouch, cocos2d::Event *pEvent); //Ĳ�I���ʨƥ�
	void onTouchEnded(cocos2d::Touch *pTouch, cocos2d::Event *pEvent); //Ĳ�I�����ƥ� 

	CREATE_FUNC(MainMenu);
};
#endif