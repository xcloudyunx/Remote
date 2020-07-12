#include "TrackpadScene.h"

Scene* Trackpad::createScene() {
    return Trackpad::create();
}

bool Trackpad::init() {
    if (!Scene::init()) {
        return false;
    }

    _visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
	
	auto background = Sprite::create("background.png");
	background->setPosition(_visibleSize/2);
	this->addChild(background);

	_server = socket(AF_INET, SOCK_DGRAM, 0);
	_serverAddr.sin_family = AF_INET; 
	_serverAddr.sin_port = htons(PORT); 
	_serverAddr.sin_addr.s_addr = inet_addr(IP.c_str());
	
	auto back = Button::create("default.png");
	back->setPosition(Vec2(_visibleSize.width/20, _visibleSize.height-_visibleSize.width/20));
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
	
	auto touchListener = EventListenerTouchOneByOne::create();
	
	_state = "";
	
	touchListener->onTouchBegan = CC_CALLBACK_2(Trackpad::onTouchBegan, this);
	
	touchListener->onTouchMoved = CC_CALLBACK_2(Trackpad::onTouchMoved, this);
	
	touchListener->onTouchEnded = CC_CALLBACK_2(Trackpad::onTouchEnded, this);

	_eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, this);

	return true;
}

bool Trackpad::onTouchBegan(Touch* touch, Event* event) {
	if (_state == "") _state = "left click";
	else if (_state == "potential double") _state = "double";
	else if (_state == "move") _state = "add while move 2";
	else if (_state == "left click") _state = "right click";
	else if (_state == "right click") _state = "search";
	else if (_state == "add while move 2") _state = "add while move 3";
	else if (_state == "search") _state = "action centre";
	else if (_state == "add while move 3") _state = "add while move 4";
	
	log("%s", _state.c_str());
	_touches.push_back(touch);
	return true;
}

void Trackpad::onTouchMoved(Touch* touch, Event* event) {
	auto deltaPos = touch->getLocation()-touch->getStartLocation();
	if (abs(deltaPos.x) > 3.0f || abs(deltaPos.y) > 3.0f) {
		if (_state == "left click") {
			_state = "move";
		}
		else if (_state == "double") {
			_state = "drag start";
			SEND(_state.c_str());
		}
		else if (_state == "right click" || _state == "add while move 2") {
			_num++;
			if (_num < 5) return;
			_num = 0;
			std::vector<Vec2> delta;
			for (auto t : _touches) delta.push_back(t->getLocation()-t->getStartLocation());
			auto dir0 = abs(delta[0].y)/delta[0].y;
			auto dir1 = abs(delta[1].y)/delta[1].y;
			auto angle0 = abs(delta[0].getAngle()*(180/3.141592));
			auto angle1 = abs(delta[1].getAngle()*(180/3.141592));
			
			// greater than 90 degrees between fingers
			if (delta[0].dot(delta[1]) == 0 || delta[0].dot(delta[1])/(delta[0].length()*delta[1].length()) <= 0) {
				_state = "zoom";
				_midPoint = Vec2(0, 0);
				for (auto i : _touches) {
					_midPoint += i->getStartLocation();
				}
				_midPoint = _midPoint / _touches.size();
			}
			// vertical motion and less than 50 degrees from vertical
			else if ((dir0 == dir1 || std::isnan(dir0) || std::isnan(dir1)) && ((angle0 >= 40.0f && angle0 <= 140.0f) || delta[0].isZero()) && ((angle1 >= 40.0f && angle1 <= 140.0f) || delta[1].isZero())) {
				_state = "scroll";
			}
			else {
				_state = "two finger movement";
			}
		}
		else if (_state == "search" || _state == "add while move 3") {
			_num++;
			if (_num < 5) return;
			_num = 0;
			// 45 degree cutoff between vertical and horizontal movements
			if (abs(deltaPos.x) > abs(deltaPos.y)) {
				_state = "alt tab";
				SEND("show start");
			}
			else if (deltaPos.y > 0) _state = "show all";
			else if (deltaPos.y < 0) _state = "show desktop";
			else _state = "three finger movement";
			
		}
		else if (_state == "action centre" || _state == "add while move 4") {
			_num++;
			if (_num < 5) return;
			_num = 0;
			if (deltaPos.x > 0) _state = "desk prev";
			else if (deltaPos.x < 0) _state = "desk next";
			else _state = "four finger movement"; //desk prev/next
		}
	}
	
	deltaPos = touch->getLocation()-touch->getPreviousLocation();
	if (_state == "move" || _state == "drag start") {
		move(deltaPos);
	} else if (_state == "scroll") {
		scroll(deltaPos);
	} else if (_state == "zoom") {
		zoom(deltaPos, touch->getLocation(), touch->getPreviousLocation());
	} else if (_state == "alt tab") {
		altTab(deltaPos);
	} 
	
	log("%s", _state.c_str());
}

void Trackpad::onTouchEnded(Touch* touch, Event* event) {
	_touches.erase(std::remove(_touches.begin(), _touches.end(), touch), _touches.end());
	if (_touches.size() == 0) {
		if (_state == "left click") {
			_state = "potential double";
			this->runAction(Sequence::create(
								DelayTime::create(0.25f),
								CallFunc::create([&](){
									if (_state != "drag start" && _state != "double") {
										SEND("left click");
										if (_state == "potential double") _state = "";
									}
								}),
								nullptr
							));
		} else {
			if (_state == "double") _state = "left click";
			else if (_state == "drag start") _state = "drag end";
			else if (_state == "move" || _state == "scroll") _state = "";
			else if (_state == "alt tab") _state = "show end";
			SEND(_state.c_str());
			_state = "";
		}
	}
	log("%s", _state.c_str());
}

void Trackpad::move(Vec2 deltaPos) {
	double v = pow(deltaPos.length() * 60, 0.65f) * 1.5f;
	deltaPos *= v/60;
	std::string tmp = "move " + std::to_string(deltaPos.x) + " " + std::to_string(-deltaPos.y);
	SEND(tmp.c_str());
}

void Trackpad::scroll(Vec2 deltaPos) {
	double v = pow(deltaPos.length() * 60, 0.8f);
	deltaPos *= v/60;
	std::string tmp = "scroll " + std::to_string((int) -deltaPos.y);
	SEND(tmp.c_str());
}

void Trackpad::zoom(Vec2 deltaPos, Vec2 newPos, Vec2 oldPos) {
	double v = pow(deltaPos.length() * 60, 0.5f);
	if (newPos.distanceSquared(_midPoint) > oldPos.distanceSquared(_midPoint)) {
		_zoomThreshold += v;
	} else {
		_zoomThreshold -= v;
	}
	if (_zoomThreshold < -50.0f) {
		SEND("zoom out");
	} else if (_zoomThreshold > 50.0f) {
		SEND("zoom in");
	}
	_zoomThreshold = fmod(_zoomThreshold, 50.0f);
	
}

void Trackpad::altTab(Vec2 deltaPos) {
	double v = pow(deltaPos.length() * 60, 0.5f);
	if (deltaPos.x > 0) {
		_showThreshold += v;
	} else if (deltaPos.x < 0) {
		_showThreshold -= v;
	}
	if (_showThreshold > 100.0f) {
		SEND("show next");
	} else if (_showThreshold < -100.0f) {
		SEND("show prev");
	}
	_showThreshold = fmod(_showThreshold, 100.0f);
}

void Trackpad::SEND(const char* msg) {
	sendto(_server, msg, strlen(msg), 0, (sockaddr*) &_serverAddr, sizeof(_serverAddr));
	log("msg: %s", msg);
}