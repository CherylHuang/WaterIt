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

//碰撞聆聽類別
class CContactListenerMainMenu : public b2ContactListener
{
public:
	//start & end
	bool _bWin;						//通過關卡
	bool _bCreateSpark;				//產生水花
	int  _NumOfSparks;				//噴發分子數
	b2Vec2 _createLoc;				//噴發點
	cocos2d::Sprite *_StartSprite;	//water
	cocos2d::Sprite *_EndSprite;	//bush

	CContactListenerMainMenu();
	//碰撞開始
	virtual void BeginContact(b2Contact* contact);
	//碰撞結束
	virtual void EndContact(b2Contact* contact);
};

class MainMenu : public cocos2d::Layer
{
private:
	bool settingVisible;
	cocos2d::Sprite *_black_bg; //黑幕遮屏
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
	static int level; //紀錄選擇關卡
	Node *_rootNode;

	// for Box2D
	b2World* _b2World;
	cocos2d::Size _visibleSize;

	//碰撞事件
	CContactListenerMainMenu _contactListener;

	// for Start and End
	float _fStartCount;				//重複從空中放球計時
	b2Body *_waterBody;				//水球物理Body
	cocos2d::Point _startPt;		//起點
	cocos2d::Sprite *_startWater;
	cocos2d::Sprite *_Bush_dry;
	cocos2d::Sprite *_Bush_live;
	float _fcount;					//重新開始計時

	//函式
	void setupStatic();			//一般靜態物件
	void setStartEndpoint();	//水球 & 終點

	//觸碰
	cocos2d::EventListenerTouchOneByOne *_listener1;
	bool onTouchBegan(cocos2d::Touch *pTouch, cocos2d::Event *pEvent); //觸碰開始事件
	void onTouchMoved(cocos2d::Touch *pTouch, cocos2d::Event *pEvent); //觸碰移動事件
	void onTouchEnded(cocos2d::Touch *pTouch, cocos2d::Event *pEvent); //觸碰結束事件 

	CREATE_FUNC(MainMenu);
};
#endif