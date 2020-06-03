#ifndef __HOME_SCENE_H__
#define __HOME_SCENE_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"

#include "TrackpadScene.h"
#include "KeyboardScene.h"
#include "NumpadScene.h"

#include "Page.h"
#include "base64.h"

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
	std::string IP = UserDefault::getInstance()->getStringForKey("ip");
	const int PORT = 1235;

	int _server;
	sockaddr_in _serverAddr;
	
	int _rows;
	int _columns;
	int _pages;
	int _currentPage;
	float _left;
	float _right;
	
	std::vector<std::unique_ptr<Page>> _p;
	
	void changePage(int page);
	
	void SEND(const char* msg);
	//std::string RECV(int size);
	std::string RECV();		//test
	bool endsWith(std::string fullString, std::string const &ending);
};

#endif // __HOME_SCENE_H__