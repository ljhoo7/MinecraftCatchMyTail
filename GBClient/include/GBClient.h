#pragma once

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <cstdlib>
#include <cassert>
#include <map>

#include "../../Core/include/GBCore.h"

const short MINECRAFT_PORT_NUMBER = 25565;

using namespace GenericBoson;

class TestClient : public Core
{
private: SOCKET m_clientSocket = INVALID_SOCKET;

private: uint32_t m_recipeID = 0;

private: SessionState m_sessionState = SessionState::start;
public: TestClient() = default;
public: ~TestClient();
public: void Start();

public: template<typename STRING> void InscribeStringToBuffer(const STRING& str, GBBuffer* pGbBuffer);
private: void ClientConsumeGatheredMessage(GBBuffer& buffer, uint32_t receivedMessageSize);
private: void ConsumeGatheredMessage(ExpandedOverlapped* pEol, char* message, const uint32_t messageSize, int& readOffSet) override {}
private: void* GetSessionInformationArray() override { return nullptr; }
private: void GatheringMessage(char* message, uint8_t leftBytesToRecieve);
};