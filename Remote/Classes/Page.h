#ifndef __PAGE_H__
#define __PAGE_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"

#include "HomeScene.h"

#include <string>
#include <thread>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h>

using namespace cocos2d;
using namespace ui;

#define ROWS 3
#define COLS 4
#define TOTAL 12

class Page {
public:
	Page(Scene* s, int server, int p, int &r, int &c) {
		_this = s;
		_server = server;
		
		_page = p;
		_rows = &r;
		_columns = &c;
		
		for (int i=0; i<ROWS; i++) {
			for (int j=0; j<COLS; j++) {
				Button* btn = Button::create();
				if (FileUtils::getInstance()->isFileExist(std::to_string((p-1)*TOTAL+i*COLS+j)+".png")) {
					btn->loadTextureNormal(std::to_string((p-1)*TOTAL+i*COLS+j)+".png");
				} else {
					btn->loadTextureNormal("default.png");
				}
				btn->setAnchorPoint(Vec2(0, 1));
				btn->setName(std::to_string((p-1)*TOTAL+i*COLS+j));
				btn->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type){
					switch (type) {
						case Widget::TouchEventType::BEGAN:
							break;
						case Widget::TouchEventType::CANCELED:
							break;
						case Widget::TouchEventType::ENDED:
							SEND(dynamic_cast<Button*>(sender)->getName().c_str());
							break;
						default:
							break;
					}
				});
				_this->addChild(btn);
				if (p != 1) {
					btn->setVisible(false);
				}
				_icons.push_back(btn);
			}
		}
		update();
	}
	
	void update() {
		float scale = std::min((_visibleSize.width-4*_size)/((*_columns)*_size*2-_size), (_visibleSize.height-2*_size)/((*_rows)*_size*2-_size));
		float xpadding = (_visibleSize.width - ((*_columns)*_size*2-_size)*scale)/2;
		float ypadding = _visibleSize.height - (_visibleSize.height - ((*_rows)*_size*2-_size)*scale)/2;
		_padding = xpadding;
		
		for (int i=0; i<*_rows; i++) {
			for (int j=0; j<*_columns; j++) {
				_icons[i*COLS+j]->setScale(scale);
				_icons[i*COLS+j]->setPosition(Vec2(xpadding+_size*2*scale*j, ypadding-_size*2*scale*i));
			}
		}
	}
	
	void sync(int id) {
		_icons[id%TOTAL]->loadTextureNormal(std::to_string(id)+".png");
	}
	
	void show() {
		for (int i=0; i<ROWS; i++) {
			for (int j=0; j<COLS; j++) {
				if (i < (*_rows) && j < (*_columns)) {
					_icons[i*COLS+j]->setVisible(true);
				} else {
					_icons[i*COLS+j]->setVisible(false);
				}
			}
		}
	}
	
	void hide() {
		for (int i=0; i<ROWS; i++) {
			for (int j=0; j<COLS; j++) {
				_icons[i*COLS+j]->setVisible(false);
			}
		}
	}
	
	int getPage() {
		return _page;
	}
	
	float getLeft() {
		return _padding;
	}
	
	float getRight() {
		return _visibleSize.width-_padding;
	}
private:
	Size _visibleSize = Director::getInstance()->getVisibleSize();

	Scene* _this;
	
	int _server;
	
	int _size = 100;
	
	float _padding;

	int* _rows;
	int* _columns;
	int _page;
	std::vector<Button*> _icons;
	
	void SEND(const char* msg) {
		send(_server, msg, strlen(msg), 0);
		log("%s", msg);
	}

	std::string RECV() {
		std::string msg = "";
		log("MSG START");
		while (!endsWith(msg, "EOFEOFEOFEOFEOFEOFEOFEOFXXX")) {
			std::vector<char> buf(4096);
			recv(_server, buf.data(), buf.size(), 0);
			msg += std::string(buf.begin(), buf.end()).c_str();
		}
		log("DONE");
		return msg.substr(0, msg.size()-27);
	}
	
	bool endsWith(std::string fullString, std::string const &ending) {
		fullString = fullString.c_str();
		if (fullString.size() >= ending.size()) {
			return (0 == fullString.compare(fullString.size() - ending.size(), ending.size(), ending));
		} else {
			return false;
		}
	}
};

#endif // __PAGE_H__
