#ifndef __CBUTTON_H__
#define __CBUTTON_H__

#include "cocos2d.h"

USING_NS_CC;

class CButton :public Node
{ 
private:
	cocos2d::Sprite *_normalPic;
	cocos2d::Sprite *_touchedPic;

	cocos2d::Size  _BtnSize;
	cocos2d::Point _BtnLoc;
	cocos2d::Rect  _BtnRect;

	float _fScale;	// �Ϥ��Y�񪺤j�p
	bool _bTouched; // �O�_�Q���U
	bool _bEnabled; // �O�_���@��
	bool _bVisible; // �O�_���

public:
	CButton();
	static CButton* create(); // �إߪ��󪺹���
	void setButtonInfo(const char *normalImg, const char *touchedImg, const cocos2d::Point locPt);
	bool touchesBegin(cocos2d::Point inPos);
	bool touchesMoved(cocos2d::Point inPos); 
	bool touchesEnded(cocos2d::Point inPos); 
	void setVisible(bool bVis);
	void setEnabled(bool bEnable);
	void setScale(float scale);

};


#endif