#ifndef __CLIGHT_H__
#define __CLIGHT_H__

// 單純的開關設定，不提供 touch 事件
// 可以設定預設是開還是關，就是顯示相對應的圖示

#include "cocos2d.h"

USING_NS_CC;

class CLight :public Node
{ 
private:
	cocos2d::Sprite *_lightOnPic;
	cocos2d::Sprite *_lightOffPic;

	bool  _bVisible; // 是否顯示
	bool  _bLightOn;
	float _fscale;
	Point _lightLoc;

public:
	CLight();
	static CLight* create(); // 建立物件的實體
	void setLightInfo(const char *lightOnImg, const char *lightOffImg, const cocos2d::Point locPt);
	void setVisible(bool bVis);
	void setScale(float fscale);
	void setLightStatus(bool bLightOn);
	bool getLightStatus();
};


#endif