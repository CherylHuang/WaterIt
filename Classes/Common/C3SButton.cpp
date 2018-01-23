#include "C3SButton.h"

USING_NS_CC;

using namespace CocosDenshion;

C3SButton* C3SButton::create()
{
	C3SButton *btnobj = new (std::nothrow) C3SButton();
	if (btnobj)
	{
		btnobj->autorelease();
		return btnobj;
	}
	CC_SAFE_DELETE(btnobj);
	return nullptr;
}

C3SButton::C3SButton()
{
	_btnEnablePic = nullptr;
	_btnPressedPic = nullptr;
	_btnDisablePic = nullptr;
	SimpleAudioEngine::getInstance()->preloadEffect("gain point.mp3");	// 預先載入音效檔
}

C3SButton::~C3SButton()
{
	SimpleAudioEngine::getInstance()->unloadEffect("gain point.mp3");  // 釋放音效檔
}

void C3SButton::setButtonInfo(const int type, const char *enablePic, const char *pressedPic, const char *disablePic, const cocos2d::Point locPt)
{
	_btnEnablePic = (Sprite *)Sprite::createWithSpriteFrameName(enablePic);
	_btnPressedPic = (Sprite *)Sprite::createWithSpriteFrameName(pressedPic);
	_btnDisablePic = (Sprite *)Sprite::createWithSpriteFrameName(disablePic);

	_BtnLoc = locPt;
	_btnEnablePic->setPosition(_BtnLoc); //set btn position
	_btnPressedPic->setPosition(_BtnLoc);
	_btnDisablePic->setPosition(_BtnLoc);
	_btnEnablePic->setVisible(true);
	_btnPressedPic->setVisible(false);
	_btnDisablePic->setVisible(false);
	//_btnPressedPic->setScale(1.2f);

	this->addChild(_btnEnablePic, 3);
	this->addChild(_btnPressedPic, 3);
	this->addChild(_btnDisablePic, 3);

	_BtnSize = _btnEnablePic->getContentSize();
	// 設定判斷區域
	_BtnRect.size = _BtnSize;
	_BtnRect.origin.x = _BtnLoc.x - _BtnSize.width*0.5f;
	_BtnRect.origin.y = _BtnLoc.y - _BtnSize.height*0.5f;
	_bPressed = false;
	_fScale = 1.0f;
	_bVisible = true;
	_bEnable = true;
	_bSelected = false;
	_iType = type; //設定按鈕類型
}

bool C3SButton::onTouchBegan(cocos2d::Point inPos)//觸碰開始事件
{
	if (_BtnRect.containsPoint(inPos) && _bVisible && _bEnable)
	{
		_bPressed = true;
		_btnEnablePic->setVisible(false);
		_btnPressedPic->setVisible(true);
		unsigned int eid = SimpleAudioEngine::getInstance()->playEffect("gain point.mp3", false);  // 播放音效檔
		return(true); // 有按在上面
	}
	return(false);
}

bool C3SButton::onTouchMoved(cocos2d::Point inPos) //觸碰移動事件
{
	if (_bPressed) { // 只有被按住的時候才處理
		if (!_BtnRect.containsPoint(inPos)) { // 手指頭位置離開按鈕
			_bPressed = false;
			_btnEnablePic->setVisible(true);
			_btnPressedPic->setVisible(false);
			return(false);
		}
		else {
			_btnPressedPic->setVisible(true);
			_btnEnablePic->setVisible(false);
			return(true);
		}
	}
	return(false); // 事後再移到按鈕上將被忽略
}

bool C3SButton::onTouchEnded(cocos2d::Point inPos) //觸碰結束事件 
{
	if (_bPressed && _BtnRect.containsPoint(inPos)) {
		_bPressed = false; //沒有點選
		switch (_iType) {
		case NORMAL_BTN:	//一般點選紐
			_btnEnablePic->setVisible(true);
			_btnPressedPic->setVisible(false);
			return true;
			break;
		case SELECT_BTN:	//選擇紐
			_bEnable = false;
			_bSelected = true; //被選擇
			_btnEnablePic->setVisible(false);
			_btnPressedPic->setVisible(false);
			_btnDisablePic->setVisible(true);
			return true;
			break;
		default:
			break;
		}
	}
	return false;
}

void C3SButton::setVisible(bool bVisible)
{
	_bVisible = bVisible;
	if (!_bVisible) { //隱藏
		if (_bEnable) _btnEnablePic->setVisible(false);
		else _btnDisablePic->setVisible(false);
	}
	else { //顯示
		if (_bEnable) _btnEnablePic->setVisible(true);
		else _btnDisablePic->setVisible(true);
	}
}

void C3SButton::setEnable(bool bEnable)
{
	_bEnable = bEnable;
	if (!_bEnable) { //Disable
		_btnEnablePic->setVisible(false);
		_btnPressedPic->setVisible(false);
		_btnDisablePic->setVisible(true);
	}
	else {
		_btnEnablePic->setVisible(true);
		_btnPressedPic->setVisible(false);
		_btnDisablePic->setVisible(false);
	}
}

void C3SButton::setScale(float scale)
{
	_fScale = scale;
	_btnEnablePic->setScale(_fScale);
	_btnPressedPic->setScale(_fScale);
	_btnDisablePic->setScale(_fScale);

	// 縮放判斷區域
	_BtnRect.size = _BtnSize*scale;
	_BtnRect.origin.x = _BtnLoc.x - _BtnSize.width*scale*0.5f;
	_BtnRect.origin.y = _BtnLoc.y - _BtnSize.height*scale*0.5f;
}

void C3SButton::setPosition(const Point &pos)
{
	_btnEnablePic->setPosition(pos);
	_btnPressedPic->setPosition(pos);
	_btnDisablePic->setPosition(pos);
}

bool C3SButton::getPressed()
{
	return(_bPressed); // 傳回按鈕是否有被按下
}

bool C3SButton::getEnable()
{
	return(_bEnable); // 傳回按鈕是否可點選
}

bool C3SButton::playerCollision(cocos2d::Point inPos)
{
	if (_BtnRect.containsPoint(inPos))
	{
		return(true); // 有按在上面
	}
	return(false);
}

void C3SButton::setPlayerRec(cocos2d::Point inPos)
{
	_BtnLoc = inPos;
	_BtnSize = _btnEnablePic->getContentSize();
	// 設定判斷區域
	_BtnRect.size = _BtnSize;
	_BtnRect.origin.x = _BtnLoc.x - _BtnSize.width*0.5f;
	_BtnRect.origin.y = _BtnLoc.y - _BtnSize.height*0.5f;
}