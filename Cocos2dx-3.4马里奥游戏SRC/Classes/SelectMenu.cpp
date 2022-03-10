#include "SelectMenu.h"
#include "MainMenu.h"
#include "Global.h"
#include "GameLayer.h"
#include <string>
using namespace std;


bool CCSelectMenu::init()
{
	bool bRet = false;
	do
	{
		CC_BREAK_IF( !Layer::init() );

		m_nCurPage = 1;
		Size visibleSize = Director::getInstance()->getVisibleSize();
		Point origin = Director::getInstance()->getVisibleOrigin();

		pLayer = Layer::create();
		CCString *seleteName = NULL;
		for (int i = 1; i <= Global::getGlobalInstance()->getTotalLevels(); ++i)
		{
			seleteName = CCString::createWithFormat("select%d.jpg", i);
			Sprite *pSprite = Sprite::create(seleteName->getCString());
			pSprite->setPosition(ccp(visibleSize.width * (i-0.5f), visibleSize.height/2 + 10));
			pLayer->addChild(pSprite, 0, i);
		}

		pLayer->setContentSize(CCSizeMake(480*8, 320));
		this->addChild(pLayer, 1);

		Sprite *pBg = Sprite::create("bg.png");
		pBg->setPosition(ccp(origin.x + visibleSize.width/2, origin.y + visibleSize.height/2));
		this->addChild(pBg, 0);

		pNewGame = MenuItemImage::create("newgameA.png", "newgameB.png",
			this, menu_selector(CCSelectMenu::menuBegin));
		pNewGame->setPosition(ccp(origin.x + visibleSize.width/2, 30));

		MenuItemImage *pBack = MenuItemImage::create("backA.png", "backB.png",
			this, menu_selector(CCSelectMenu::menuBack));
		pBack->setPosition(ccp(origin.x + 20, origin.y + visibleSize.height - 20));

		Menu *pMenu = Menu::create(pNewGame, pBack, NULL);
		pMenu->setAnchorPoint(Vec2::ZERO);
		pMenu->setPosition(Vec2::ZERO);
		this->addChild(pMenu, 2);

		m_pLevel = CCLabelTTF::create("Level: 1", "Arial", 20);
		m_pLevel->setPosition(ccp(origin.x + visibleSize.width/2, origin.y + visibleSize.height - 20));
		this->addChild(m_pLevel, 3);

		bRet = true;
	}while(0);

	return bRet;

}

void CCSelectMenu::menuCloseCallback(Ref* pSender)
{

}

void CCSelectMenu::onEnter()
{
	Layer::onEnter();
	//以后更改
	//在那个类中实现触屏回调函数
	//Director::getInstance()->getTouchDispatcher()->addTargetedDelegate(this, 1, false);

	CCString *ccStr = NULL;
	string str;
	Sprite *pSp = NULL;
	for (int i = 2; i <= 8; ++i)
	{
		ccStr = CCString::createWithFormat("Level%d", i);
		str = UserDefault::sharedUserDefault()->getStringForKey(ccStr->getCString());
		if (str == "no")
		{
			LayerColor *pColor = LayerColor::create(ccc4(0, 0, 0, 200));
			pSp = (Sprite*)pLayer->getChildByTag(i);
			pColor->ignoreAnchorPointForPosition(false);
			pColor->setAnchorPoint(ccp(0.5, 0.5));
			pColor->setPosition(pSp->getPosition());
			pColor->setContentSize(pSp->getContentSize());
			pLayer->addChild(pColor, pSp->getZOrder() + 1);
		}
	}
}

void CCSelectMenu::onExit()
{
	//以后更改
	//Director::getInstance()->getTouchDispatcher()->removeDelegate(this);
	Layer::onExit();
}


bool CCSelectMenu::onTouchBegan(Touch *pTouch, Event *pEvent)
{
	m_touchPoint = Director::getInstance()->convertToGL(pTouch->getLocation());
	return true;
}

void CCSelectMenu::onTouchMoved(Touch *pTouch, Event *pEvent)
{

}

void CCSelectMenu::onTouchEnded(Touch *pTouch, Event *pEvent)
{
	Point endPoint = Director::getInstance()->convertToGL(pTouch->getLocation());
	float distance = endPoint.x - m_touchPoint.x;
	if(fabs(distance) > 5)
	{
		adjustView(distance);
	}
}

void CCSelectMenu::onTouchCancelled(cocos2d::Touch *pTouch, cocos2d::Event *pEvent)
{
	Point endPoint = Director::getInstance()->convertToGL(pTouch->getLocation());
	float distance = endPoint.x - m_touchPoint.x;
	if(fabs(distance) > 5)
	{
		adjustView(distance);
	}
}

void CCSelectMenu::adjustView(float offset)
{
	Size visibleSize = Director::getInstance()->getVisibleSize();
	Point origin = Director::getInstance()->getVisibleOrigin();

	if (offset<0)
	{
		++m_nCurPage;
	}else
	{
		--m_nCurPage;
	}

	if (m_nCurPage <1)
	{
		m_nCurPage = 1;
	}
	if (m_nCurPage > 8)
	{
		m_nCurPage = 8;
	}

	Point  adjustPos = ccp(origin.x - visibleSize.width * (m_nCurPage-1), 0);
	pLayer->runAction(CCMoveTo::create(0.2f, adjustPos));


	CCString *ccStr = CCString::createWithFormat("Level: %d", m_nCurPage);
	m_pLevel->setString(ccStr->getCString());
	ccStr = CCString::createWithFormat("Level%d", m_nCurPage);
	string str = UserDefault::sharedUserDefault()->getStringForKey(ccStr->getCString());
	if (str == "no")
	{
		pNewGame->setEnabled(false);
	}else
	{
		pNewGame->setEnabled(true);
	}
	    
}

void CCSelectMenu::menuBegin( Ref *pSender )
{
	Global::getGlobalInstance()->setCurrentLevel(m_nCurPage);
	CCGameLayer *pGameLayer = CCGameLayer::create();

	Scene *pScene = Scene::create();
	pScene->addChild(pGameLayer);
	Director::getInstance()->replaceScene(pScene);
}

void CCSelectMenu::menuBack( Ref *pSender )
{
	MainMenu *pMainMenu = MainMenu::create();
	Scene *pScene = Scene::create();
	pScene->addChild(pMainMenu);
	Director::getInstance()->replaceScene(pScene);
}
