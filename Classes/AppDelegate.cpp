#include <cstdlib>
#include <ctime>
#include "AppDelegate.h"

//#include "1_DebugDrawScene.h"

#define BOX2D_EXAMPLE 0

#if BOX2D_EXAMPLE == 10
#include "StaticDynamicScene.h"
#elif  BOX2D_EXAMPLE == 11
#include "FixtureCollisionScene.h"
#elif  BOX2D_EXAMPLE == 12
#include "JointScene.h"
#elif  BOX2D_EXAMPLE == 0
#include "MainMenu.h"
#elif  BOX2D_EXAMPLE == 1
#include "JointLevel.h"
#elif  BOX2D_EXAMPLE == 2
#include "GearLevel.h"
#elif  BOX2D_EXAMPLE == 3
#include "GravityLevel.h"
#elif  BOX2D_EXAMPLE == 4
#include "CarLevel.h"
#endif

USING_NS_CC;

AppDelegate::AppDelegate() {

}

AppDelegate::~AppDelegate() 
{
}

//if you want a different context,just modify the value of glContextAttrs
//it will takes effect on all platforms
void AppDelegate::initGLContextAttrs()
{
    //set OpenGL context attributions,now can only set six attributions:
    //red,green,blue,alpha,depth,stencil
    GLContextAttrs glContextAttrs = {8, 8, 8, 8, 24, 8};

    GLView::setGLContextAttrs(glContextAttrs);
}

bool AppDelegate::applicationDidFinishLaunching() {
    // initialize director
    auto director = Director::getInstance();
    auto glview = director->getOpenGLView();
	if (!glview) {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_MAC) || (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)
		glview = GLViewImpl::createWithRect("MyBox2D", Rect(0, 0, 1920, 1080), 0.6f);
#else
		glview = GLViewImpl::create("MyBox2D");
#endif
		director->setOpenGLView(glview);
	}

	director->getOpenGLView()->setDesignResolutionSize(1280, 720, ResolutionPolicy::SHOW_ALL);

    // turn on display FPS
    director->setDisplayStats(true);

    // set FPS. the default value is 1.0/60 if you don't call this
    director->setAnimationInterval(1.0f / 60.0f);

    FileUtils::getInstance()->addSearchPath("res");

	srand(time(NULL));

    // create a scene. it's an autorelease object
#if BOX2D_EXAMPLE == 10
	auto scene = StaticDynamic::createScene();
#elif  BOX2D_EXAMPLE == 11
	auto scene = FixtureCollision::createScene();
#elif  BOX2D_EXAMPLE == 12
	auto scene = JointScene::createScene();
#elif  BOX2D_EXAMPLE == 0
	auto scene = MainMenu::createScene();
#elif  BOX2D_EXAMPLE == 1
	auto scene = JointLevel::createScene();
#elif  BOX2D_EXAMPLE == 2
	auto scene = GearLevel::createScene();
#elif  BOX2D_EXAMPLE == 3
	auto scene = GravityLevel::createScene();
#elif  BOX2D_EXAMPLE == 4
	auto scene = CarLevel::createScene();
#endif

    // run
    director->runWithScene(scene);

    return true;
}

// This function will be called when the app is inactive. When comes a phone call,it's be invoked too
void AppDelegate::applicationDidEnterBackground() {
    Director::getInstance()->stopAnimation();

    // if you use SimpleAudioEngine, it must be pause
    // SimpleAudioEngine::getInstance()->pauseBackgroundMusic();
}

// this function will be called when the app is active again
void AppDelegate::applicationWillEnterForeground() {
    Director::getInstance()->startAnimation();

    // if you use SimpleAudioEngine, it must resume here
    // SimpleAudioEngine::getInstance()->resumeBackgroundMusic();
}
