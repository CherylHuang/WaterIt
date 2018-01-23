#ifndef __CLIGHT_H__
#define __CLIGHT_H__

// ��ª��}���]�w�A������ touch �ƥ�
// �i�H�]�w�w�]�O�}�٬O���A�N�O��ܬ۹������ϥ�

#include "cocos2d.h"

USING_NS_CC;

class CLight :public Node
{ 
private:
	cocos2d::Sprite *_lightOnPic;
	cocos2d::Sprite *_lightOffPic;

	bool  _bVisible; // �O�_���
	bool  _bLightOn;
	float _fscale;
	Point _lightLoc;

public:
	CLight();
	static CLight* create(); // �إߪ��󪺹���
	void setLightInfo(const char *lightOnImg, const char *lightOffImg, const cocos2d::Point locPt);
	void setVisible(bool bVis);
	void setScale(float fscale);
	void setLightStatus(bool bLightOn);
	bool getLightStatus();
};


#endif