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
	
	auto touchListener = EventListenerTouchAllAtOnce::create();
	
	touchListener->onTouchesBegan = [&](const std::vector<Touch*> &touches, Event* event) {
		_firstMove = true;
		if (_touches == 0) {
			_click = true;
			this->runAction(Sequence::create(
									DelayTime::create(0.3f),
									CallFunc::create([&](){
										if (_click) {
											_click = false;
											_move = true;
										}
									}),
									nullptr));
			if (_dragPotential) {
				_drag = true;
				_dragPotential = false;
				SEND("drag start");
			}
		}
		_touches += touches.size();
	};
	
	touchListener->onTouchesMoved = [&](const std::vector<Touch*> &touches, Event* event) {
		if (_firstMove) {
			this->runAction(Sequence::create(
									DelayTime::create(0.05f),
									CallFunc::create([&](){
										if (_firstMove) {
											_firstMove = false;
										}
									}),
									nullptr));
		} else {
			move(touches);
		}
	};
	
	touchListener->onTouchesEnded = [&](const std::vector<Touch*> &touches, Event* event) {
		if (_touches == 1) {
			if (_click) {
				_click = false;
				_dragPotential = true;
				this->runAction(Sequence::create(
										DelayTime::create(0.25f),
										CallFunc::create([&](){
											if (_dragPotential) {
												_dragPotential = false;
											}
										}),
										nullptr));
				this->runAction(Sequence::create(
										DelayTime::create(0.25f),
										CallFunc::create([&](){
											if (!_drag) {
												SEND("left click");
											}
										}),
										nullptr));
			}
			if (_move) {
				_move = false;
			}
			if (_drag) {
				_drag = false;
				SEND("drag end");
			}
		} else if (_touches == 2) {
			if (_click) {
				_click = false;
				SEND("right click");
			}
			if (_move) {
				_move = false;
			}
			if (_scroll) {
				_scroll = false;
			}
			if (_zoom) {
				_zoomThreshold = 0;
				_zoom = false;
			}
		} else if (_touches == 3) {
			if (_click) {
				_click = false;
				SEND("search");
			} else {
				if (_triple == "show next" || _triple == "show prev") {
					_triple = "show end";
					_showThreshold = 0;
				}
				SEND(_triple.c_str());
				_move = false;
			}
		}  else if (_touches == 4) {
			SEND(_quad.c_str());
			_move = false;
		}
		_touches = 0;
	};

	_eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, this);

	return true;
}

void Trackpad::move(const std::vector<Touch*> &touches) {
	if (_touches == 1) {
		Vec2 deltaPos = touches[0]->getLocation()-touches[0]->getPreviousLocation();
		
		if (_click && (abs(deltaPos.x)>_visibleSize.width*0.005f || abs(deltaPos.y)>_visibleSize.width*0.005f)) {
			_click = false;
			_move = true;
		}
		if (_move) {
			double v = sqrt(pow(deltaPos.x, 2) + pow(deltaPos.y, 2)) * 60;
			v = pow(v, 0.65f)*1.5f;
			deltaPos *= v/60;
			std::string tmp = "move " + std::to_string(deltaPos.x) + " " + std::to_string(-deltaPos.y);
			SEND(tmp.c_str());
		}
	} else if (_touches == 2) {
		Vec2 deltaPos0 = touches[0]->getLocation()-touches[0]->getPreviousLocation();
		Vec2 deltaPos1 = touches[1]->getLocation()-touches[1]->getPreviousLocation();
		
		if (_click && ((abs(deltaPos0.x) > _visibleSize.width*0.005f || abs(deltaPos0.y) > _visibleSize.width*0.005f) || (abs(deltaPos1.x) > _visibleSize.width*0.005f || abs(deltaPos1.y) > _visibleSize.width*0.005f))) {
			_click = false;
			_move = true;
		}
		if (_move) {
			if (!_zoom && (_scroll || ((abs(deltaPos0.y)/deltaPos0.y == abs(deltaPos1.y)/deltaPos1.y) && (abs(deltaPos0.x-deltaPos1.x) <= _visibleSize.width*0.05f)))) {
				_scroll = true;
				double v = sqrt(pow(deltaPos0.x, 2) + pow(deltaPos0.y, 2)) * 60;
				v = pow(v, 0.8f);
				deltaPos0 *= v/60;
				std::string tmp = "scroll " + std::to_string((int) -deltaPos0.y);
				SEND(tmp.c_str());
			} else if (!_scroll && abs(deltaPos0.x)/deltaPos0.x != abs(deltaPos1.x)/deltaPos1.x) {
				_zoom = true;
				if (touches[0]->getPreviousLocation().x > touches[1]->getPreviousLocation().x) {
					std::swap(deltaPos0, deltaPos1);
				}
				double v = sqrt(pow(deltaPos0.x, 2) + pow(deltaPos0.y, 2)) * 60;
				v = pow(v, 0.5f);
				if (deltaPos0.x > 0) {
					_zoomThreshold -= v;
				} else if (deltaPos0.x < 0) {
					_zoomThreshold += v;
				}
			} else if (!_scroll && abs(deltaPos0.y)/deltaPos0.y != abs(deltaPos1.y)/deltaPos1.y) {
				_zoom = true;
				if (touches[0]->getPreviousLocation().y > touches[1]->getPreviousLocation().y) {
					std::swap(deltaPos0, deltaPos1);
				}
				double v = sqrt(pow(deltaPos0.x, 2) + pow(deltaPos0.y, 2)) * 60;
				v = pow(v, 0.5f);
				if (deltaPos0.y > 0) {
					_zoomThreshold -= v;
				} else if (deltaPos0.y < 0) {
					_zoomThreshold += v;
				}
			}
			if (_zoom) {
				if (_zoomThreshold < -50.0f) {
					SEND("zoom out");
				} else if (_zoomThreshold > 50.0f) {
					SEND("zoom in");
				}
				_zoomThreshold = fmod(_zoomThreshold, 50.0f);
			}
		}
	} else if (_touches == 3) {
		Vec2 deltaPos0 = touches[0]->getLocation()-touches[0]->getPreviousLocation();
		Vec2 deltaPos1 = touches[1]->getLocation()-touches[1]->getPreviousLocation();
		Vec2 deltaPos2 = touches[2]->getLocation()-touches[2]->getPreviousLocation();
		
		if (_click && ((abs(deltaPos0.x) > _visibleSize.width*0.005f || abs(deltaPos0.y) > _visibleSize.width*0.005f) || (abs(deltaPos1.x) > _visibleSize.width*0.005f || abs(deltaPos1.y) > _visibleSize.width*0.005f) || (abs(deltaPos2.x) > _visibleSize.width*0.005f || abs(deltaPos2.y) > _visibleSize.width*0.005f))) {
			_click = false;
			_move = true;
		}
		if (_move) {
			if ((abs(deltaPos0.y)/deltaPos0.y == abs(deltaPos1.y)/deltaPos1.y) && (abs(deltaPos0.y)/deltaPos0.y == abs(deltaPos2.y)/deltaPos2.y) && (abs(deltaPos0.y) > abs(deltaPos0.x))) {
				if (deltaPos0.y > 0) {
					_triple = "show all";
				} else if (deltaPos0.y < 0) {
					_triple = "show desktop";
				}
			} else if ((abs(deltaPos0.x)/deltaPos0.x == abs(deltaPos1.x)/deltaPos1.x) && (abs(deltaPos0.x)/deltaPos0.x == abs(deltaPos2.x)/deltaPos2.x) && (abs(deltaPos0.x) > abs(deltaPos0.y))) {
				double v = sqrt(pow(deltaPos0.x, 2) + pow(deltaPos0.y, 2)) * 60;
				v = pow(v, 0.5f);
				log("v: %f", v);
				if (deltaPos0.x > 0) {
					_showThreshold += v;
				} else if (deltaPos0.x < 0) {
					_showThreshold -= v;
				}
				if (_showThreshold > 100.0f) {
					_triple = "show next";
					SEND("show next");
				} else if (_showThreshold < -100.0f) {
					_triple = "show prev";
					SEND("show prev");
				}
				_showThreshold = fmod(_showThreshold, 100.0f);
			}
		}
	} else if (_touches == 4) {
		Vec2 deltaPos0 = touches[0]->getLocation()-touches[0]->getPreviousLocation();
		Vec2 deltaPos1 = touches[1]->getLocation()-touches[1]->getPreviousLocation();
		Vec2 deltaPos2 = touches[2]->getLocation()-touches[2]->getPreviousLocation();
		Vec2 deltaPos3 = touches[3]->getLocation()-touches[3]->getPreviousLocation();
		
		if (_click && ((abs(deltaPos0.x) > _visibleSize.width*0.005f || abs(deltaPos0.y) > _visibleSize.width*0.005f) || (abs(deltaPos1.x) > _visibleSize.width*0.005f || abs(deltaPos1.y) > _visibleSize.width*0.005f) || (abs(deltaPos2.x) > _visibleSize.width*0.005f || abs(deltaPos2.y) > _visibleSize.width*0.005f) || (abs(deltaPos3.x) > _visibleSize.width*0.005f || abs(deltaPos3.y) > _visibleSize.width*0.005f))) {
			_click = false;
			_move = true;
		}
		if (_move) {
			if ((abs(deltaPos0.x)/deltaPos0.x == abs(deltaPos1.x)/deltaPos1.x) && (abs(deltaPos0.x)/deltaPos0.x == abs(deltaPos2.x)/deltaPos2.x) && (abs(deltaPos0.x)/deltaPos0.x == abs(deltaPos3.x)/deltaPos3.x) && (abs(deltaPos0.x) > abs(deltaPos0.y))) {
				if (deltaPos0.x > 0) {
					_quad = "desk prev";
				} else if (deltaPos0.x < 0) {
					_quad = "desk next";
				}
			}
		}
	}
}

void Trackpad::SEND(const char* msg) {
	sendto(_server, msg, strlen(msg), 0, (sockaddr*) &_serverAddr, sizeof(_serverAddr));
	log("%s", msg);
}