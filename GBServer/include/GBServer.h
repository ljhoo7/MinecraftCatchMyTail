#pragma once

#include "../../Core/include/GBCore.h"
#include "Character.h"
#include "World.h"

namespace GenericBoson
{
	class Server : public Core
	{
	public: World m_world;

	private: struct SessionInfomation : public ExpandedOverlapped
	{
		Character m_controllableCharacter;

		GBUUID m_uuid;
		std::string m_userName;

		SessionState m_sessionState = SessionState::start;

		short m_protocolVersion = 0;
	};

	private: SessionInfomation m_sessionInfoArray[Core::EXTENDED_OVERLAPPED_ARRAY_SIZE];

	public: void SendStartCompress(SessionInfomation*);
	public: void SendLoginSuccess(SessionInfomation*);
	public: void SendJoinGame(SessionInfomation*);
	public: void SendSpawnSpot(SessionInfomation*);
	public: void SendDifficulty(SessionInfomation*);
	public: void SendCharacterAbility(SessionInfomation*);
	public: void SendTime(SessionInfomation*);
	public: void SendInventory(SessionInfomation*);
	public: void SendHealth(SessionInfomation*);

	public: void SendExperience(SessionInfomation*);
	public: void SendEquippedItem(SessionInfomation*);
	public: void SendPlayerList(SessionInfomation*);
	public: void SendStatistics(SessionInfomation*);

	private: void ConsumeGatheredMessage(ExpandedOverlapped* pEol, char* message, const uint32_t messageSize, int& readOffSet) override;
	private: void* GetSessionInformationArray() override;

	protected: template<typename FUNCTION> void MakeAndSendPacket(SessionInfomation* pSi, const FUNCTION& func)
	{
		pSi->m_writeBuffer.Reset();

		char* pPacketLength = AssignFromBufferForWrite<char>(&pSi->m_writeBuffer);

		func(pSi);

		assert(pSi->m_writeBuffer.m_writeOffset < 257);

		*pPacketLength = (char)(pSi->m_writeBuffer.m_writeOffset - 1);

		EnqueueAndIssueSend(pSi);
	}
	};
}