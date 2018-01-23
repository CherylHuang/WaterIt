#include "CButton.h"

USING_NS_CC;

CButton* CButton::create()
{
	CButton *btnobj = new (std::nothrow) CButton();
	if (btnobj)
	{
		btnobj->autorelease();
		return btnobj;
	}
	CC_SAFE_DELETE(btnobj);
	return nullptr;
}


CButton::CButton()
{
	_normalPic = nullptr;
	_touchedPic = nullptr;
}

void CButton::setButtonInfo(const char *normalImg, const char *touchedImg, const cocos2d::Point locPt)
{
	_normalPic = (Sprite *)Sprite::createWithSpriteFrameName(normalImg);
	_touchedPic = (Sprite *)Sprite::createWithSpriteFrameName(touchedImg);
	_BtnLoc = locPt;
	_normalPic->setPosition(_BtnLoc); // 設定位置
	_touchedPic->setPosition(_BtnLoc); // 設定位置
	_touchedPic->setVisible(false);
	_touchedPic->setScale(1.25f);
	this->addChild(_normalPic,1);  // 加入目前的 Layer 中 1: Z 軸的層次，越大代表在越上層
	this->addChild(_touchedPic,1); // 加入目前的 Layer 中 1: Z 軸的層次，越大代表在越上層

	// 取得大小
	_BtnSize = _normalPic->getContentSize();
	// 設定判斷區域
	_BtnRect.size = _BtnSize;
	_BtnRect.origin.x = _BtnLoc.x - _BtnSize.width*0.5f;
	_BtnRect.origin.y = _BtnLoc.y - _BtnSize.height*0.5f;
	_bTouched = false;
	_fScale = 1.0f;
	_bVisible = _bEnabled = true;
}


bool CButton::touchesBegin(cocos2d::Point inPos)
{
	if( _BtnRect.containsPoint(inPos) && _bVisible && _bEnabled)
	{
		_bTouched = true;
		_normalPic->setVisible(false);
		_touchedPic->setVisible(true);
		return(true); // 有按在上面
	}
	return(false);
}

bool CButton::touchesMoved(cocos2d::Point inPos)
{
	if( _bTouched ) { // 只有被按住的時候才處理
		if( !_BtnRect.containsPoint(inPos) ) { // 手指頭位置離開按鈕
			_bTouched = false;
			_normalPic->setVisible(true);
			_touchedPic->setVisible(false);
			return(false);
		}
		else return(true);
	}
	return(false); // 事後再移到按鈕上將被忽略
}

bool CButton::touchesEnded(cocos2d::Point inPos)
{
	if( _bTouched )
	{
		_bTouched = false;
		_normalPic->setVisible(true);
		_touchedPic->setVisible(false);
		if( _BtnRect.containsPoint(inPos) ) return(true);  // 手指頭位置按鈕時，還在該按鈕上
	}
	return false;
}

void CButton::setVisible(bool bVisible)
{
	_bVisible = bVisible;
	if (!_bVisible) _normalPic->setVisible(false);
	else _normalPic->setVisible(true);
}


void CButton::setEnabled(bool bEnable)
{
	_bEnabled = bEnable;
	if (_bEnabled) _normalPic->setColor(Color3B(255, 255, 255)); 
	else _normalPic->setColor(Color3B(50, 50, 50));
}

void CButton::setScale(float scale)
{

	_fScale = scale;
	_normalPic->setScale(_fScale);
	_touchedPic->setScale(_fScale*1.25f);
}