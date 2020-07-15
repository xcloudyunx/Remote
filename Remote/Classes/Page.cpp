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
	_server = Server::getInstance();
	
	_size = 100;
	
	_page = p;
	_rows = &r;
	_columns = &c;
	
	for (int i=0; i<ROWS; i++) {
		for (int j=0; j<COLS; j++) {
			auto btn = Button::create("buttonNormal.png", "buttonSelected.png");
			auto icon = Sprite::create();
			icon->setName("icon");
			icon->setAnchorPoint(Vec2(0.5f, 0.43f));
			icon->setPosition(btn->getContentSize()/2);
			btn->addChild(icon);
			if (FileUtils::getInstance()->isFileExist(std::to_string((p-1)*TOTAL+i*COLS+j)+".png")) {
				icon->setTexture(std::to_string((p-1)*TOTAL+i*COLS+j)+".png");
			}
			btn->setName(std::to_string((p-1)*TOTAL+i*COLS+j));
			btn->addTouchEventListener([&](Ref* sender, Widget::TouchEventType type){
				switch (type) {
					case Widget::TouchEventType::BEGAN:
						dynamic_cast<Node*>(dynamic_cast<Node*>(sender)->getChildByName("icon"))->setAnchorPoint(Vec2(0.5f, 0.5f));
						break;
					case Widget::TouchEventType::CANCELED:
						dynamic_cast<Node*>(dynamic_cast<Node*>(sender)->getChildByName("icon"))->setAnchorPoint(Vec2(0.5f, 0.43f));
						break;
					case Widget::TouchEventType::ENDED:
						dynamic_cast<Node*>(dynamic_cast<Node*>(sender)->getChildByName("icon"))->setAnchorPoint(Vec2(0.5f, 0.43f));
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
	
	return true;
}

void Page::update() {
	Size visibleSize = Director::getInstance()->getVisibleSize();
	
	if (visibleSize.width >= visibleSize.height) {
		float scale = std::min((visibleSize.width-2*_size)/((*_columns)*_size*2-_size), (visibleSize.height-2*_size)/((*_rows)*_size*2-_size));
		float xpadding = (visibleSize.width - ((*_columns)*_size*2-_size)*scale)/2;
		float ypadding = visibleSize.height - (visibleSize.height - ((*_rows)*_size*2-_size)*scale)/2;
		_padding = xpadding;
		
		for (int i=0; i<ROWS; i++) {
			for (int j=0; j<COLS; j++) {
				if (i < *_rows && j < *_columns) {
					_icons[i*COLS+j]->setScale(scale);
					_icons[i*COLS+j]->setPosition(Vec2(xpadding + _size*scale/2 + _size*2*scale*j, ypadding - _size*scale/2 - _size*2*scale*i));
					_icons[i*COLS+j]->setVisible(true);
				} else {
					_icons[i*COLS+j]->setVisible(false);
				}
			}
		}
	} else {
		float scale = std::min((visibleSize.width-2*_size)/((*_rows)*_size*2-_size), (visibleSize.height-2*_size)/((*_columns)*_size*2-_size));
		float xpadding = (visibleSize.width - ((*_rows)*_size*2-_size)*scale)/2;
		float ypadding = visibleSize.height - (visibleSize.height - ((*_columns)*_size*2-_size)*scale)/2;
		_padding = xpadding;
		
		for (int i=0; i<ROWS; i++) {
			for (int j=0; j<COLS; j++) {
				if (i < *_rows && j < *_columns) {
					_icons[i*COLS+j]->setScale(scale);
					_icons[i*COLS+j]->setPosition(Vec2(visibleSize.width - xpadding - _size*scale/2 - _size*2*scale*i, ypadding - _size*scale/2 - _size*2*scale*j));
					_icons[i*COLS+j]->setVisible(true);
				} else {
					_icons[i*COLS+j]->setVisible(false);
				}
			}
		}
	}
}

void Page::sync(int id) {
	dynamic_cast<Sprite*>(_icons[id%TOTAL]->getChildByName("icon"))->setTexture(std::to_string(id)+".png");
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
	Size visibleSize = Director::getInstance()->getVisibleSize();
	return visibleSize.width-_padding;
}