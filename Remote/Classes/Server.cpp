#include "Server.h"

static Server* _instance = nullptr;

Server* Server::getInstance() {
	if (!_instance) {
		_instance = new (std::nothrow) Server;
		if (!_instance->init()) return nullptr;
    }
	return _instance;
}

bool Server::init() {
	std::string IP = UserDefault::getInstance()->getStringForKey("ip");
	const int PORT = 1235;
	
	_server = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET; 
	serverAddr.sin_port = htons(PORT); 
	serverAddr.sin_addr.s_addr = inet_addr(IP.c_str());
	if (connect(_server, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) < 0) {	//add a timeout???
		log("Connection Failed");
		return false;
	}
	log("Connection Succeeded");
	
	return true;
}

void Server::SEND(const char* msg) {
	send(_server, msg, strlen(msg), 0);
	log("%s", msg);
}

std::string Server::RECV() {
	std::string msg = "";
	log("MSG START");
	while (!endsWith(msg, "EOFEOFEOFEOFEOFEOFEOFEOFXXX")) {
		std::vector<char> buf(4096);
		recv(_server, buf.data(), buf.size(), 0);
		msg += std::string(buf.begin(), buf.end()).c_str();
	}
	log("DONE");
	return msg.substr(0, msg.size()-27);
	// add something that deals with server crash
}

bool Server::endsWith(std::string fullString, std::string const &ending) {
    fullString = fullString.c_str();
    if (fullString.size() >= ending.size()) {
        return (0 == fullString.compare(fullString.size() - ending.size(), ending.size(), ending));
    } else {
        return false;
    }
}