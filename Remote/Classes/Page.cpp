#include "Page.h"

Page* Page::create(int p, int &r, int &c) {
	Page *page = new (std::nothrow) Page();
	if (page && page->init(p, r, c)) {
		page->autorelease();
		return page;
	}
	CC_SAFE_DELETE(page);

	return nullptr;
}

bool Page::init(int p, int &r, int &c) {
	_visibleSize = Director::getInstance()->getVisibleSize();
	
	_server = Server::getInstance();
	
	_size = 100;
	
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
						_server->SEND(dynamic_cast<Button*>(sender)->getName().c_str());
						break;
					default:
						break;
				}
			});
			this->addChild(btn);
			if (p != 1) {
				btn->setVisible(false);
			}
			_icons.push_back(btn);
		}
	}
	update();
}

void Page::update() {
	float scale = std::min((_visibleSize.width-4*_size)/((*_columns)*_size*2-_size), (_visibleSize.height-2*_size)/((*_rows)*_size*2-_size));
	float xpadding = (_visibleSize.width - ((*_columns)*_size*2-_size)*scale)/2;
	float ypadding = _visibleSize.height - (_visibleSize.height - ((*_rows)*_size*2-_size)*scale)/2;
	_padding = xpadding;
	
	for (int i=0; i<ROWS; i++) {
		for (int j=0; j<COLS; j++) {
			if (i < *_rows && j < *_columns) {
				_icons[i*COLS+j]->setScale(scale);
				_icons[i*COLS+j]->setPosition(Vec2(xpadding+_size*2*scale*j, ypadding-_size*2*scale*i));
				_icons[i*COLS+j]->setVisible(true);
			} else {
				_icons[i*COLS+j]->setVisible(false);
			}
		}
	}
}

void Page::sync(int id) {
	_icons[id%TOTAL]->loadTextureNormal(std::to_string(id)+".png");
}

void Page::show() {
	this->setVisible(true);
}

void Page::hide() {
	this->setVisible(false);
}

int Page::getPage() {
	return _page;
}

float Page::getLeft() {
	return _padding;
}

float Page::getRight() {
	return _visibleSize.width-_padding;
}