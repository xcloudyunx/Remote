#ifndef __SERVER_H__
#define __SERVER_H__

#include "cocos2d.h"

#include <string>
#include <thread>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/time.h>

using namespace cocos2d;

class Server : public Ref {
public:
	static void Reset();
	static Server* getInstance();
	
	bool init();
	
	void end();
	
	void SEND(const char* msg);
	std::string RECV();


private:
	int _server;
	
	bool endsWith(std::string fullString, std::string const &ending);
};

#endif // __SERVER_H__
