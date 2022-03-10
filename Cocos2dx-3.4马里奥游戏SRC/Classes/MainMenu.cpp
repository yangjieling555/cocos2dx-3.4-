#include "MainMenu.h"
#include "GameLayer.h"
#include "Global.h"
#include "SelectMenu.h"
#include "AboutMenu.h"
#include "SetMusic.h"

MainMenu::MainMenu()
{
	Size winSize = Director::getInstance()->getWinSize();

	Sprite *pBackground = Sprite::create("background.png");
	pBackground->setPosition(ccp(winSize.width/2, winSize.height/2));
	this->addChild(pBackground);
	
	pStartGame = MenuItemImage::create("startgame_normal.png", "startgame_select.png",
		this, menu_selector(MainMenu::menuCallBackForStartGame));
	pStartGame->setPosition(ccp(winSize.width/2, winSize.height/2));

	pQuit = MenuItemImage::create("quitgame_normal.png", "quitgame_select.png",
		this, menu_selector(MainMenu::menuQuit));
	pQuit->setPosition(ccp(winSize.width/2, winSize.height/2 - 40));

	pAbout = MenuItemImage::create("about_normal.png", "about_select.png",
		this, menu_selector(MainMenu::menuCallBackForAbout));
	pAbout->setPosition(ccp(winSize.width - 50, 20));

	pSetting = MenuItemImage::create("Setting_n.png", "setting_s.png",
		this, menu_selector(MainMenu::menuSetting));
	pSetting->setPosition(ccp(winSize.width/2, winSize.height/2 - 80));

	pMenu = Menu::create(pStartGame, pAbout, pQuit, pSetting, NULL);
	pMenu->setPosition(ccp(0, 0));
	this->addChild(pMenu, 0);
}

MainMenu::~MainMenu()
{
	this->unscheduleAllSelectors();
}

bool MainMenu::init()
{
	this->setTouchEnabled(true);
	//Director::getInstance()->getTouchDispatcher()->addTargetedDelegate(this, 0, false);
	auto listen = EventListenerTouchOneByOne::create();
	listen->onTouchBegan = CC_CALLBACK_2(MainMenu::onTouchBegan, this);
	listen->onTouchMoved = CC_CALLBACK_2(MainMenu::onTouchMoved, this);
	listen->onTouchEnded = CC_CALLBACK_2(MainMenu::onTouchEnded, this);
	listen->onTouchCancelled = CC_CALLBACK_2(MainMenu::onTouchCancelled, this);
	listen->setSwallowTouches(true);
	Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listen, this);

	return true;
}


void MainMenu::menuCallBackForStartGame(Ref *pSender)
{
	Global::getGlobalInstance()->setCurrentLifeNum(3);
	//CCGlobal::getGlobalInstance()->reSetLevel();  // 关卡重置为第一关

	//CCGameLayer *pGameLayer = CCGameLayer::create();
	CCSelectMenu *pSelectMenu = CCSelectMenu::create();
	Scene *pScene = Scene::create();
	pScene->addChild(pSelectMenu);
	Director::getInstance()->replaceScene(pScene);
	//CCGameScene *pGameScene =  CCGameScene::getGameSceneInstance();
	//pGameScene->removeAllChildrenWithCleanup(true);

	//pGameScene->addChild(pSelectMenu);
}

void MainMenu::menuCallBackForAbout(Ref *pSender)
{
	CCAbout *pAbout = CCAbout::create();
	Scene *pScene = Scene::create();
	pScene->addChild(pAbout);
	Director::getInstance()->replaceScene(pScene);
}

bool MainMenu::onTouchBegan(Touch *touch, Event * pEvent)
{
	return true;
}

void MainMenu::onTouchEnded(Touch *touch, Event * pEvent)
{

}

void MainMenu::onTouchCancelled(Touch *touch, Event *pEvent)
{

}

void MainMenu::onTouchMoved(Touch *touch, Event *pEvent)
{

}

void MainMenu::menuQuit( Ref *pSender )
{
	Director::getInstance()->end();
}

void MainMenu::menuSetting( Ref *pSender )
{
	Scene *pScene = Scene::create();
	pScene->addChild(CCSetMusic::create());
	Director::getInstance()->replaceScene(pScene);
}

