#include "CLight.h"

USING_NS_CC;

CLight* CLight::create()
{
	CLight *lightobj = new (std::nothrow) CLight();
	if (lightobj)
	{
		lightobj->autorelease();
		return lightobj;
	}
	CC_SAFE_DELETE(lightobj);
	return nullptr;
}


CLight::CLight()
{
	_lightOnPic = nullptr;
	_lightOffPic = nullptr;
}

void CLight::setLightInfo(const char *lightOnImg, const char *lightOffImg, const cocos2d::Point locPt)
{
	_lightOnPic = (Sprite *)Sprite::createWithSpriteFrameName(lightOnImg);
	_lightOffPic = (Sprite *)Sprite::createWithSpriteFrameName(lightOffImg);
	_lightLoc = locPt;
	_lightOnPic->setPosition(_lightLoc); // 設定位置
	_lightOffPic->setPosition(_lightLoc); // 設定位置
	_bLightOn = false;
	_lightOnPic->setVisible(false);  // 預設為關閉狀態

	this->addChild(_lightOnPic, 1);  // 加入目前的 Layer 中 1: Z 軸的層次，越大代表在越上層
	this->addChild(_lightOffPic,1); // 加入目前的 Layer 中 1: Z 軸的層次，越大代表在越上層

	_fscale = 1.0f;
	_bVisible = true;
}

void  CLight::setLightStatus(bool bLightOn)
{
	_bLightOn = bLightOn;
	if (_bLightOn) {
		_lightOnPic->setVisible(true);
		_lightOffPic->setVisible(false);
	}
	else {
		_lightOnPic->setVisible(false);
		_lightOffPic->setVisible(true);
	}
}

void CLight::setScale(float fscale)
{
	_fscale = fscale;
	_lightOnPic->setScale(_fscale);
	_lightOffPic->setScale(_fscale);
}

void CLight::setVisible(bool bVisible)
{
	_bVisible = bVisible;
	if (!_bVisible) {
		_lightOnPic->setVisible(false);
		_lightOffPic->setVisible(false);
	}
	else {
		if( _bLightOn ) _lightOnPic->setVisible(true);
		else _lightOffPic->setVisible(true);
	}
}

bool CLight::getLightStatus()
{
	return(_bLightOn);
}