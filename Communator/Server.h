#ifndef SERVER_H
#define SERVER_H

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>
#include "Month.h"
#include "Defines.h"

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

const string port = "42069";
const int buff_len = 16;

class Server
{
public:
	Server() : listen_sock_(INVALID_SOCKET), client_sock_(INVALID_SOCKET) {}

	void Init(HWND hwnd);
	void Accept();
	Readings Read();
	void Shutdown();

//private:	

	//WSADATA wsa_;
	SOCKET listen_sock_;
	SOCKET client_sock_;
};

#endif /* SERVER_H */