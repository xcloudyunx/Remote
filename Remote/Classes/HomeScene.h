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

#include <string>
#include <thread>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 

using namespace cocos2d;
using namespace ui;

class Home : public Scene {
public:
	static Scene* createScene();

	virtual bool init();

    CREATE_FUNC(Home);

private:
	Server* _server;
	
	int _rows;
	int _columns;
	int _pages;
	int _currentPage;
	float _left;
	float _right;
	
	std::vector<Page> _p;
	
	void changePage(int page);
};

#endif // __HOME_SCENE_H__