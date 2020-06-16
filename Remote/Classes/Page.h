#ifndef __PAGE_H__
#define __PAGE_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"

#include "Server.h"

using namespace cocos2d;
using namespace ui;

#define ROWS 3
#define COLS 4
#define TOTAL 12

class Page : public Node {
public:
	static Page* create(int p, int &r, int &c);

	virtual bool init(int p, int &r, int &c)
	
	void update();
	
	void sync(int id);
	
	void show();
	void hide();
	
	int getPage();
	
	float getLeft();
	float getRight();
private:
	Size _visibleSize;
	
	Server* _server;
	
	int _size;
	
	float _padding;

	int* _rows;
	int* _columns;
	int _page;
	std::vector<Button*> _icons;
};

#endif // __PAGE_H__
