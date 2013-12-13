#include "UIManager.h"
#include "AppDemo.h"
#include "CommandManager.h"
#include "MainMenuControl.h"

using namespace RssApp;

template<> UIManager* Ogre::Singleton<UIManager>::msSingleton = 0;

UIManager:: UIManager()
{
	this->mDemoGUI = new MyGUI::Gui();
	this->mPlatform = new MyGUI::OgrePlatform();
}

UIManager::~UIManager()
{
	delete CommandManager::getInstancePtr();
	delete MainMenuControl::getInstancePtr();

	this->mDemoGUI->shutdown();
	delete mDemoGUI;
	this->mDemoGUI = nullptr;
	this->mPlatform->shutdown();
	delete mPlatform;
	this->mPlatform = nullptr;
}

bool UIManager::bootUI(const Ogre::RenderWindow* wnd, Ogre::SceneManager* mgr)
{
	if(!mPlatform) 
		return false;
	this->mPlatform->initialise(const_cast<Ogre::RenderWindow*>(wnd),  const_cast<Ogre::SceneManager*>(mgr));
	if(!mDemoGUI)
		return false;
	mDemoGUI->initialise("MyGUI_Core.xml");
	initialiseUI(wnd);
	return true;
}

void UIManager::initialiseUI(const Ogre::RenderWindow* wnd)
{
	new CommandManager();
	new MainMenuControl();

}

void UIManager::windowResized(Ogre::RenderWindow* rw)
{

}