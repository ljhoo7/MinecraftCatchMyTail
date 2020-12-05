#include "GBServer.h"

namespace GenericBoson
{
	void Server::SendStartCompress(SessionInfomation* pSi)
	{
		WriteByteByByte(&pSi->m_writeBuffer, (int32_t)PacketType::StartCompression);
		WriteByteByByte(&pSi->m_writeBuffer, InternalConstant::CompressThreshold);

		EnqueueAndIssueSend(pSi);
	}

	void Server::SendLoginSuccess(SessionInfomation* pSi)
	{
		WriteByteByByte(&pSi->m_writeBuffer, (int32_t)PacketType::LoginSuccess);

		// UUID #ToDo
		pSi->m_uuid = "5550AEA5-0443-4C06-A1CB-CF916EA1623D";
		WriteString(&pSi->m_writeBuffer, pSi->m_uuid);
		WriteString(&pSi->m_writeBuffer, pSi->m_userName);

		EnqueueAndIssueSend(pSi);
	}

	void Server::SendJoinGame(SessionInfomation* pSi)
	{
		WriteByteByByte(&pSi->m_writeBuffer, (int32_t)PacketType::JoinGame);

		// Sending FermionID
		Write4BytesAsBigEndian(&pSi->m_writeBuffer, pSi->m_controllableCharacter.m_ID);

		uint8_t hardCoreFlag = 0;
		WriteByteByByte(&pSi->m_writeBuffer, hardCoreFlag);

		Dimension demension = Dimension::overworld;
		Write4BytesAsBigEndian(&pSi->m_writeBuffer, demension);

		uint8_t difficulty = 2; // 2 = Normal
		WriteByteByByte(&pSi->m_writeBuffer, difficulty);

		uint8_t maxPlayerCount = 255;
		WriteByteByByte(&pSi->m_writeBuffer, maxPlayerCount);

		std::string levelType = "default";
		WriteString(&pSi->m_writeBuffer, levelType);

		uint8_t reducedDebugInfo = 0; // bool
		WriteByteByByte(&pSi->m_writeBuffer, reducedDebugInfo);

		EnqueueAndIssueSend(pSi);
	}

	void Server::SendSpawnSpot(SessionInfomation* pSi)
	{
		WriteByteByByte(&pSi->m_writeBuffer, (int32_t)PacketType::SpawnSpot);

		GBVector3<int> spawnSpot(10, 10, 10);
		WriteIntGBVector3(&pSi->m_writeBuffer, spawnSpot);

		EnqueueAndIssueSend(pSi);
	}

	void Server::SendDifficulty(SessionInfomation* pSi)
	{
		WriteByteByByte(&pSi->m_writeBuffer, (int32_t)PacketType::Difficulty);

		char difficulty = 1;
		WriteByteByByte(&pSi->m_writeBuffer, difficulty);

		EnqueueAndIssueSend(pSi);
	}

	void Server::SendCharacterAbility(SessionInfomation* pSi)
	{
		WriteByteByByte(&pSi->m_writeBuffer, (int32_t)PacketType::CharacterAbility);

		WriteByteByByte(&pSi->m_writeBuffer, pSi->m_controllableCharacter.m_abilityState);

		float correctedFlyingMaxSpeed = 0.05f * pSi->m_controllableCharacter.m_flyingMaxSpeed;
		Write4BytesAsBigEndian(&pSi->m_writeBuffer, correctedFlyingMaxSpeed);

		float correctedSprintingMaxSpeed = 0.05f * pSi->m_controllableCharacter.m_sprintingMaxSpeed;
		Write4BytesAsBigEndian(&pSi->m_writeBuffer, correctedSprintingMaxSpeed);

		EnqueueAndIssueSend(pSi);
	}

	void Server::SendTime(SessionInfomation* pSi)
	{
		WriteByteByByte(&pSi->m_writeBuffer, (int32_t)PacketType::Time);
		Write8BytesAsBigEndian(&pSi->m_writeBuffer, m_world.m_ageMs);

		// false == m_world.m_dayLightEnabled #ToDo

		Write8BytesAsBigEndian(&pSi->m_writeBuffer, m_world.m_timeOfDay);

		EnqueueAndIssueSend(pSi);
	}

	void Server::SendInventory(SessionInfomation* pSi)
	{
		WriteByteByByte(&pSi->m_writeBuffer, (int32_t)PacketType::Inventory);
		WriteByteByByte(&pSi->m_writeBuffer, pSi->m_controllableCharacter.m_inventory.m_ID);
		WriteByteByByte(&pSi->m_writeBuffer, (int16_t)pSi->m_controllableCharacter.m_inventory.GetTotalSlotCount());

		for (auto& pSlot : pSi->m_controllableCharacter.m_inventory.m_slotVector)
		{
			// #ToDo
			pSlot->WriteItem();
		}

		EnqueueAndIssueSend(pSi);
	}

	void Server::SendHealth(SessionInfomation* pSi)
	{
		WriteByteByByte(&pSi->m_writeBuffer, (int32_t)PacketType::Health);
		Write4BytesAsBigEndian(&pSi->m_writeBuffer, pSi->m_controllableCharacter.m_health);
		WriteByteByByte(&pSi->m_writeBuffer, pSi->m_controllableCharacter.m_foodLevel);
		Write4BytesAsBigEndian(&pSi->m_writeBuffer, pSi->m_controllableCharacter.m_foodSaturationLevel);

		EnqueueAndIssueSend(pSi);
	}

	void Server::SendExperience(SessionInfomation* pSi)
	{
		WriteByteByByte(&pSi->m_writeBuffer, (int32_t)PacketType::Experience);

		float xpPercentage = pSi->m_controllableCharacter.GetXpPercentage();
		Write4BytesAsBigEndian(&pSi->m_writeBuffer, xpPercentage);

		int level = pSi->m_controllableCharacter.GetLevel();
		WriteByteByByte(&pSi->m_writeBuffer, (int32_t)PacketType::Experience);
		WriteByteByByte(&pSi->m_writeBuffer, pSi->m_controllableCharacter.m_experience);

		EnqueueAndIssueSend(pSi);
	}

	void Server::SendEquippedItem(SessionInfomation* pSi)
	{
		WriteByteByByte(&pSi->m_writeBuffer, (int32_t)PacketType::EquippedItemChange);
		WriteByteByByte(&pSi->m_writeBuffer, (int8_t)pSi->m_controllableCharacter.m_inventory.m_equippedSlotID);

		EnqueueAndIssueSend(pSi);
	}

	void Server::SendPlayerList(SessionInfomation* pSi)
	{
		WriteByteByByte(&pSi->m_writeBuffer, (int32_t)PacketType::PlayerList);

		WriteByteByByte(&pSi->m_writeBuffer, (int32_t)0);

		WriteByteByByte(&pSi->m_writeBuffer, (int32_t)1);

		WriteString(&pSi->m_writeBuffer, pSi->m_uuid);

		WriteString(&pSi->m_writeBuffer, pSi->m_userName);

		// #ToDo
		// Send Property List Size
		WriteByteByByte(&pSi->m_writeBuffer, (int32_t)0);

		WriteByteByByte(&pSi->m_writeBuffer, (int32_t)m_world.m_gameMode);

		// #ToDo
		WriteByteByByte(&pSi->m_writeBuffer, (int32_t)m_world.m_pingMs);

		WriteByteByByte(&pSi->m_writeBuffer, (char)0);

		EnqueueAndIssueSend(pSi);
	}

	void* Server::GetSessionInformationArray()
	{
		return (void*)m_sessionInfoArray;
	}

	void Server::ConsumeGatheredMessage(ExpandedOverlapped* pEol, char* message, const uint32_t messageSize, int& readOffSet)
	{
		SessionInfomation* pSi = static_cast<SessionInfomation*>(pEol);

		// Packet Type
		char packetType = 0;
		uint32_t readPacketTypeByteLength = ReadByteByByte(message, packetType);
		readOffSet += readPacketTypeByteLength;
		message += readPacketTypeByteLength;

		switch (pSi->m_sessionState)
		{
		case SessionState::login:
		{
			switch ((PacketType)packetType)
			{
			case PacketType::LoginStart:
			{
				// Server Address
				std::string userName;
				uint32_t rr = ReadString(message, userName);
				readOffSet += rr;
				message += rr;

				pSi->m_userName = userName;

				SendStartCompress(pSi);
				SendLoginSuccess(pSi);
				SendJoinGame(pSi);
				SendSpawnSpot(pSi);
				SendDifficulty(pSi);
				SendCharacterAbility(pSi);

				//SendWeather #ToDo

				SendTime(pSi);
				SendInventory(pSi);
				SendHealth(pSi);
				SendExperience(pSi);
				SendEquippedItem(pSi);
				SendPlayerList(pSi);

				//eol.m_sessionState = authed;
			}
			break;
			default:
				assert(false);
			}
		}
		break;
		case SessionState::start:
		{
			// Protocol Version
			short protocolVersion = 0;
			uint32_t rr1 = ReadByteByByte(message, protocolVersion);
			readOffSet += rr1;
			message += rr1;

			pSi->m_protocolVersion = protocolVersion;

			// Server Address
			std::string serverAddressStr;
			uint32_t rr2 = ReadString(message, serverAddressStr);
			readOffSet += rr2;
			message += rr2;

			// Server Port
			uint16_t portNumber;
			uint32_t rr3 = Read(message, portNumber);
			portNumber = ntohs(portNumber);
			readOffSet += rr3;
			message += rr3;

			// Next Stage
			char nextStage = 0;
			uint32_t rr4 = ReadByteByByte(message, nextStage);
			readOffSet += rr4;
			message += rr4;

			pSi->m_sessionState = (SessionState)nextStage;
		}
		break;
		case SessionState::in_game:
		{
			switch ((PacketType)packetType)
			{
			case PacketType::ClientSettings:
			{
				// Server Address
				std::string localeString;
				uint32_t rr = ReadString(message, localeString);
				readOffSet += rr;
				message += rr;
			}
			break;
			default:
				assert(false);
			}
		}
		break;
		default:
			assert(false);
			break;
		}
	}

}