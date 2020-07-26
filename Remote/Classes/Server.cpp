#include "Server.h"

static Server* _instance = nullptr;

Server* Server::getInstance() {
	if (!_instance) {
		_instance = new (std::nothrow) Server;
		if (!_instance->init()) return _instance = nullptr;
	}
	return _instance;
}

bool Server::init() {
	std::string IP = UserDefault::getInstance()->getStringForKey("ip");
	const int PORT = 1235;
	
	int res;
	long arg;
	fd_set fd;
	timeval timeout;
	int valopt;
	socklen_t lon;
	
	_server = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	serverAddr.sin_addr.s_addr = inet_addr(IP.c_str());
	log("Attempting to connect to %s", IP.c_str());
	
	if ((arg = fcntl(_server, F_GETFL, nullptr)) < 0) {
		log("Connection Failed 0");
		UserDefault::getInstance()->setStringForKey("ip", "NEW");
		return false;
	}
	arg |= O_NONBLOCK;
	if (fcntl(_server, F_SETFL, arg) < 0) {
		log("Connection Failed 1");
		UserDefault::getInstance()->setStringForKey("ip", "NEW");
		return false;
	}
	
	res = connect(_server, (sockaddr*) &serverAddr, sizeof(serverAddr));
	
	if (res < 0) {
		if (errno == EINPROGRESS) {
			log("connecting...");
			do {
				timeout.tv_sec = 10;
				timeout.tv_usec = 0;
				FD_ZERO(&fd);
				FD_SET(_server, &fd);
				res = select(_server+1, nullptr, &fd, nullptr, &timeout);
				if (res < 0 && errno != EINTR) {
					log("Connection Failed 2");
					UserDefault::getInstance()->setStringForKey("ip", "NEW");
					return false;
				} else if (res > 0) {
					lon = sizeof(int);
					if (getsockopt(_server, SOL_SOCKET, SO_ERROR, (void*)(&valopt), &lon) < 0) {
						log("Connection Failed 3");
						UserDefault::getInstance()->setStringForKey("ip", "NEW");
						return false;
					}
					if (valopt) {
						log("Connection Failed 4");
						UserDefault::getInstance()->setStringForKey("ip", "NEW");
						return false;
					}
					break;
				} else {
					log("Connection Failed 5");
					UserDefault::getInstance()->setStringForKey("ip", "NEW");
					return false;
				}
			} while (true);
		} else {
			log("Connection Failed 6");
			UserDefault::getInstance()->setStringForKey("ip", "NEW");
			return false;
		}
	}
	
	if ((arg = fcntl(_server, F_GETFL, nullptr)) < 0) {
		log("Connection Failed 7");
		UserDefault::getInstance()->setStringForKey("ip", "NEW");
		return false;
	}
	arg &= (~O_NONBLOCK);
	if (fcntl(_server, F_SETFL, arg) < 0) {
		log("Connection Failed 8");
		UserDefault::getInstance()->setStringForKey("ip", "NEW");
		return false;
	}
	
	log("Connection Succeeded");
	return true;
}

void Server::end() {
	close(_server);
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