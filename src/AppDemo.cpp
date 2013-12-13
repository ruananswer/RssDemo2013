#include <iostream>
#include "AppDemo.h"
#include <Windows.h>
#include "UIManager.h"
#include <MyGUI.h>
#include <MyGUI_OgrePlatform.h>

using namespace RssApp;

template<> AppDemo* Ogre::Singleton<AppDemo>::msSingleton = nullptr;

AppDemo::AppDemo()
	:mLog(0),
	mRoot(0),
	mRenderWnd(0),
	mInputMgr(0),
	mSceneMgr(0),
	mCamera(0),
	mTimer(0),
	mKeyboard(0),
	mMouse(0),
	mShutDown(false),
	mVideoFrame(nullptr),
	mVideoCapture(nullptr)
{
	 
}

AppDemo::~AppDemo()
{
	delete UIManager::getSingletonPtr();
	cvReleaseCapture(&mVideoCapture);
	cvDestroyWindow("video");

 	mRenderWnd->removeAllViewports();
   
	mSceneMgr->destroyAllCameras();
    mRoot->destroySceneManager(mSceneMgr);
	
	mLog->logMessage("Shutdown Application");

 	if(mInputMgr) OIS::InputManager::destroyInputSystem(mInputMgr);
	if(mRoot)	  delete mRoot;
}

bool AppDemo::initOgre(Ogre::String wndTitle)
{
	Ogre::LogManager* logMgr = new Ogre::LogManager();
	mLog = Ogre::LogManager::getSingletonPtr()->createLog("AppLog.log", true, true, false);
	
#ifdef _DEBUG
	mLog->setDebugOutputEnabled(true);
	mRoot = new Ogre::Root("config/plugins_d.cfg","config/ogre_d.cfg");
#else 
	mLog->setDebugOutputEnabled(false);
	mRoot = new Ogre::Root("config/plugins.cfg","config/ogre.cfg");
#endif
	if(!mRoot->restoreConfig())
	{
		if(!mRoot->showConfigDialog())
		 return false;
	}
	mRenderWnd = mRoot->initialise(true, wndTitle);
  
	setupResources();
  
    mTimer = new Ogre::Timer();
    mTimer->reset();
	
	initOIS();

	Ogre::WindowEventUtilities::addWindowEventListener(mRenderWnd,this);

	char dir[100];
	::GetCurrentDirectoryA(100,dir);
	mWorkDirectory = dir;
	return true;
}

void AppDemo::initOIS(void)
{
	unsigned long hWnd;
	mRenderWnd->getCustomAttribute("WINDOW",static_cast<void*>(&hWnd));

	OIS::ParamList pl;
	pl.insert(OIS::ParamList::value_type("WINDOW",Ogre::StringConverter::toString(hWnd)));
	pl.insert(OIS::ParamList::value_type("w32_mouse", "DISCL_FOREGROUND"));
	pl.insert(OIS::ParamList::value_type("w32_mouse", "DISCL_NONEXCLUSIVE"));
	
	mInputMgr = OIS::InputManager::createInputSystem(pl);
	mMouse    = static_cast<OIS::Mouse*>(mInputMgr->createInputObject(OIS::OISMouse,true));
	mKeyboard = static_cast<OIS::Keyboard*>(mInputMgr->createInputObject(OIS::OISKeyboard,true));
	
	mMouse->getMouseState().width  = mRenderWnd->getWidth();
	mMouse->getMouseState().height = mRenderWnd->getHeight();
	
    mKeyboard->setEventCallback(this);
    mMouse->setEventCallback(this);
}

void AppDemo::setupResources(void)
{
	Ogre::String secName, typeName, archName;
    Ogre::ConfigFile config;

#ifdef _DEBUG
    config.load("config/resources_d.cfg");
#else 
	config.load("config/resources.cfg");
#endif 

    Ogre::ConfigFile::SectionIterator seci = config.getSectionIterator();
    while (seci.hasMoreElements())
    {
        secName = seci.peekNextKey();
        Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
        Ogre::ConfigFile::SettingsMultiMap::iterator i;
        for (i = settings->begin(); i != settings->end(); ++i)
        {
            typeName = i->first;
            archName = i->second;
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(archName, typeName, secName);
        }
    }
    Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);
    Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
}

void AppDemo::windowResized(Ogre::RenderWindow* rw)
{
/*  这个让后面的子面板去执行了
 	mMouse->getMouseState().width  = rw->getWidth();
	mMouse->getMouseState().height = rw->getHeight();
	UIManager::getSingletonPtr()->windowResized(rw);*/
	unsigned int width, height, depth;
	int left, top;
	rw->getMetrics(width, height, depth, left, top);

	const OIS::MouseState &ms = mMouse->getMouseState();
	ms.width = width;
	ms.height = height;

}

bool AppDemo::keyPressed(const OIS::KeyEvent &keyEvt)
{
	if(mKeyboard->isKeyDown(OIS::KC_SYSRQ))   // take a screenshot
    {
        mRenderWnd->writeContentsToTimestampedFile("Screenshot_", ".jpg");
    }
	else if(keyEvt.key == OIS::KC_F5)   // refresh all textures
	{
		Ogre::TextureManager::getSingleton().reloadAll();
	}
	else if (keyEvt.key == OIS::KC_ESCAPE)
	{
		mShutDown = true;
	}
	MyGUI::InputManager::getInstance().injectKeyPress(MyGUI::KeyCode::Enum(keyEvt.key), keyEvt.text);
 	return true; 
}

bool AppDemo::keyReleased(const OIS::KeyEvent &keyEvt)
{
	MyGUI::InputManager::getInstance().injectKeyRelease(MyGUI::KeyCode::Enum(keyEvt.key));
	return true;
}

void AppDemo::initDemo()
{
 	//mSceneMgr
	mSceneMgr = mRoot->createSceneManager(Ogre::ST_GENERIC,"SceneMgr_Workbench");
	mSceneMgr->setAmbientLight(Ogre::ColourValue(0.7f,0.7f,0.7f));
	mSceneMgr->setShadowTechnique(Ogre::SHADOWTYPE_STENCIL_ADDITIVE);
	mSceneMgr->setFog(Ogre::FOG_EXP2,Ogre::ColourValue(0.8431f,0.941f,1.0f),0.0002f);
	//Camera
	mCamera = mSceneMgr->createCamera("DemoCamera");
	mCamera->setPosition(0.0f,20.0f,111.96152422706631880582339024518f);
	mCamera->lookAt(0.0,15.0,0.0);
 	mCamera->setNearClipDistance(1);

	//ViewPort
	mRenderWnd->removeAllViewports();
	Ogre::Viewport *vp = mRenderWnd->addViewport(mCamera);

	mCamera->setAspectRatio(vp->getActualWidth()/Ogre::Real(vp->getActualHeight()));
  	vp->setBackgroundColour(Ogre::ColourValue(0.8431f,0.941f,1.0f));

	new UIManager();
	UIManager::getSingletonPtr()->bootUI(mRenderWnd,mSceneMgr);
}

void AppDemo::createScene(void)
{
 	//Floor
	Ogre::Plane plane(Ogre::Vector3::UNIT_Y,Ogre::Vector3(0.0f,0.0f,0.0f));
	Ogre::MeshManager::getSingleton().createPlane("floor",Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,plane,2000,2000,40,40,true,1,40,40,Ogre::Vector3::UNIT_Z);
 	Ogre::Entity *ent = mSceneMgr->createEntity("floorEnt","floor");
  	mSceneMgr->getRootSceneNode()->attachObject(ent);
	ent->setCastShadows(false);
	ent->setMaterialName("RssDemo/Floor");
	ent->setQueryFlags(1);
	// light 
	Ogre::Light * l = mSceneMgr->createLight("mainLight");
	l->setType(Ogre::Light::LT_DIRECTIONAL);
	l->setDirection(Ogre::Vector3(0.2f,-1.0f,-0.5f));
	l->setPosition(Ogre::Vector3(20,40,20));
  	l->setDiffuseColour(Ogre::ColourValue(1.0,1.0,1.0));
	l->setSpecularColour(Ogre::ColourValue(1.0,1.0,1.0)); 
	//video texture
	mVideoCapture = cvCreateFileCapture("media\\Video\\Record_2013_12_10_01_44_24_101.avi");
	createVideoTexture();

	//implememting a video screen
	mVideoScreen = new Ogre::Rectangle2D(true);
	//mVideoScreen->setCorners(-1.0f, 0.75f, 1.0f, -0.75f);
	mVideoScreen->setCorners(-0.0f, 1.0f, 1.0f, -0.0f);
	mVideoScreen->setBoundingBox(Ogre::AxisAlignedBox(-100000.0f * Ogre::Vector3::UNIT_SCALE, 100000.0f * Ogre::Vector3::UNIT_SCALE));
	mVideoScreen->setMaterial(material->getName());
	Ogre::SceneNode *videoScreenNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("VideoScreenNode");
	videoScreenNode->attachObject(mVideoScreen);
}

void AppDemo::run()
{
	if(!initOgre("RssApp")) return;

	mLog->logMessage("Demo Initialised!");

	initDemo();
	createScene();
	createFrameListener();
	double timeSinceLastFrame = 1.0;
	int startTime = 0;

	mRoot->startRendering();

	while(!mShutDown)
	{
		if(mRenderWnd->isClosed())
			mShutDown = true;

		Ogre::WindowEventUtilities::messagePump();
		
		if(mRenderWnd->isActive())
		{
	 		startTime = mTimer->getMillisecondsCPU();

			mKeyboard->capture();
			mMouse->capture();
 			 
			mRoot->renderOneFrame();
  			timeSinceLastFrame = (mTimer->getMillisecondsCPU() - startTime)/1000.0f;
		}
		else
		{
	         Sleep(50);
  		}
	}
  	mLog->logMessage("Main Loop Quit");
}

bool AppDemo::mouseMoved(const OIS::MouseEvent &evt)
{
	MyGUI::PointerManager::getInstance().setVisible(false);
    bool ret = MyGUI::InputManager::getInstance().injectMouseMove(evt.state.X.abs,evt.state.Y.abs,evt.state.Z.abs);
	
    if(ret) return true;


	return true;
}

bool AppDemo::mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID id)
{
 	bool flag = MyGUI::InputManager::getInstance().injectMousePress(evt.state.X.abs,evt.state.Y.abs,MyGUI::MouseButton::Enum(id));
	if(!flag)
	{
		return false;
	}
		
 	return true;
}

bool AppDemo::mouseReleased(const OIS::MouseEvent &evt, OIS::MouseButtonID id)
{
 	bool flag = MyGUI::InputManager::getInstance().injectMouseRelease(evt.state.X.abs,evt.state.Y.abs,MyGUI::MouseButton::Enum(id));
	if(!flag)
	{
		return false;
	}
 	return true;
}

void AppDemo::createVideoTexture()
{
	mVideoFrame = cvQueryFrame(mVideoCapture);
	if(!mVideoFrame)
	{
		return;
	}

	// Implementing a video texture
	texture = Ogre::TextureManager::getSingleton().createManual(
		"VideoTexture",      // name
		Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		Ogre::TEX_TYPE_2D,   // type
		mVideoFrame->width,  // width
		mVideoFrame->height, // height
		0,                   // number of mipmaps
		Ogre::PF_B8G8R8A8,   // pixel format
		Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE // usage, for textures updated very often
		);
	// Get the pixel buffer
	pixelBuffer = texture->getBuffer();
	// Lock the pixel buffer and get a pixel box
	unsigned char* buffer = static_cast<unsigned char*>(
		pixelBuffer->lock(0, mVideoFrame->width*mVideoFrame->height*4, Ogre::HardwareBuffer::HBL_DISCARD) );
	for(int y = 0; y < mVideoFrame->height; ++y)
	{
		for(int x = 0; x < mVideoFrame->width; ++x)
		{
			buffer[ ((y*mVideoFrame->width)+x)*4 + 0 ] = mVideoFrame->imageData[ ((y*mVideoFrame->width)+x)*3 + 0 ]; // B
			buffer[ ((y*mVideoFrame->width)+x)*4 + 1 ] = mVideoFrame->imageData[ ((y*mVideoFrame->width)+x)*3 + 1 ]; // G
			buffer[ ((y*mVideoFrame->width)+x)*4 + 2 ] = mVideoFrame->imageData[ ((y*mVideoFrame->width)+x)*3 + 2 ]; // R
			buffer[ ((y*mVideoFrame->width)+x)*4 + 3 ] = 255;                                                        // A
		}
	}
	// Unlock the pixel buffer
	pixelBuffer->unlock();

	// Create a materail using the texture
	material = Ogre::MaterialManager::getSingleton().create(
		"DynamicTextureMaterial",//"VideoTextureMaterial", // name
		Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	material->getTechnique(0)->getPass(0)->createTextureUnitState("VideoTexture");
	material->getTechnique(0)->getPass(0)->setDepthCheckEnabled(false);
	material->getTechnique(0)->getPass(0)->setDepthWriteEnabled(false);
	material->getTechnique(0)->getPass(0)->setLightingEnabled(false);
}

void AppDemo::updateVodeoTexture()
{
	mVideoFrame = cvQueryFrame(mVideoCapture);
	if(!mVideoFrame)
	{
		return;
	}
	//opencv show
	//cvShowImage("img", mVideoFrame);
	//mImage = mVideoFrame;

	cv::Mat img(mVideoFrame, 0);
	mImage = img;
	//cv::imshow("Display Window",mImage);
	std::vector<cv::Rect> found, found_filtered;
	cv::HOGDescriptor people_dectect_hog;
	people_dectect_hog.setSVMDetector(cv::HOGDescriptor::getDefaultPeopleDetector());
	people_dectect_hog.detectMultiScale(mImage, found, 0, cv::Size(8, 8), cv::Size(32, 32), 1.05, 2);

	std::size_t i, j;
	for (i = 0; i < found.size(); i++)
	{
		cv::Rect r = found[i];
		for( j = 0; j < found.size(); j++ )  
			if( j != i && (r & found[j]) == r)  
				break;  
		if( j == found.size() )  
			found_filtered.push_back(r);  
	}

	for (i = 0; i < found_filtered.size(); i++)
	{
		cv::Rect r = found_filtered[i];
		r.x += cvRound(r.width*0.1);
		r.width = cvRound(r.width*0.8);
		r.y += cvRound(r.height*0.07);
		r.height = cvRound(r.height*0.8);
		cv::rectangle(mImage, r.tl(), r.br(), cv::Scalar(0, 255, 0), 3);	
	}
	//cv::imshow("Display Window",mImage);
	// Lock the pixel buffer
	unsigned char* buffer = static_cast<unsigned char*>(pixelBuffer->lock(0, mVideoFrame->width*mVideoFrame->height*4, Ogre::HardwareBuffer::HBL_DISCARD) );
	//const PixelBox &pb = pixelBuffer->getCurrentLock();

	for(int y = 0; y < mVideoFrame->height; ++y)
	{
		for(int x = 0; x < mVideoFrame->width; ++x)
		{
			buffer[ ((y*mVideoFrame->width)+x)*4 + 0 ] = mVideoFrame->imageData[ ((y*mVideoFrame->width)+x)*3 + 0 ]; // B
			buffer[ ((y*mVideoFrame->width)+x)*4 + 1 ] = mVideoFrame->imageData[ ((y*mVideoFrame->width)+x)*3 + 1 ]; // G
			buffer[ ((y*mVideoFrame->width)+x)*4 + 2 ] = mVideoFrame->imageData[ ((y*mVideoFrame->width)+x)*3 + 2 ]; // R
			buffer[ ((y*mVideoFrame->width)+x)*4 + 3 ] = 255;                                                        // A
		}
	}
	// Unlock the pixel buffer
	pixelBuffer->unlock();
}

bool AppDemo::frameRenderingQueued(const Ogre::FrameEvent &evt)
{
	if(!mVideoFrame)
	{
		return false;
	}
	else
	{
		updateVodeoTexture();
	}
	return true;
}

void AppDemo::createFrameListener(void)
{
	windowResized(mRenderWnd);
	mRoot->addFrameListener(this);
}
