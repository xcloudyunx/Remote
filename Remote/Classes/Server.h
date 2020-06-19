#ifndef __SERVER_H__
#define __SERVER_H__

#include "cocos2d.h"

using namespace cocos2d;

class Server : public Ref {
public:
	static void Reset();
	static Server* getInstance();
	
	bool init();
	
	void SEND(const char* msg);
	std::string RECV();


private:
	int _server;
	
	bool endsWith(std::string fullString, std::string const &ending);
};

#endif // __SERVER_H__
