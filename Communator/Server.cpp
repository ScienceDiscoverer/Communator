#include "Server.h"

// Server implementation
void Server::Init(HWND hwnd)
{
	int res;

	addrinfo hints;
	addrinfo* result;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	res = getaddrinfo(NULL, port.c_str(), &hints, &result);
	if(res != 0)
	{
		MessageBox(NULL, ("getaddrinfo() failed! Error: " + to_string(res)).c_str(), "Error", MB_OK | MB_ICONERROR);
		WSACleanup();
		return;
	}

	listen_sock_ = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if(listen_sock_ == INVALID_SOCKET)
	{
		MessageBox(NULL, ("socket() failed! Error: " + to_string(WSAGetLastError())).c_str(), "Error", MB_OK | MB_ICONERROR);
		freeaddrinfo(result);
		WSACleanup();
		return;
	}

	res = bind(listen_sock_, result->ai_addr, (int)result->ai_addrlen);
	if(res != 0)
	{
		MessageBox(NULL, ("getaddrinfo() failed! Error: " + to_string(res)).c_str(), "Error", MB_OK | MB_ICONERROR);
		freeaddrinfo(result);
		closesocket(listen_sock_);
		WSACleanup();
		return;
	}
	freeaddrinfo(result);

	res = listen(listen_sock_, SOMAXCONN);
	if(res == SOCKET_ERROR)
	{
		MessageBox(NULL, ("listen() failed! Error: " + to_string(res)).c_str(), "Error", MB_OK | MB_ICONERROR);
		closesocket(listen_sock_);
		WSACleanup();
	}

	res = WSAAsyncSelect(listen_sock_, hwnd, NET_EVENT, FD_ACCEPT | FD_CONNECT | FD_READ | FD_CLOSE);
	if(res == SOCKET_ERROR)
	{
		MessageBox(NULL, ("WSAAsyncSelect() failed! Error: " + to_string(res)).c_str(), "Error", MB_OK | MB_ICONERROR);
		closesocket(listen_sock_);
		WSACleanup();
	}
}

void Server::Accept()
{
	client_sock_ = accept(listen_sock_, NULL, NULL);
	if(client_sock_ == INVALID_SOCKET)
	{
		MessageBox(NULL, ("accept() failed! Error: " + to_string(WSAGetLastError())).c_str(), "Error", MB_OK | MB_ICONERROR);
		closesocket(listen_sock_);
		WSACleanup();
		return;
	}
	closesocket(listen_sock_);
}

Readings Server::Read()
{
	int res;
	char buff[buff_len];
	Readings out;

	res = recv(client_sock_, buff, buff_len, 0);
	if(res > 0)
	{
		out.ele_1 = ((int*)buff)[0];
		out.ele_2 = ((int*)buff)[1];
		out.gas = ((int*)buff)[2];
		out.water = ((int*)buff)[3];
	}
	else if(res < 0)
	{
		MessageBox(NULL, ("recv() failed! Error: " + to_string(WSAGetLastError())).c_str(), "Error", MB_OK | MB_ICONERROR);
		closesocket(client_sock_);
		WSACleanup();
	}

	res = shutdown(client_sock_, SD_SEND);
	if(res == SOCKET_ERROR)
	{
		MessageBox(NULL, ("shutdown() failed! Error: " + to_string(WSAGetLastError())).c_str(), "Error", MB_OK | MB_ICONERROR);
		closesocket(client_sock_);
		WSACleanup();
	}

	return out;
}

void Server::Shutdown()
{
	closesocket(client_sock_);
}
// End of Server implementation