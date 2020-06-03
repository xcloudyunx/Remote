#ifndef __TRACKPAD_SCENE_H__
#define __TRACKPAD_SCENE_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"

#include <string>
#include <thread>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 

using namespace cocos2d;
using namespace ui;

class Trackpad : public Scene {
public:
	static Scene* createScene();

	virtual bool init();

    CREATE_FUNC(Trackpad);
	
private:
	std::string IP = UserDefault::getInstance()->getStringForKey("ip");
	const int PORT = 1235;

	Size _visibleSize;
	int _server;
	sockaddr_in _serverAddr;
	
	int _touches = 0;
	bool _firstMove = false;
	bool _click = false;
	bool _move = false;
	bool _dragPotential = false;
	bool _drag = false;
	bool _scroll = false;
	bool _zoom = false;
	double _zoomThreshold = 0;
	std::string _triple;
	double _showThreshold = 0;
	std::string _quad;


	void move(const std::vector<Touch*> &touches);
	void SEND(const char* msg);
};

#endif // __TRACKPAD_SCENE_H__