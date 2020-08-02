#include "HomeScene.h"

Scene* Home::createScene() {
    return Home::create();
}

bool Home::init() {
    if (!Scene::init()) {
        return false;
    }
	
	this->setName("Home");

    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
	
	_background = Sprite::create("background.png");
	_background->setPosition(visibleSize/2);
	_background->setScale(std::max(visibleSize.width, visibleSize.height));
	this->addChild(_background);
	
	_server = Server::getInstance();
	if (!_server) {
		this->runAction(CallFunc::create([]() {
			Director::getInstance()->popScene();
		}));
		return true;
	}
	
	_rows = UserDefault::getInstance()->getIntegerForKey("rows", 3);
	_columns = UserDefault::getInstance()->getIntegerForKey("columns", 4);
	_pages = UserDefault::getInstance()->getIntegerForKey("pages", 1);
	_currentPage = 1;
	
	for (int k=1; k<=_pages; k++) {
		auto p = Page::create(k, _rows, _columns);
		this->addChild(p);
		_p.push_back(p);
	}
	_left = _p[_currentPage-1]->getLeft();
	_right = _p[_currentPage-1]->getRight();
	
	auto touchListener = EventListenerTouchOneByOne::create();
	
	touchListener->onTouchBegan = [](Touch* touch, Event* event){
		return true;
	};
	
	touchListener->onTouchEnded = [&](Touch* touch, Event* event) {
		auto delta = touch->getLocation()-touch->getStartLocation();
		if (abs(delta.x) > abs(delta.y)) {
			if (delta.x >= 0) {
				changePage((_currentPage+1)%_pages ? (_currentPage+1)%_pages : _pages);
			} else {
				changePage((_currentPage-1+_pages)%_pages ? (_currentPage-1+_pages)%_pages : _pages );
			}
		}
		
		/*if (touch->getLocation().x < _left) {
			changePage((_currentPage-1+_pages)%_pages ? (_currentPage-1+_pages)%_pages : _pages );
		} else if (touch->getLocation().x > _right) {
			changePage((_currentPage+1)%_pages ? (_currentPage+1)%_pages : _pages);
		}*/
	};

	_eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, this);
	
	
	std::thread serv([&](){
		while (true) {
			auto task = _server->RECV();
			log("%s", task.c_str());
			if (task == "new image") {
				auto id = stoi(_server->RECV());
				std::string image = _server->RECV();
				image = base64_decode(image);
				auto path = FileUtils::getInstance()->getWritablePath();
				FileUtils::getInstance()->writeStringToFile(image, path+std::to_string(id)+".png");
				Director::getInstance()->getTextureCache()->removeTextureForKey(std::to_string(id)+".png");
				Director::getInstance()->getScheduler()->performFunctionInCocosThread([&](){
					_p[id/TOTAL]->sync(id);
				});
			} else if (task == "reset image") {
				auto id = stoi(_server->RECV());
				auto path = FileUtils::getInstance()->getWritablePath();
				FileUtils::getInstance()->removeFile(path+std::to_string(id)+".png");
			} else if (task == "grid") {
				// CHECK IF USER HAS PREMIUM
				_pages = stoi(_server->RECV());
				_rows = stoi(_server->RECV());
				_columns = stoi(_server->RECV());
				UserDefault::getInstance()->setIntegerForKey("rows", _rows);
				UserDefault::getInstance()->setIntegerForKey("columns", _columns);
				UserDefault::getInstance()->setIntegerForKey("pages", _pages);
				
				Director::getInstance()->getScheduler()->performFunctionInCocosThread([&](){
					for (auto i : _p) {
						i->update();
					}
					if (_pages > _p.size()) {
						for (int k=_p.size()+1; k<=_pages; k++) {
							auto p = Page::create(k, _rows, _columns);
							this->addChild(p);
							_p.push_back(p);
						}
					}
					_left = _p[_currentPage-1]->getLeft();
					_right = _p[_currentPage-1]->getRight();
					changePage(std::min(_currentPage, _pages));
				});
			} else if (task == "trackpad") {
				Director::getInstance()->getScheduler()->performFunctionInCocosThread([&](){
					Director::getInstance()->pushScene(Trackpad::createScene());
				});
			} else if (task == "keyboard") {
				Director::getInstance()->getScheduler()->performFunctionInCocosThread([&](){
					Director::getInstance()->pushScene(Keyboard::createScene());
				});
			} else if (task == "numpad") {
				Director::getInstance()->getScheduler()->performFunctionInCocosThread([&](){
					Director::getInstance()->pushScene(Numpad::createScene());
				});
			}
		}
	});
	serv.detach();
	
	return true;
}

void Home::changePage(int page) {
	log("CHANGE PAGE TO %d", page);
	_currentPage = page;
	for (auto i: _p) {
		if (i->getPage() == page) {
			i->show();
		} else {
			i->hide();
		}
	}
}

void Home::updateOrientation() {
	Size visibleSize = Director::getInstance()->getVisibleSize();
	_background->setPosition(visibleSize/2);
	for (auto i : _p) {
		i->update();
	}
}

void Home::updateServer() {
	_server = Server::getInstance();
	if (!_server) {
		this->runAction(CallFunc::create([]() {
			Director::getInstance()->popScene();
		}));
	}
}

void Home::onEnter() {
	Scene::onEnter();
	this->updateOrientation();
	this->updateServer();
}