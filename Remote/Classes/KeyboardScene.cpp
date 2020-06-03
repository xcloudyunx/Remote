#include "KeyboardScene.h"

Scene* Keyboard::createScene() {
    return Keyboard::create();
}

bool Keyboard::init() {
    if (!Scene::init()) {
        return false;
    }

    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
	
	auto background = Sprite::create("background.png");
	background->setPosition(visibleSize/2);
	this->addChild(background);

	_server = socket(AF_INET, SOCK_DGRAM, 0);
	_serverAddr.sin_family = AF_INET; 
	_serverAddr.sin_port = htons(PORT); 
	_serverAddr.sin_addr.s_addr = inet_addr(IP.c_str());
	
	auto board = EditBox::create(Size(0, 0), Scale9Sprite::create("background.png"));
	board->setText(" ");
	board->setDelegate(this);
	board->openKeyboard();
	this->addChild(board);
	
	auto back = Button::create("default.png");
	back->setPosition(Vec2(visibleSize.width/20, visibleSize.height-visibleSize.width/20));
	back->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type){
		switch (type) {
			case Widget::TouchEventType::BEGAN:
				break;
			case Widget::TouchEventType::CANCELED:
				break;
			case Widget::TouchEventType::ENDED:
				SEND("exit");
				close(_server);
				Director::getInstance()->popScene();
				break;
			default:
				break;
		}
	});
	this->addChild(back);

	return true;
}

void Keyboard::editBoxTextChanged(EditBox* editbox, const std::string &text) {
	log("CHANGED");
	editbox->setText(" ");
	if (text.size() == 0) {
		SEND("backspace");
	} else {
		SEND(text.substr(1, text.size()).c_str());
	}
}

void Keyboard::editBoxReturn(EditBox* editbox) {
	log("DONE");
}

void Keyboard::SEND(const char* msg) {
	sendto(_server, msg, strlen(msg), 0, (sockaddr*) &_serverAddr, sizeof(_serverAddr));
	log("%s", msg);
}