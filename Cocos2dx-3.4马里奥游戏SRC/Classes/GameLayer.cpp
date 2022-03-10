#include "GameLayer.h"
#include "GameMap.h"
#include "Hero.h"
#include "AnimationManager.h"
#include "Item.h"
#include "MainMenu.h"
#include "Global.h"
#include "SimpleAudioEngine.h"
#include "SelectMenu.h"

float CCGameLayer::mapMaxH;

CCGameLayer::CCGameLayer():
mainMap(NULL),
hero(NULL)
{
	mapMaxH = 0.0f;
	mapBeginPos = ccp(0.0f, 96.0f);

	birthPoint.x = 180.0f;
	birthPoint.y = 32.0f;

	isKeyDownA = false;
	isKeyDownD = false;
	isSky = false;
	heroFireable = true;

	moveOffset = 0.0f;
	moveDelta = 0.0f;
	JumpOffset = 0.0f;

	ccMoveDelta = 0.05f;
	ccMoveOffset = 2.0f;
	ccJumpOffset = 0.3f;

	currentPos.x = 0.0f;
	currentPos.y = 0.0f;

	Size winsize = Director::getInstance()->getWinSize();
	heroAnchor = ccp(winsize.width/2 - 80, winsize.height/2);

	backKeyPos = ccp(84, 48);
	leftKeyPos = ccp(40, 48);
	rightKeyPos = ccp(128, 48);
	jumpKeyPos = ccp(432, 35);
	fireKeyPos = ccp(353, 35);
	MSetKeyPos = ccp(260, 33);

	FireBallPos = ccp(winsize.width - 70, winsize.height - 20);
	arrowPos = ccp(winsize.width - 30, winsize.height - 20);

	mainLayer = Layer::create();
	mainLayer->retain();

	isPass = false;

	isLeftKeyDown = false;
	isRightKeyDown = false;
	isJumpKeyDown = false;
	isFireKeyDown = false;
}

CCGameLayer::~CCGameLayer()
{
	//Director::getInstance()->getOpenGLView()->SetWin32KeyLayer(NULL);

	// 这里面的几个指针全部都是通过create生成的，已经autorelease，交给引擎管理，不用手动删除
	//CC_SAFE_DELETE(hero);
	//CC_SAFE_DELETE(mainMap);
	//CC_SAFE_DELETE(mainLayer);

	this->unscheduleUpdate();
	this->unscheduleAllSelectors();
}

bool CCGameLayer::init()
{
	CocosDenshion::SimpleAudioEngine::sharedEngine()->playBackgroundMusic("OnLand.ogg", true);

	this->initHeroAndMap();

	this->initcontrolUI();

	this->initBulletUI();

	this->initRect();

	this->initSetMenu();

	//Director::getInstance()->getOpenGLView()->SetWin32KeyLayer(this);
	//this->registerWithTouchDispatcher();

	//后面检查
	//Director::getInstance()->getTouchDispatcher()->addStandardDelegate(this, kCCMenuHandlerPriority);
	auto touchMore = EventListenerTouchAllAtOnce::create();
	touchMore->onTouchesBegan = CC_CALLBACK_2(CCGameLayer::onTouchesBegan, this);
	touchMore->onTouchesMoved = CC_CALLBACK_2(CCGameLayer::onTouchesMoved, this);
	touchMore->onTouchesEnded = CC_CALLBACK_2(CCGameLayer::onTouchesEnded, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(touchMore, this);

	//键盘事件
	auto keyboardListen = EventListenerKeyboard::create();
	keyboardListen->onKeyPressed = CC_CALLBACK_2(CCGameLayer::onKeyPressed, this);
	keyboardListen->onKeyReleased = CC_CALLBACK_2(CCGameLayer::onKeyReleased, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(keyboardListen, this);

	this->setTouchEnabled(true);
	this->scheduleUpdate();

	return true;
}

void CCGameLayer::onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event)
{
	switch (keyCode)
	{
	case cocos2d::EventKeyboard::KeyCode::KEY_A:
		isLeftKeyDown = true;
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_D:
		isRightKeyDown = true;
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_J:
		isJumpKeyDown = true;
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_K:
		isFireKeyDown = true;
		break;
	default:
		break;
	}
}

void CCGameLayer::onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event)
{
	switch (keyCode)
	{
	case cocos2d::EventKeyboard::KeyCode::KEY_A:
		isLeftKeyDown = false;
		isKeyDownA = false;
		moveOffset = 0.0f;
		moveDelta = 0.0f;
		hero->setHeroState(eNormalLeft);
		pBackKeyImage->setDisplayFrame(pBackKeyNormal);
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_D:
		isRightKeyDown = false;
		isKeyDownD = false;
		moveOffset = 0.0f;
		moveDelta = 0.0f;
		hero->setHeroState(eNormalRight);
		pBackKeyImage->setDisplayFrame(pBackKeyNormal);
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_J:
		isJumpKeyDown = false;
		pJumpImage->setDisplayFrame(pAB_Normal);
		break;
	case cocos2d::EventKeyboard::KeyCode::KEY_K:
		isFireKeyDown = false;
		pFireImage->setDisplayFrame(pAB_Normal);
		break;
	default:
		break;
	}
}

void CCGameLayer::initSetMenu()
{
	pColor = LayerColor::create(ccc4(0, 0, 0, 100));
	this->addChild(pColor, this->getChildrenCount());
	pColor->setVisible(false);

	pSetMenu = Sprite::create("Set_Menu.png");
	pSetMenu->setAnchorPoint(ccp(129.0/248, 71.0/132));
	Point origin = Director::getInstance()->getVisibleOrigin();
	Size visibleSize = Director::getInstance()->getVisibleSize();
	pSetMenu->setPosition(ccp(origin.x+visibleSize.width/2, origin.y+visibleSize.height/2));
	this->addChild(pSetMenu, this->getChildrenCount()+1);
	pSetMenu->setVisible(false);

	pResume = MenuItemImage::create("resume_n.png", "resume_s.png",
		this, menu_selector(CCGameLayer::menuResume));
	pResume->setPosition(ccp(origin.x+visibleSize.width/2, origin.y+visibleSize.height/2+40));
	pResume->setVisible(false);
	pResume->setEnabled(false);

	pReStart = MenuItemImage::create("restart_n.png", "restart_s.png",
		this, menu_selector(CCGameLayer::menuReStart));
	pReStart->setPosition(ccp(origin.x+visibleSize.width/2, origin.y+visibleSize.height/2));
	pReStart->setVisible(false);
	pReStart->setEnabled(false);

	pSelectMenu = MenuItemImage::create("select_n.png", "select_s.png",
		this, menu_selector(CCGameLayer::menuSelectMenu));
	pSelectMenu->setPosition(ccp(origin.x+visibleSize.width/2, origin.y+visibleSize.height/2-40));
	pSelectMenu->setVisible(false);
	pSelectMenu->setEnabled(false);

	pMenu->addChild(pResume, pMenu->getChildrenCount());
	pMenu->addChild(pReStart, pMenu->getChildrenCount());
	pMenu->addChild(pSelectMenu, pMenu->getChildrenCount());
}

void CCGameLayer::menuMSet( Ref *pSender )
{
	this->pauseGameLayer();
}

void CCGameLayer::menuResume( Ref *pSender )
{
	this->resumeGameLayer();
}

void CCGameLayer::menuReStart( Ref *pSender )
{
	Scene *pScene = Scene::create();
	pScene->addChild(CCGameLayer::create());
	Director::getInstance()->replaceScene(pScene);
}

void CCGameLayer::menuSelectMenu( Ref *pSender )
{
	Scene *pScene = Scene::create();
	pScene->addChild(CCSelectMenu::create());
	Director::getInstance()->replaceScene(pScene);
}

void CCGameLayer::initRect()
{
	leftKeyRect = CCRectMake(leftKeyPos.x - pLeftKey->getContentSize().width/2,
		leftKeyPos.y - pLeftKey->getContentSize().height/2,
		pLeftKey->getContentSize().width,
		pLeftKey->getContentSize().height);

	rightKeyRect = CCRectMake(rightKeyPos.x - pRightKey->getContentSize().width/2,
		rightKeyPos.y - pRightKey->getContentSize().height/2,
		pRightKey->getContentSize().width,
		pRightKey->getContentSize().height);

	jumpKeyRect = CCRectMake(jumpKeyPos.x - pJump->getContentSize().width/2,
		jumpKeyPos.y - pJump->getContentSize().height/2,
		pJump->getContentSize().width,
		pJump->getContentSize().height);

	fireKeyRect = CCRectMake(fireKeyPos.x - pFire->getContentSize().width/2,
		fireKeyPos.y - pFire->getContentSize().height/2,
		pFire->getContentSize().width,
		pFire->getContentSize().height);
}

void CCGameLayer::initHeroAndMap()
{
	char temp[20] = { 0 };
	sprintf(temp, "MarioMap%d.tmx", Global::getGlobalInstance()->getCurrentLevel());
	mainMap = CCGameMap::create(temp);

	mapSize = CCSizeMake(mainMap->getMapSize().width * mainMap->getTileSize().width, 
		mainMap->getMapSize().height * mainMap->getTileSize().height);
	mainMap->setPosition(ccp(0, 0));
	mainLayer->addChild(mainMap);
	hero = Hero::create();
	hero->setBodyType(Global::getGlobalInstance()->currentHeroType);
	hero->setAnchorPoint(ccp(0.5f, 0.0f));
	hero->setPosition(mainMap->getMarioBirthPos());
	heroSize = hero->getCurrentSize();
	mainLayer->addChild(hero);
	mainLayer->setPosition(mapBeginPos);
	this->addChild(mainLayer);
}

void CCGameLayer::loadMap()
{

}

void CCGameLayer::initcontrolUI()
{
	controlUI = Sprite::create("controlUI.png");
	controlUI->setAnchorPoint(ccp(0, 0));
	this->addChild(controlUI);

	Size winSize = Director::getInstance()->getWinSize();

	pGameOverBack = Sprite::create("gameover.png");
	pGameOverBack->setPosition(ccp(winSize.width/2, winSize.height/2 + 50));
	this->addChild(pGameOverBack);
	pGameOverBack->setVisible(false);

	pPassFailure = Sprite::create("PassFailure.png");
	pPassFailure->setPosition(ccp(winSize.width/2, winSize.height/2 + 50));
	this->addChild(pPassFailure);
	pPassFailure->setVisible(false);

	pPassSuccess = Sprite::create("PassSuccess.png");
	pPassSuccess->setPosition(ccp(winSize.width/2, winSize.height/2 + 50));
	this->addChild(pPassSuccess);
	pPassSuccess->setVisible(false);


	winSize = Director::getInstance()->getWinSize();

	pBackKeyImage = Sprite::create("backKeyImage.png");
	pBackKeyImage->setPosition(backKeyPos);
	this->addChild(pBackKeyImage);

	pBackKeyNormal = SpriteFrame::create("backKeyImage.png", CCRectMake(0, 0, 72, 72));
	pBackKeyNormal->retain();
	pBackKeyLeft = SpriteFrame::create("backKeyLeft.png", CCRectMake(0, 0, 72, 72));
	pBackKeyLeft->retain();
	pBackKeyRight = SpriteFrame::create("backKeyRight.png", CCRectMake(0, 0, 72, 72));
	pBackKeyRight->retain();

	pAB_Normal = SpriteFrame::create("AB_normal.png", CCRectMake(0, 0, 72, 50));
	pAB_Normal->retain();
	pAB_Selected = SpriteFrame::create("AB_select.png", CCRectMake(0, 0, 72, 50));
	pAB_Selected->retain();

	pJumpImage = Sprite::createWithSpriteFrame(pAB_Normal);
	pJumpImage->setPosition(jumpKeyPos);
	this->addChild(pJumpImage, 3);

	pFireImage = Sprite::createWithSpriteFrame(pAB_Normal);
	pFireImage->setPosition(fireKeyPos);
	this->addChild(pFireImage, 3);

	pLeftKey = MenuItemImage::create("leftright.png", "leftright.png",
		this, menu_selector(CCGameLayer::menuCallBackLeftKey));
	pRightKey = MenuItemImage::create("leftright.png", "leftright.png",
		this, menu_selector(CCGameLayer::menuCallBackRightKey));
	pJump = MenuItemImage::create("AB_normal.png", "AB_select.png",
		this, menu_selector(CCGameLayer::menuCallBackJumpKey));
	pFire = MenuItemImage::create("AB_normal.png", "AB_select.png",
		this, menu_selector(CCGameLayer::menuCallBackFireKey));
	pMSet = MenuItemImage::create("M_n.png", "M_s.png",
		this, menu_selector(CCGameLayer::menuMSet));

	pBackToMenu = MenuItemImage::create("backToMenu.png", "backToMenu.png",
		this, menu_selector(CCGameLayer::menuCallBackBackToMenu));
	pBackToMenu->setEnabled(false);
	pBackToMenu->setVisible(false);

	pNext = MenuItemImage::create("nextlevel_normal.png", "nextlevel_select.png",
		this, menu_selector(CCGameLayer::menuNext));
	pNext->setPosition(ccp(winSize.width/2, winSize.height/2 + 40));
	pNext->setVisible(false);
	pNext->setEnabled(false);

	pRetry = MenuItemImage::create("retry_normal.png", "retry_select.png",
		this, menu_selector(CCGameLayer::menuRetry));
	pRetry->setPosition(ccp(winSize.width/2, winSize.height/2));
	pRetry->setVisible(false);
	pRetry->setEnabled(false);

	pLeftKey->setPosition(leftKeyPos);
	pRightKey->setPosition(rightKeyPos);
	pJump->setPosition(jumpKeyPos);
	pFire->setPosition(fireKeyPos);
	pMSet->setPosition(MSetKeyPos);
	pBackToMenu->setPosition(ccp(winSize.width/2, winSize.height/2 + 20));

	pMenu = Menu::create(pNext, pRetry, pMSet, NULL);
	pMenu->setAnchorPoint(Vec2::ZERO);
	pMenu->setPosition(ccp(0, 0));

	this->addChild(pMenu, 100);
}

void CCGameLayer::initBulletUI()
{
	pBulletBorderArrow = Sprite::create("bulletBorder.png");
	pBulletBorderArrow->setPosition(arrowPos);
	//pBulletBorderArrow->setScale(2.0f);

	pBulletBorderFireBall = Sprite::create("bulletBorder.png");
	pBulletBorderFireBall->setPosition(FireBallPos);

	Sprite *pArrow = Sprite::create("arrowBullet.png");
	
	pMenuArrow = MenuItemSprite::create(pArrow, pArrow,pArrow,
		this, menu_selector(CCGameLayer::menuCallBackArrow));
	pMenuArrow->setPosition(arrowPos);
	pMenu->addChild(pMenuArrow);

	Sprite *pFireBall = Sprite::create("fireBall.png");
	//pFireBall->setScale(2.0f);
	pMenuFireBall = MenuItemSprite::create(pFireBall, pFireBall,pFireBall,
		this, menu_selector(CCGameLayer::menuCallBackFireBall));
	pMenuFireBall->setPosition(FireBallPos);
	pMenu->addChild(pMenuFireBall);
}

void CCGameLayer::menuCallBackLeftKey(Ref *pSender)
{
	isKeyDownA = false;
	moveOffset = 0.0f;
	moveDelta = 0.0f;
	hero->setHeroState(eNormalLeft);
	pBackKeyImage->setDisplayFrame(pBackKeyNormal);
}

void CCGameLayer::menuCallBackRightKey(Ref *pSender)
{
	isKeyDownD = false;
	moveOffset = 0.0f;
	moveDelta = 0.0f;
	hero->setHeroState(eNormalRight);
	pBackKeyImage->setDisplayFrame(pBackKeyNormal);
}

void CCGameLayer::menuCallBackJumpKey(Ref *pSender)
{
	
}

void CCGameLayer::menuCallBackFireKey(Ref *pSender)
{

}

void CCGameLayer::menuCallBackArrow(Ref *pSender)
{
	if (hero->isBulletable())
	{
		Global::getGlobalInstance()->setCurrentBulletType(eBullet_arrow);
		hero->setBulletType(eBullet_arrow);
	}
}

void CCGameLayer::menuCallBackFireBall(Ref *pSender)
{
	if (hero->isBulletable())
	{
		Global::getGlobalInstance()->setCurrentBulletType(eBullet_common);
		hero->setBulletType(eBullet_common);
	}
}

void CCGameLayer::menuCallBackBackToMenu(Ref *pSender)
{
	this->toMainMenu();
}

void CCGameLayer::onTouchesBegan(const std::vector<Touch*>& touches, Event *unused_event)
{
	//for (CCSetIterator it = pTouches->begin(); it != pTouches->end(); ++it)
	//{
	//	Touch *pTouch = (Touch*)(*it);
	//	Point touchPos = pTouch->getLocation();
	//	if (leftKeyRect.containsPoint(touchPos))
	//	{
	//		isLeftKeyDown = true;
	//	}
	//	if (rightKeyRect.containsPoint(touchPos))
	//	{
	//		isRightKeyDown = true;
	//	}
	//	if (jumpKeyRect.containsPoint(touchPos))
	//	{
	//		isJumpKeyDown = true;
	//	}
	//	if (fireKeyRect.containsPoint(touchPos))
	//	{
	//		isFireKeyDown = true;
	//	}
	//}
}

void CCGameLayer::onTouchesEnded(const std::vector<Touch*>& touches, Event *unused_event)
{
	//for (CCSetIterator it = pTouches->begin(); it != pTouches->end(); ++it)
	//{
	//	Touch *pTouch = (Touch*)(*it);
	//	Point touchPos = pTouch->getLocation();
	//	if (leftKeyRect.containsPoint(touchPos))
	//	{
	//		isLeftKeyDown = false;
	//		isKeyDownA = false;
	//		moveOffset = 0.0f;
	//		moveDelta = 0.0f;
	//		hero->setHeroState(eNormalLeft);
	//		pBackKeyImage->setDisplayFrame(pBackKeyNormal);
	//	}
	//	if (rightKeyRect.containsPoint(touchPos))
	//	{
	//		isRightKeyDown = false;
	//		isKeyDownD = false;
	//		moveOffset = 0.0f;
	//		moveDelta = 0.0f;
	//		hero->setHeroState(eNormalRight);
	//		pBackKeyImage->setDisplayFrame(pBackKeyNormal);
	//	}
	//	if (jumpKeyRect.containsPoint(touchPos))
	//	{
	//		isJumpKeyDown = false;
	//		pJumpImage->setDisplayFrame(pAB_Normal);
	//	}
	//	if (fireKeyRect.containsPoint(touchPos))
	//	{
	//		isFireKeyDown = false;
	//		pFireImage->setDisplayFrame(pAB_Normal);
	//	}
	//}
}

void CCGameLayer::onTouchesMoved(const std::vector<Touch*>& touches, Event *unused_event)
{
	//if (isLeftKeyDown)
	//{
	//	bool flag = false;
	//	for (CCSetIterator it = pTouches->begin(); it != pTouches->end(); ++it)
	//	{
	//		Touch *pTouch = (Touch*)(*it);
	//		Point touchPos = pTouch->getLocation();

	//		if (leftKeyRect.containsPoint(touchPos))
	//		{
	//			flag = true;
	//		}
	//	}
	//	if (flag == false)
	//	{
	//		isLeftKeyDown = false;
	//		isKeyDownA = false;
	//		moveOffset = 0.0f;
	//		moveDelta = 0.0f;
	//		hero->setHeroState(eNormalLeft);
	//		pBackKeyImage->setDisplayFrame(pBackKeyNormal);
	//	}
	//}

	//if (isRightKeyDown)
	//{
	//	bool flag = false;
	//	for (CCSetIterator it = pTouches->begin(); it != pTouches->end(); ++it)
	//	{
	//		Touch *pTouch = (Touch*)(*it);
	//		Point touchPos = pTouch->getLocation();

	//		if (rightKeyRect.containsPoint(touchPos))
	//		{
	//			flag = true;
	//		}
	//	}
	//	if (flag == false)
	//	{
	//		isRightKeyDown = false;
	//		isKeyDownD = false;
	//		moveOffset = 0.0f;
	//		moveDelta = 0.0f;
	//		hero->setHeroState(eNormalRight);
	//		pBackKeyImage->setDisplayFrame(pBackKeyNormal);
	//	}
	//}

	//if (isJumpKeyDown)
	//{
	//	bool flag = false;
	//	for (CCSetIterator it = pTouches->begin(); it != pTouches->end(); ++it)
	//	{
	//		Touch *pTouch = (Touch*)(*it);
	//		Point touchPos = pTouch->getLocation();

	//		if (jumpKeyRect.containsPoint(touchPos))
	//		{
	//			flag = true;
	//		}
	//	}
	//	if (flag == false)
	//	{
	//		isJumpKeyDown = false;
	//		pJumpImage->setDisplayFrame(pAB_Normal);
	//	}
	//}

	//if (isFireKeyDown)
	//{
	//	bool flag = false;
	//	for (CCSetIterator it = pTouches->begin(); it != pTouches->end(); ++it)
	//	{
	//		Touch *pTouch = (Touch*)(*it);
	//		Point touchPos = pTouch->getLocation();

	//		if (fireKeyRect.containsPoint(touchPos))
	//		{
	//			flag = true;
	//		}
	//	}
	//	if (flag == false)
	//	{
	//		isFireKeyDown = false;
	//		pFireImage->setDisplayFrame(pAB_Normal);
	//	}
	//}
}

void CCGameLayer::onTouchesCancelled(const std::vector<Touch*>&touches, Event *unused_event)
{

}

//void CCGameLayer::registerWithTouchDispatcher()
//{
//	Layer::registerWithTouchDispatcher();
//	Director::getInstance()->getTouchDispatcher()->addStandardDelegate(this, 0);
//}

void CCGameLayer::menuCloseCallBack(Ref *pSender)
{
	this->toMainMenu();
}

//void CCGameLayer::processWin32KeyPress(UINT message, WPARAM wParam, LPARAM lParam)
//{
//	if (hero->isHeroDied())
//	{
//		return ;
//	}
//	switch (message)
//	{
//	case WM_KEYDOWN:
//		if (wParam == 65)
//		{
//			isKeyDownA = true;
//			moveOffset = -ccMoveOffset;
//			moveDelta = -ccMoveDelta;
//			hero->setHeroState(eLeft);
//		}
//		else if (wParam == 68)
//		{
//			isKeyDownD = true;
//			moveOffset = ccMoveOffset;
//			moveDelta = ccMoveDelta;
//			hero->setHeroState(eRight);
//		}
//		else if (wParam == 32)
//		{
//			hero->gadgetable = false;
//			if (!isSky)
//			{
//				CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("Jump.wma");
//				JumpOffset = 6.0f;
//				isSky = true;
//				hero->isFlying = true;
//			}
//		}
//		else if (wParam == 73)
//		{
//			// 发射子弹
//			if (hero->isBulletable())
//			{
//				if (heroFireable)
//				{
//					mainMap->createNewBullet();
//					hero->fireAction();
//					heroFireable = false;
//					DelayTime *pDelay = DelayTime::create(0.5f);
//					this->runAction(Sequence::create(pDelay, 
//						CallFunc::create(this, callfunc_selector(CCGameLayer::reSetHeroFireable)), NULL));
//
//				}
//			}
//		}
//		break;
//	case WM_KEYUP:
//		if (wParam == 65)
//		{
//			isKeyDownA = false;
//			moveOffset = 0.0f;
//			moveDelta = 0.0f;
//			hero->setHeroState(eNormalLeft);
//		}
//		else if (wParam == 68)
//		{
//			isKeyDownD = false;
//			moveOffset = 0.0f;
//			moveDelta = 0.0f;
//			hero->setHeroState(eNormalRight);
//		}
//		break;
//	}
//}




void CCGameLayer::updateControl()
{
	if (!hero->isHeroDied())
	{
		if (/*pLeftKey->isSelected()*/ isLeftKeyDown )
		{
			isKeyDownA = true;
			moveOffset = -ccMoveOffset;
			moveDelta = -ccMoveDelta;
			hero->setHeroState(eLeft);
			pBackKeyImage->setDisplayFrame(pBackKeyLeft);
		}
		else if (/*pRightKey->isSelected()*/ isRightKeyDown )
		{
			isKeyDownD = true;
			moveOffset = ccMoveOffset;
			moveDelta = ccMoveDelta;
			hero->setHeroState(eRight);
			pBackKeyImage->setDisplayFrame(pBackKeyRight);
		}
		if (/*pJump->isSelected()*/ isJumpKeyDown )
		{
			hero->gadgetable = false;
			if (!isSky)
			{
				CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("Jump.ogg");
				JumpOffset = 6.0f;
				isSky = true;
				hero->isFlying = true;				
			}
			pJumpImage->setDisplayFrame(pAB_Selected);
		}
		if (/*pFire->isSelected()*/ isFireKeyDown )
		{
			// 发射子弹
			if (hero->isBulletable())
			{
				if (heroFireable)
				{
					CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("RengHuoQiu.ogg");
					mainMap->createNewBullet();
					hero->fireAction();
					heroFireable = false;
					DelayTime *pDelay = DelayTime::create(0.5f);
					this->runAction(Sequence::create(pDelay, 
						CallFunc::create(this, callfunc_selector(CCGameLayer::reSetHeroFireable)), NULL));

				}
			}
			pFireImage->setDisplayFrame(pAB_Selected);
		}
	}

}

void CCGameLayer::toMainMenu()
{
	Scene *pScene = Scene::create();
	pScene->addChild(MainMenu::create());
	Director::getInstance()->replaceScene(pScene);
}

void CCGameLayer::stopForPassFailure()
{
	if (Global::getGlobalInstance()->getCurrentLifeNum() == 0)
	{
		CocosDenshion::SimpleAudioEngine::sharedEngine()->stopBackgroundMusic(true);
		CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("GameOver.ogg");
		//Global::getGlobalInstance()->setWhyToMainMenu(efor_GameOver);
		pGameOverBack->setVisible(true);
	}
	else
	{
		//Global::getGlobalInstance()->setWhyToMainMenu(efor_PassFailure);
		pPassFailure->setVisible(true);
	}

	mainMap->stopUpdateForHeroDie();
	this->unscheduleUpdate();
	this->reSetKeyNo();

	DelayTime *pDelay = DelayTime::create(3);
	this->runAction(Sequence::create(pDelay,
		CallFunc::create(this, callfunc_selector(CCGameLayer::reShowPassFailure)), NULL));
}

void CCGameLayer::reSetKeyNo()
{
	//pFire->setEnabled(false);
	//pJump->setEnabled(false);
	pMenuFireBall->setEnabled(false);
	pMenuArrow->setEnabled(false);
}

void CCGameLayer::reShowPassFailure()
{	
	if (Global::getGlobalInstance()->getCurrentLifeNum() == 0)
	{
		this->toMainMenu();
	}
	else
	{
		pRetry->setVisible(true);
		pRetry->setEnabled(true);
	}
	//pBackToMenu->setEnabled(true);
	//pBackToMenu->setVisible(true);
}

void CCGameLayer::stopForPassSuccess()
{
	int level = Global::getGlobalInstance()->getCurrentLevel();
	CCString *str = CCString::createWithFormat("Level%d", level + 1);
	UserDefault::sharedUserDefault()->setStringForKey(str->getCString(), "yes");

	
	mainMap->stopUpdateForHeroDie();
	this->unscheduleUpdate();

	DelayTime *pDelay = DelayTime::create(3);
	this->runAction(Sequence::create(pDelay,
		CallFunc::create(this, callfunc_selector(CCGameLayer::reShowPassSuccess)), NULL));

}

void CCGameLayer::reShowPassSuccess()
{
	pPassSuccess->setVisible(true);
	//pBackToMenu->setEnabled(true);
	//pBackToMenu->setVisible(true);

	
	if (Global::getGlobalInstance()->getCurrentLevel() == Global::getGlobalInstance()->getTotalLevels())
	{
		this->showPassAll();
	}else
	{
		pNext->setVisible(true);
		pNext->setEnabled(true);
	}


}

void CCGameLayer::showHeroJump()
{
	hero->reSetForSuccess();
	CCJumpTo *pJump = CCJumpTo::create(1.0f, mainMap->finalPoint, 32, 3);
	hero->runAction(pJump);
}

void CCGameLayer::update(float dt)
{
	// 是否死亡判断
	if (hero->isHeroDied())
	{
		this->stopForPassFailure();
		return ;
	}
	// 是否通关判断
	if (isPass)
	{
		this->stopForPassSuccess();
		return ;
	}

	this->updateControl();

	currentPos = hero->getPosition();
	heroSize = hero->getContentSize();

	if (hero->isGadgetable())
	{
		currentPos.x += moveOffset + mainMap->heroInGadget->getMoveOffset();
		currentPos.y += JumpOffset + mainMap->heroInGadget->getJumpOffset();
	}
	else
	{
		currentPos.x += moveOffset;
		currentPos.y += JumpOffset;
	}
	if (isSky)
	{
		switch (hero->face)
		{
		case eLeft:
			hero->setHeroState(eJumpLeft);
			break;
		case eRight:
			hero->setHeroState(eJumpRight);
			break;
		default:
			break;
		}
	}

	hero->setPosition(currentPos);
	setSceneScrollPosition();
	collistionV();
	collistionH();
}

void CCGameLayer::updateContent(float dt)
{
	currentPos = hero->getPosition();
	currentPos.x = currentPos.x + moveOffset + moveDelta;
	hero->setPosition(currentPos);
	setSceneScrollPosition();
	collistionH();
}

void CCGameLayer::updateDelta(float dt)
{
	//if (isKeyDownA)
	//{
	//	moveDelta -= ccMoveDelta;
	//}
	//else if (isKeyDownD)
	//{
	//	moveDelta += ccMoveDelta;
	//}
}

// 水平方向碰撞检测
// 悬浮在半空中大约一秒钟的bug应该是水平碰撞的问题，已解决
void CCGameLayer::collistionH()
{
	Point currentPos = hero->getPosition();
	// 做判断不让主角移除屏幕的左侧
	if ( (currentPos.x - heroSize.width/2 - mapMaxH) <= 0 )
	{
		Point pp = ccp(mapMaxH + heroSize.width/2, currentPos.y);
		hero->setPosition(pp);
		return ;
	}

	bool flag = false;
	// 右侧判断
	Point rightCollision = ccp(currentPos.x + heroSize.width/2, currentPos.y /* + heroSize.height/2*/);
	Point rightTileCoord = mainMap->positionToTileCoord(rightCollision);
	if (mainMap->isMarioEatMushroom(rightTileCoord))
	{
		hero->changeForGotMushroom();
	}
	if (mainMap->isMarioEatAddLifeMushroom(rightTileCoord))
	{
		hero->changeForGotAddLifeMushroom();
	}
	Point rightPos = mainMap->tilecoordToPosition(rightTileCoord);
	rightPos = ccp(rightPos.x - heroSize.width/2, currentPos.y);

	TileType tileType = mainMap->tileTypeforPos(rightTileCoord);
	switch (tileType)
	{
	case eTile_Block:
	case eTile_Pipe:
	case eTile_Land:
		hero->setPosition(rightPos);
		flag = true;
		break;
	case eTile_Flagpole:
		{
			isPass = true;
			mainMap->showFlagMove();
			this->showHeroJump();
			return ;
			break;
		}
	case eTile_Coin:
		CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("EatCoin.ogg");
		mainMap->getCoinLayer()->removeTileAt(rightTileCoord);
		break;
	default:
		break;
	}

	// 主角的左侧碰撞检测点
	Point leftCollision = ccp(currentPos.x - heroSize.width/2, currentPos.y /* + heroSize.height/2*/);
	Point leftTileCoord = mainMap->positionToTileCoord(leftCollision);
	if (mainMap->isMarioEatMushroom(leftTileCoord))
	{
		hero->changeForGotMushroom();
	}
	if (mainMap->isMarioEatAddLifeMushroom(leftTileCoord))
	{
		hero->changeForGotAddLifeMushroom();
	}
	Point leftPos = mainMap->tilecoordToPosition(leftTileCoord);
	leftPos = ccp(leftPos.x + heroSize.width/2 + mainMap->getTileSize().width, currentPos.y);
	tileType = mainMap->tileTypeforPos(leftTileCoord);
	switch (tileType)
	{
	case eTile_Block:
	case eTile_Pipe:
	case eTile_Land:
		hero->setPosition(leftPos);
		flag = true;
		break;
	case eTile_Coin:
		CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("EatCoin.ogg");
		mainMap->getCoinLayer()->removeTileAt(leftTileCoord);
		break;
	case eTile_Flagpole:
		{
			isPass = true;
			mainMap->showFlagMove();
			this->showHeroJump();
			return ;
			break;
		}
	default:
		break;
	}
}

// 垂直方向碰撞检测
void CCGameLayer::collistionV()
{
	Point currentPos = hero->getPosition();

	// 下面是判断马里奥的脚底是否已经接近地图下边缘的5像素处，以此为die
	if (currentPos.y <= 0)
	{
		hero->setHeroDie(true);
		hero->setPosition(ccp(currentPos.x, 1));
		hero->dieForTrap();  // 执行一段动画，表示因掉入陷阱而死
		CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("DiaoRuXianJingSi.ogg");
		return ;
	}

	// 下面是判断马里奥是否头顶越过游戏画面最上层
	if (currentPos.y > mapSize.height - heroSize.height - 2)
	{
		JumpOffset = 0.0f;
		hero->setPosition(ccp(currentPos.x, mapSize.height - heroSize.height - 2));
		isSky = false;

		return ;
	}

	// 主角头顶的碰撞检测
	for (int heroIdx = 6; heroIdx <= heroSize.width - 6; ++heroIdx)
	{
		Point upCollision = ccp(currentPos.x - heroSize.width/2 + heroIdx, currentPos.y + heroSize.height);
		Point upTileCoord = mainMap->positionToTileCoord(upCollision);
		// 判断是否吃到蘑菇
		if (mainMap->isMarioEatMushroom(upTileCoord))
		{
			hero->changeForGotMushroom();
		}
		if (mainMap->isMarioEatAddLifeMushroom(upTileCoord))
		{
			hero->changeForGotAddLifeMushroom();
		}
		Point upPos = mainMap->tilecoordToPosition(upTileCoord);
		upPos = ccp(currentPos.x, upPos.y - heroSize.height);
		TileType tileType = mainMap->tileTypeforPos(upTileCoord);
		bool flagUp = false;
		switch (tileType)
		{
		case eTile_Block:
		case eTile_Land:
			if (JumpOffset > 0)
			{
				mainMap->breakBlock(upTileCoord, hero->getCurrentBodyType());
				JumpOffset = 0.0f;
				hero->setPosition(upPos);
				flagUp = true;
			}
			break;
		case eTile_Coin:
			CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("EatCoin.ogg");
			mainMap->getCoinLayer()->removeTileAt(upTileCoord);
			break;
		default:
			break;
		}
		if (flagUp)
		{
			JumpOffset -= ccJumpOffset;
			return ;
		}
	}

	float heroLeftSide = currentPos.x - heroSize.width/2;

	for (int heroIdx = 4; heroIdx <= heroSize.width - 4; ++heroIdx)
	{
		Point downCollision = ccp(heroLeftSide + heroIdx, currentPos.y);
		Point downTileCoord = mainMap->positionToTileCoord(downCollision);
		if (mainMap->isMarioEatMushroom(downTileCoord))
		{
			hero->changeForGotMushroom();
		}
		if (mainMap->isMarioEatAddLifeMushroom(downTileCoord))
		{
			hero->changeForGotAddLifeMushroom();
		}
		downTileCoord.y += 1;
		Point downPos = mainMap->tilecoordToPosition(downTileCoord);
		downPos = ccp(currentPos.x , downPos.y + mainMap->getTileSize().height);
		
		TileType tileType = mainMap->tileTypeforPos(downTileCoord);
		bool flagDown = false;
		switch (tileType)
		{
		case eTile_Flagpole:
			{
				isPass = true;
				mainMap->showFlagMove();
				this->showHeroJump();
				return ;
				break;
			}
		case eTile_Coin:
			CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("EatCoin.ogg");
			mainMap->getCoinLayer()->removeTileAt(downTileCoord);
			break;
		//case eTile_Trap:
		//	hero->setPosition(ccp(currentPos.x, 16.0f));
		//	hero->setHeroDie(true);
		//	return ;
		//	break;
		case eTile_Land:
		case eTile_Pipe:
		case eTile_Block:
			{
				if (JumpOffset < 0)
				{
					hero->setGadgetable(false);
					JumpOffset = 0.0f;
					hero->setPosition(downPos);
					isSky = false;
					hero->isFlying = false;
					switch (hero->face)
					{
					case eLeft:
						if (isKeyDownA)
						{
							hero->setHeroState(eLeft);
						}
						else
						{
							hero->setHeroState(eNormalLeft);
						}
						break;
					case eRight:
						if (isKeyDownD)
						{
							hero->setHeroState(eRight);
						}
						else
						{
							hero->setHeroState(eNormalRight);
						}
						break;
					default:
						break;
					}
				}
				flagDown = true;
			}
			break;
		default:
			break;
		}
		if (flagDown) 
		{
			return ;
		}

		// 检测主角是否站在梯子上
		float gadgetLevel = 0.0f;
		if (mainMap->isHeroInGadget(downCollision, gadgetLevel))
		{
			JumpOffset = 0.0f;
			downPos = ccp(currentPos.x, gadgetLevel);
			hero->setPosition(downPos);
			hero->setGadgetable(true);
			isSky = false;
			hero->isFlying = false;
			switch (hero->face)
			{
			case eLeft:
				if (isKeyDownA)
				{
					hero->setHeroState(eLeft);
				}
				else
				{
					hero->setHeroState(eNormalLeft);
				}
				break;
			case eRight:
				if (isKeyDownD)
				{
					hero->setHeroState(eRight);
				}
				else
				{
					hero->setHeroState(eNormalRight);
				}
				break;
			default:
				break;
			}
			return ;
		}
		else
		{
			hero->setGadgetable(false);
		}
	}

	JumpOffset -= ccJumpOffset;
}

void CCGameLayer::setSceneScrollPosition()
{
	Point pos = hero->getPosition();
	Size winsize = Director::getInstance()->getWinSize();

	float x = MAX(pos.x, heroAnchor.x);
	float y = MAX(pos.y, heroAnchor.y);

	x = MIN(x, mapSize.width - winsize.width/2 - 80);
	y = MIN(y, mapSize.height - winsize.height/2);
	Point actualPosition = ccp(x, y);

	Point viewPoint = ccpSub(heroAnchor, actualPosition);
	
	// 使得地图不能倒退回去
	if (fabsf(viewPoint.x) <= mapMaxH)
	{
		return ;
	} 
	else
	{
		mainLayer->setPosition(viewPoint);
		mapMaxH = fabsf(mainLayer->getPosition().x);
	}
}

float CCGameLayer::getMapMaxH()
{
	return mapMaxH;
}

void CCGameLayer::reSetHeroFireable()
{
	heroFireable = true;
}

void CCGameLayer::menuRetry( Ref *pSender )
{
	Scene *pScene = Scene::create();
	pScene->addChild(CCGameLayer::create());
	Director::getInstance()->replaceScene(pScene);
}

void CCGameLayer::menuNext( Ref *pSender )
{
	Global::getGlobalInstance()->currentLevelPlusOne();
	Scene *pScene = Scene::create();
	pScene->addChild(CCGameLayer::create());
	Director::getInstance()->replaceScene(pScene);
}

void CCGameLayer::showPassAll()
{
	CCLabelTTF *pPassAll = CCLabelTTF::create("You Pass All", "Arial", 40);
	Point origin = Director::getInstance()->getVisibleOrigin();
	Size visibleSize = Director::getInstance()->getVisibleSize();

	pPassAll->setPosition(ccp(origin.x + visibleSize.width/2, origin.y + visibleSize.height));
	this->addChild(pPassAll, this->getChildrenCount());
	CCMoveTo *pTo = CCMoveTo::create(0.3f, ccp(origin.x + visibleSize.width/2, origin.y + visibleSize.height/2));
	DelayTime *pDelay = DelayTime::create(2.0f);
	pPassAll->runAction(Sequence::create(pTo, pDelay,
		CallFunc::create(this, callfunc_selector(CCGameLayer::toMainMenu)), NULL));
}

void CCGameLayer::pauseGameLayer()
{
	mainMap->pauseGameMap();
	this->unscheduleUpdate();

	//pLeftKey->setEnabled(false);
	//pRightKey->setEnabled(false);
	//pJump->setEnabled(false);
	//pFire->setEnabled(false);
	pMenuArrow->setEnabled(false);
	pMenuFireBall->setEnabled(false);

	pColor->setVisible(true);
	pSetMenu->setVisible(true);
	pResume->setVisible(true);
	pResume->setEnabled(true);
	pReStart->setVisible(true);
	pReStart->setEnabled(true);
	pSelectMenu->setVisible(true);
	pSelectMenu->setEnabled(true);
}

void CCGameLayer::resumeGameLayer()
{
	mainMap->resumeGameMap();
	this->scheduleUpdate();

	//pLeftKey->setEnabled(true);
	//pRightKey->setEnabled(true);
	//pJump->setEnabled(true);
	//pFire->setEnabled(true);
	pMenuArrow->setEnabled(true);
	pMenuFireBall->setEnabled(true);

	pColor->setVisible(false);
	pSetMenu->setVisible(false);
	pResume->setVisible(false);
	pResume->setEnabled(false);
	pReStart->setVisible(false);
	pReStart->setEnabled(false);
	pSelectMenu->setVisible(false);
	pSelectMenu->setEnabled(false);
}






