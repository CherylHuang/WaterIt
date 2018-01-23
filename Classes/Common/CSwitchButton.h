#ifndef __CSWITCHBUTTON_H__
#define __CSWITCHBUTTON_H__

#include "cocos2d.h"

USING_NS_CC;

class CSwitchButton :public Node
{ 
private:
	cocos2d::Sprite *_SwitchOffPic;
	cocos2d::Sprite *_SwitchTDPic;
	cocos2d::Sprite *_SwitchOnPic;

	cocos2d::Size  _BtnSize;
	cocos2d::Point _BtnLoc;
	cocos2d::Rect  _BtnRect;

	float _fScale;	// 圖片縮放的大小
	bool _bTouched;	// 是否被按下
	bool _bSwitchOn;// 按鈕是否再開啟狀態
	bool _bVisible; // 是否顯示

public:
	CSwitchButton();
	static CSwitchButton* create(); // 建立物件的實體
	void setButtonInfo(const char *offImg, const char *onImg, const char *tdImg, const cocos2d::Point locPt);
	bool touchesBegan(cocos2d::Point inPos);
	bool touchesMoved(cocos2d::Point inPos); 
	bool touchesEnded(cocos2d::Point inPos); 
	void setVisible(bool bVis);
	void setScale(float scale);
	bool getStatus();
	void setColor(const Color3B& color);

};


#endif