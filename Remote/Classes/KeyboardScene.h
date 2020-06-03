#ifndef __KEYBOARD_SCENE_H__
#define __KEYBOARD_SCENE_H__

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

class Keyboard : public Scene, public EditBoxDelegate {
public:
	static Scene* createScene();

	virtual bool init();

    CREATE_FUNC(Keyboard);
	
	virtual void editBoxTextChanged(EditBox* editbox, const std::string &text);
	virtual void editBoxReturn(EditBox* editbox);
	
private:
	std::string IP = UserDefault::getInstance()->getStringForKey("ip");
	const int PORT = 1235;

	int _server;
	sockaddr_in _serverAddr;
	
	void SEND(const char* msg);
};

#endif // __KEYBOARD_SCENE_H__