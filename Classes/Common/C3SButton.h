#pragma once
#ifndef __C3SBUTTON_H__
#define __C3SBUTTON_H__

#include "cocos2d.h"
#include "SimpleAudioEngine.h"

#define NORMAL_BTN 1 //�@���I���
#define SELECT_BTN 2 //��ܯ�

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

	int _iType; //���s����
	float _fScale;	// �Ϥ��Y�񪺤j�p
	bool _bSelected; //type:2 �ﶵ�s
	bool _bPressed;	// �O�_�Q���U
	bool _bEnable;  // ���s�O�_�i��
	bool _bVisible; // �O�_���

public:
	C3SButton();
	~C3SButton();
	static C3SButton* create(); // �إߪ������
	void setButtonInfo(const int type ,const char *enablePic, const char *pressedPic, const char *disablePic, const cocos2d::Point locPt);

	//Ĳ�I
	bool onTouchBegan(cocos2d::Point inPos); //Ĳ�I�}�l�ƥ�
	bool onTouchMoved(cocos2d::Point inPos); //Ĳ�I���ʨƥ�
	bool onTouchEnded(cocos2d::Point inPos); //Ĳ�I�����ƥ� 
	
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