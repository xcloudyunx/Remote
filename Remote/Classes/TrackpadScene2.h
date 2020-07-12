#ifndef __TRACKPAD_SCENE2_H__
#define __TRACKPAD_SCENE2_H__

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
	
	std::string _state;
	std::vector<Touch*> _touches;
	
	int _num = 0;
	
	Vec2 _midPoint;
	float _zoomThreshold;
	float _showThreshold;
	
	virtual bool onTouchBegan(Touch* touch, Event* event);
	virtual void onTouchMoved(Touch* touch, Event* event);
	virtual void onTouchEnded(Touch* touch, Event* event);
	
	void move(Vec2 deltaPos);
	void scroll(Vec2 deltaPos);
	void zoom(Vec2 deltaPos, Vec2 newPos, Vec2 oldPos);
	void altTab(Vec2 deltaPos);
	
	void SEND(const char* msg);
};

#endif // __TRACKPAD_SCENE2_H__