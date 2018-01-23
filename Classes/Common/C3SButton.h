#pragma once
#ifndef __C3SBUTTON_H__
#define __C3SBUTTON_H__

#include "cocos2d.h"
#include "SimpleAudioEngine.h"

#define NORMAL_BTN 1 //一般點選紐
#define SELECT_BTN 2 //選擇紐

USING_NS_CC;

class C3SButton : public cocos2d::Node
{
private:
	cocos2d::Sprite *_btnEnablePic;
	cocos2d::Sprite *_btnPressedPic;
	cocos2d::Sprite *_btnDisablePic;

	cocos2d::Size  _BtnSize;
	cocos2d::Point _BtnLoc;
	cocos2d::Rect  _BtnRect;

	int _iType; //按鈕種類
	float _fScale;	// 圖片縮放的大小
	bool _bSelected; //type:2 選項鈕
	bool _bPressed;	// 是否被按下
	bool _bEnable;  // 按鈕是否可按
	bool _bVisible; // 是否顯示

public:
	C3SButton();
	~C3SButton();
	static C3SButton* create(); // 建立物件實體
	void setButtonInfo(const int type ,const char *enablePic, const char *pressedPic, const char *disablePic, const cocos2d::Point locPt);

	//觸碰
	bool onTouchBegan(cocos2d::Point inPos); //觸碰開始事件
	bool onTouchMoved(cocos2d::Point inPos); //觸碰移動事件
	bool onTouchEnded(cocos2d::Point inPos); //觸碰結束事件 
	
	void setVisible(bool bVis);
	void setEnable(bool bEnable);
	void setScale(float scale);
	void setPosition(const cocos2d::Point &pos);
	bool getPressed();
	bool getEnable();

	//collision
	bool playerCollision(cocos2d::Point inPos);
	void setPlayerRec(cocos2d::Point inPos);
};

#endif // __C3SBUTTON_H__