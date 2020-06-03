#include "HomeScene.h"

Scene* Home::createScene() {
    return Home::create();
}

bool Home::init() {
    if (!Scene::init()) {
        return false;
    }

    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
	
	auto background = Sprite::create("background.png");
	background->setPosition(visibleSize/2);
	this->addChild(background);
	
	_server = socket(AF_INET, SOCK_STREAM, 0);
	_serverAddr.sin_family = AF_INET; 
	_serverAddr.sin_port = htons(PORT); 
	_serverAddr.sin_addr.s_addr = inet_addr(IP.c_str());
	if (connect(_server, (struct sockaddr*) &_serverAddr, sizeof(_serverAddr)) < 0) {	//add a timeout???
		log("Connection Failed");
		this->runAction(CallFunc::create([&](){
							Director::getInstance()->popScene();
						}));
	}
	
	_rows = UserDefault::getInstance()->getIntegerForKey("rows", 3);
	_columns = UserDefault::getInstance()->getIntegerForKey("columns", 4);
	_pages = UserDefault::getInstance()->getIntegerForKey("pages", 1);
	_currentPage = 1;
	
	for (int k=1; k<=_pages; k++) {
		auto tmp = std::unique_ptr<Page>(new Page(this, _server, k, _rows, _columns));
		_p.push_back(std::move(tmp));
	}
	_left = _p[_currentPage-1]->getLeft();
	_right = _p[_currentPage-1]->getRight();
	
	auto touchListener = EventListenerTouchOneByOne::create();
	
	touchListener->onTouchBegan = [](Touch* touch, Event* event){
		return true;
	};
	
	touchListener->onTouchEnded = [&](Touch* touch, Event* event) {
		if (touch->getLocation().x < _left) {
			changePage((_currentPage-1+_pages)%_pages ? (_currentPage-1+_pages)%_pages : _pages );
		} else if (touch->getLocation().x > _right) {
			changePage((_currentPage+1)%_pages ? (_currentPage+1)%_pages : _pages);
		}
	};

	_eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, this);
	
	
	std::thread serv([&](){
		while (true) {
			auto task = RECV();
			log("%s", task.c_str());
			if (task == "image") {
				auto id = stoi(RECV());
				std::string image = RECV();
				image = base64_decode(image);
				auto path = FileUtils::getInstance()->getWritablePath();
				FileUtils::getInstance()->addSearchPath(path, true);
				FileUtils::getInstance()->writeStringToFile(image, path+std::to_string(id)+".png");
				Director::getInstance()->getTextureCache()->removeTextureForKey(std::to_string(id)+".png");
				Director::getInstance()->getScheduler()->performFunctionInCocosThread([&](){
					_p[id/TOTAL]->sync(id);
				});
			} else if (task == "background") {
				std::string image = RECV();
				image = base64_decode(image);
				auto path = FileUtils::getInstance()->getWritablePath();
				FileUtils::getInstance()->addSearchPath(path, true);
				FileUtils::getInstance()->writeStringToFile(image, path+"background.png");
				Director::getInstance()->getTextureCache()->removeTextureForKey("background.png");
				Director::getInstance()->getScheduler()->performFunctionInCocosThread([&](){
					background->setTexture("background.png");
				});
			} else if (task == "grid") {
				_pages = stoi(RECV());
				_rows = stoi(RECV());
				_columns = stoi(RECV());
				UserDefault::getInstance()->setIntegerForKey("rows", _rows);
				UserDefault::getInstance()->setIntegerForKey("columns", _columns);
				UserDefault::getInstance()->setIntegerForKey("pages", _pages);
				
				Director::getInstance()->getScheduler()->performFunctionInCocosThread([&](){
					for (auto &i : _p) {
						i->update();
					}
					if (_pages > _p.size()) {
						for (int k=_p.size()+1; k<=_pages; k++) {
							auto tmp = std::unique_ptr<Page>(new Page(this, _server, k, _rows, _columns));
							_p.push_back(std::move(tmp));
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
	for (auto &i: _p) {
		if (i->getPage() == page) {
			i->show();
		} else {
			i->hide();
		}
	}
}

void Home::SEND(const char* msg) {
	send(_server, msg, strlen(msg), 0);
	log("%s", msg);
}

std::string Home::RECV() {
	std::string msg = "";
	log("MSG START");
	while (!endsWith(msg, "EOFEOFEOFEOFEOFEOFEOFEOFXXX")) {
		std::vector<char> buf(4096);
		recv(_server, buf.data(), buf.size(), 0);
		msg += std::string(buf.begin(), buf.end()).c_str();
	}
	log("DONE");
	return msg.substr(0, msg.size()-27);
	// add something that deals with server crash
}

bool Home::endsWith(std::string fullString, std::string const &ending) {
    fullString = fullString.c_str();
    if (fullString.size() >= ending.size()) {
        return (0 == fullString.compare(fullString.size() - ending.size(), ending.size(), ending));
    } else {
        return false;
    }
}