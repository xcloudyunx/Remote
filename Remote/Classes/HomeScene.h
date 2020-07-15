#ifndef __HOME_SCENE_H__
#define __HOME_SCENE_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"

#include "TrackpadScene.h"
#include "KeyboardScene.h"
#include "NumpadScene.h"

#include "Page.h"
#include "base64.h"
#include "Server.h"

using namespace cocos2d;
using namespace ui;

class Home : public Scene {
public:
	static Scene* createScene();

	virtual bool init();
	
	virtual void onEnter();
	
	void updateOrientation();

    CREATE_FUNC(Home);

private:
	Sprite* _background;

	Server* _server;
	
	int _rows;
	int _columns;
	int _pages;
	int _currentPage;
	float _left;
	float _right;
	
	std::vector<Page*> _p;
	
	void changePage(int page);
};

#endif // __HOME_SCENE_H__