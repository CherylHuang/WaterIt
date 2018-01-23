#include "CSwitchButton.h"

USING_NS_CC;

CSwitchButton* CSwitchButton::create()
{
	CSwitchButton *btnobj = new (std::nothrow) CSwitchButton();
	if (btnobj)
	{
		btnobj->autorelease();
		return btnobj;
	}
	CC_SAFE_DELETE(btnobj);
	return nullptr;
}


CSwitchButton::CSwitchButton()
{
	_SwitchOffPic = nullptr;
	_SwitchTDPic = nullptr;
	_SwitchOnPic = nullptr;
}

void CSwitchButton::setButtonInfo(const char *offImg, const char *onImg, const char *tdImg, const cocos2d::Point locPt)
{
	_SwitchOffPic = (Sprite *)Sprite::createWithSpriteFrameName(offImg);
	_SwitchTDPic  = (Sprite *)Sprite::createWithSpriteFrameName(tdImg);
	_SwitchOnPic = (Sprite *)Sprite::createWithSpriteFrameName(onImg);

	_BtnLoc = locPt;
	_SwitchOffPic->setPosition(_BtnLoc); // 設定位置
	_SwitchTDPic->setPosition(_BtnLoc); // 設定位置
	_SwitchOnPic->setPosition(_BtnLoc); // 設定位置
	_SwitchOffPic->setVisible(true);
	_SwitchTDPic->setVisible(false);
	_SwitchOnPic->setVisible(false);
	//_SwitchTDPic->setScale(1.25f);

	this->addChild(_SwitchOffPic,1);
	this->addChild(_SwitchTDPic, 1);
	this->addChild(_SwitchOnPic, 1);

	// 取得大小
	_BtnSize = _SwitchOffPic->getContentSize();
	// 設定判斷區域
	_BtnRect.size = _BtnSize;
	_BtnRect.origin.x = _BtnLoc.x - _BtnSize.width*0.5f;
	_BtnRect.origin.y = _BtnLoc.y - _BtnSize.height*0.5f;
	_bTouched = false;
	_fScale = 1.0f;
	_bVisible = true;
	_bSwitchOn = false;
}


bool CSwitchButton::touchesBegan(cocos2d::Point inPos)
{
	if( _BtnRect.containsPoint(inPos) && _bVisible )
	{
		_bTouched = true;
		_SwitchOnPic->setVisible(false);
		_SwitchTDPic->setVisible(true);
		_SwitchOffPic->setVisible(false);
		return(true); // 有按在上面
	}
	return(false);
}

bool CSwitchButton::touchesMoved(cocos2d::Point inPos)
{
	if( _bTouched ) { // 只有被按住的時候才處理
		if( !_BtnRect.containsPoint(inPos) ) { // 手指頭位置離開按鈕
			_bTouched = false;
			if (_bSwitchOn) { // 顯示亮起來的圖示
				_SwitchOnPic->setVisible(true);
				_SwitchTDPic->setVisible(false);
			}
			else {
				_SwitchOffPic->setVisible(true);
				_SwitchTDPic->setVisible(false);
			}
			return(false);
		}
		else return(true);
	}
	return(false); // 事後再移到按鈕上將被忽略
}

bool CSwitchButton::touchesEnded(cocos2d::Point inPos)
{
	if( _bTouched ) {
		if (_bSwitchOn) { // 狀態切換，按鈕設定為關閉
			_SwitchOnPic->setVisible(false);
			_SwitchTDPic->setVisible(false);
			_SwitchOffPic->setVisible(true);
		}
		else { // 狀態切換，按鈕設定為開啟
			_SwitchOnPic->setVisible(true);
			_SwitchTDPic->setVisible(false);
			_SwitchOffPic->setVisible(false);
		}
		_bTouched = false;
		_bSwitchOn = !_bSwitchOn;
		if( _BtnRect.containsPoint(inPos) ) return(true);  // 手指頭位置按鈕時，還在該按鈕上
	}
	return false;
}

void CSwitchButton::setVisible(bool bVisible)
{
	_bVisible = bVisible;
	if (!_bVisible) {
		_SwitchOffPic->setVisible(false);
	}
	else {
		_SwitchOffPic->setVisible(true);
	}
}

void CSwitchButton::setScale(float scale)
{
	_fScale = scale;
	_SwitchOnPic->setScale(_fScale);
	_SwitchTDPic->setScale(_fScale);
	_SwitchOffPic->setScale(_fScale);
}

bool CSwitchButton::getStatus()
{
	return(_bSwitchOn); // 傳回目前按鈕的狀態為開或是關
}

void CSwitchButton::setColor(const Color3B& color)
{
	_SwitchOnPic->setColor(color);
	_SwitchTDPic->setColor(color);
	_SwitchOffPic->setColor(color);
}