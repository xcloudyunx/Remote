#include "newIPScene.h"

Scene* newIP::createScene() {
    return newIP::create();
}

bool newIP::init() {
    if (!Scene::init()) {
        return false;
    }
	
	FileUtils::getInstance()->addSearchPath(FileUtils::getInstance()->getWritablePath(), true);
	
	/*if (UserDefault::getInstance()->getStringForKey("ip", "NEW") != "NEW") {
		this->runAction(CallFunc::create([](){
			Director::getInstance()->pushScene(Home::createScene());
		}));
	}*/

	Size visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();
	
	auto background = Sprite::create("background.png");
	background->setPosition(visibleSize/2);
	this->addChild(background);
	
	auto ipInput = EditBox::create(Size(800, 80), Scale9Sprite::create("ip.png"));
	ipInput->setPlaceHolder("Enter IP address here");
	ipInput->setInputMode(EditBox::InputMode::NUMERIC);
	ipInput->setMaxLength(15);
	ipInput->setReturnType(EditBox::KeyboardReturnType::DONE);
	ipInput->setDelegate(this);
	ipInput->setName("IP");
	ipInput->setPosition(Vec2(visibleSize.width/2, visibleSize.height*3/4));
	this->addChild(ipInput);
	
	return true;
}

void newIP::editBoxReturn(EditBox* editBox) {
	log("DONE");
	UserDefault::getInstance()->setStringForKey("ip", editBox->getText());
	Director::getInstance()->pushScene(Home::createScene());
}