#pragma once

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <cstdlib>
#include <cassert>

#include "../../Core/include/GBCore.h"

const short MINECRAFT_PORT_NUMBER = 25565;

using namespace GenericBoson;

class TestClient : public Core
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

	template<typename STRING>
	void InscribeStringToBuffer(const STRING& str, GBBuffer* pGbBuffer);

private: void ConsumeGatheredMessage(ExpandedOverlapped* pEol, char* message, const uint32_t messageSize, int& readOffSet) override {}
private: void* GetSessionInformationArray() override { return nullptr; }
};