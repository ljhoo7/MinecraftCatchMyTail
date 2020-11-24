#pragma once

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <cstdlib>
#include <cassert>

const short MINECRAFT_PORT_NUMBER = 25565;

class TestClient
{
	SOCKET m_clientSocket = INVALID_SOCKET;
public:
	TestClient() = default;
	~TestClient()
	{
		closesocket(m_clientSocket);
		WSACleanup();
	}

	void Start();

	template<typename T>
	void Send(const T& param);

	template<typename STRING>
	void SendString(const STRING& str);
};