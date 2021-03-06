#include "GBServer.h"

namespace GenericBoson
{
	void Server::SendStartCompress(SessionInfomation* pSi)
	{
		WriteByteByByte(&pSi->m_writeBuffer, (int32_t)PacketType::StartCompression);
		WriteByteByByte(&pSi->m_writeBuffer, InternalConstant::CompressThreshold);
	}

	void Server::SendLoginSuccess(SessionInfomation* pSi)
	{
		WriteByteByByte(&pSi->m_writeBuffer, (int32_t)PacketType::LoginSuccess);

		// UUID #ToDo
		//pSi->m_uuid.FromString(L"5550AEA5-0443-4C06-A1CB-CF916EA1623D");
		//std::string uuidString = pSi->m_uuid.ToString();
		std::string uuidString = "cd219047-4980-3421-86fd-81dfa659fabe";
		WriteString(&pSi->m_writeBuffer, uuidString);
		std::string tmpNameString = "ljhoo7";
		WriteString(&pSi->m_writeBuffer, tmpNameString);//pSi->m_userName);
	}

	void Server::SendJoinGame(SessionInfomation* pSi)
	{
		WriteByteByByte(&pSi->m_writeBuffer, (int32_t)PacketType::JoinGame);

		// Sending FermionID
		Write4BytesAsBigEndian_Without_Sign(&pSi->m_writeBuffer, pSi->m_controllableCharacter.m_ID);

		uint8_t hardCoreFlag = 0;
		WriteByteByByte(&pSi->m_writeBuffer, hardCoreFlag);

		Dimension demension = Dimension::overworld;
		Write4BytesAsBigEndian_Without_Sign(&pSi->m_writeBuffer, demension);

		uint8_t difficulty = 2; // 2 = Normal
		WriteByteByByte(&pSi->m_writeBuffer, difficulty);

		uint8_t maxPlayerCount = 255;
		WriteByteByByte(&pSi->m_writeBuffer, maxPlayerCount);

		std::string levelType = "default";
		WriteString(&pSi->m_writeBuffer, levelType);

		uint8_t reducedDebugInfo = 0; // bool
		WriteByteByByte(&pSi->m_writeBuffer, reducedDebugInfo);
	}

	void Server::SendSpawnSpot(SessionInfomation* pSi)
	{
		WriteByteByByte(&pSi->m_writeBuffer, (int32_t)PacketType::SpawnSpot);

		GBVector3<int> spawnSpot(-16, 71, 0);
		WriteIntGBVector3(&pSi->m_writeBuffer, spawnSpot);
	}

	void Server::SendDifficulty(SessionInfomation* pSi)
	{
		WriteByteByByte(&pSi->m_writeBuffer, (int32_t)PacketType::Difficulty);

		char difficulty = 1;
		WriteByteByByte(&pSi->m_writeBuffer, difficulty);
	}

	void Server::SendCharacterAbility(SessionInfomation* pSi)
	{
		WriteByteByByte(&pSi->m_writeBuffer, (int32_t)PacketType::CharacterAbility);

		WriteByteByByte(&pSi->m_writeBuffer, pSi->m_controllableCharacter.m_abilityState);

		float correctedFlyingMaxSpeed = 0.05f * pSi->m_controllableCharacter.m_flyingMaxSpeed;
		WriteFloat(&pSi->m_writeBuffer, correctedFlyingMaxSpeed);

		float correctedSprintingMaxSpeed = 0.05f * pSi->m_controllableCharacter.m_sprintingMaxSpeed;
		WriteFloat(&pSi->m_writeBuffer, correctedSprintingMaxSpeed);
	}

	void Server::SendTime(SessionInfomation* pSi)
	{
		WriteByteByByte(&pSi->m_writeBuffer, (int32_t)PacketType::Time);
		Write8BytesAsBigEndian_Signed(&pSi->m_writeBuffer, m_world.m_ageMs);

		// false == m_world.m_dayLightEnabled #ToDo

		Write8BytesAsBigEndian_Signed(&pSi->m_writeBuffer, m_world.m_timeOfDay);
	}

	void Server::SendInventory(SessionInfomation* pSi)
	{
		WriteByteByByte(&pSi->m_writeBuffer, (int32_t)PacketType::Inventory);
		WriteByteByByte(&pSi->m_writeBuffer, pSi->m_controllableCharacter.m_inventory.m_ID);
		int32_t slotCount = pSi->m_controllableCharacter.m_inventory.GetTotalSlotCount();
		Write2BytesAsBigEndian_Signed(&pSi->m_writeBuffer, slotCount);

		for (auto& pSlot : pSi->m_controllableCharacter.m_inventory.m_slotVector)
		{
			// #ToDo
			pSlot->WriteItem();
		}
	}

	void Server::SendHealth(SessionInfomation* pSi)
	{
		WriteByteByByte(&pSi->m_writeBuffer, (int32_t)PacketType::Health);
		WriteFloat(&pSi->m_writeBuffer, pSi->m_controllableCharacter.m_health);
		WriteByteByByte(&pSi->m_writeBuffer, pSi->m_controllableCharacter.m_foodLevel);
		WriteFloat(&pSi->m_writeBuffer, pSi->m_controllableCharacter.m_foodSaturationLevel);
	}

	void Server::SendExperience(SessionInfomation* pSi)
	{
		WriteByteByByte(&pSi->m_writeBuffer, (int32_t)PacketType::Experience);

		float xpPercentage = pSi->m_controllableCharacter.GetXpPercentage();
		WriteFloat(&pSi->m_writeBuffer, xpPercentage);

		int level = pSi->m_controllableCharacter.GetLevel();
		WriteByteByByte(&pSi->m_writeBuffer, level);
		WriteByteByByte(&pSi->m_writeBuffer, pSi->m_controllableCharacter.m_experience);
	}

	void Server::SendEquippedItem(SessionInfomation* pSi)
	{
		WriteByteByByte(&pSi->m_writeBuffer, (int32_t)PacketType::EquippedItemChange);
		WriteByteByByte(&pSi->m_writeBuffer, (int8_t)pSi->m_controllableCharacter.m_inventory.m_equippedSlotID);
	}

	void Server::SendPlayerList(SessionInfomation* pSi)
	{
		WriteByteByByte(&pSi->m_writeBuffer, (int32_t)PacketType::PlayerList);

		WriteByteByByte(&pSi->m_writeBuffer, (int32_t)0);

		WriteByteByByte(&pSi->m_writeBuffer, (int32_t)1);

		WriteUUID(&pSi->m_writeBuffer, pSi->m_uuid);

		WriteString(&pSi->m_writeBuffer, pSi->m_userName);

		// #ToDo
		// Send Property List Size
		WriteByteByByte(&pSi->m_writeBuffer, (int32_t)0);

		WriteByteByByte(&pSi->m_writeBuffer, (int32_t)m_world.m_gameMode);

		// #ToDo
		WriteByteByByte(&pSi->m_writeBuffer, (int32_t)m_world.m_pingMs);

		WriteByteByByte(&pSi->m_writeBuffer, (char)0);
	}

	void Server::SendStatistics(SessionInfomation* pSi)
	{
		// #ToDo
		// Send storeSizeSum Size
		WriteByteByByte(&pSi->m_writeBuffer, (int32_t)0);

		// size
		WriteByteByByte(&pSi->m_writeBuffer, (int32_t)5);

		std::string statics1 = "stat.timeSinceDeath";
		WriteString(&pSi->m_writeBuffer, statics1);
		WriteByteByByte(&pSi->m_writeBuffer, (int32_t)107);

		std::string statics2 = "stat.drop";
		WriteString(&pSi->m_writeBuffer, statics2);
		WriteByteByByte(&pSi->m_writeBuffer, (int32_t)0);

		std::string statics3 = "stat.leaveGame";
		WriteString(&pSi->m_writeBuffer, statics3);
		WriteByteByByte(&pSi->m_writeBuffer, (int32_t)11);

		std::string statics4 = "stat.playOneMinute";
		WriteString(&pSi->m_writeBuffer, statics4);
		WriteByteByByte(&pSi->m_writeBuffer, (int32_t)107);

		std::string statics5 = "stat.walkOneCm";
		WriteString(&pSi->m_writeBuffer, statics5);
		WriteByteByByte(&pSi->m_writeBuffer, (int32_t)0);
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
		uint32_t rr = ReadByteByByte(message, packetType);
		readOffSet += rr;
		message += rr;

		SessionState ss = pSi->m_sessionState;
		PacketType pt = (PacketType)packetType;
		if (SessionState::login == ss)
		{
			if (PacketType::LoginStart == pt)
			{
				// Server Address
				std::string userName;
				rr = ReadString(message, userName);
				readOffSet += rr;
				message += rr;

				pSi->m_userName = userName;

				ss = SessionState::in_game;

				MakeAndSendPacket(pSi, [this](SessionInfomation* pSi)
				{
					int prevWriteOffset = 0;

					AppendSizeAtFront(pSi, [this](SessionInfomation* pSi)
					{SendStartCompress(pSi);}, prevWriteOffset);

					AppendSizeAtFront(pSi, [this](SessionInfomation* pSi)
					{SendLoginSuccess(pSi);}, prevWriteOffset);

					AppendSizeAtFront(pSi, [this](SessionInfomation* pSi)
					{SendJoinGame(pSi); }, prevWriteOffset);

					AppendSizeAtFront(pSi, [this](SessionInfomation* pSi)
					{SendSpawnSpot(pSi); }, prevWriteOffset);

					AppendSizeAtFront(pSi, [this](SessionInfomation* pSi)
					{SendDifficulty(pSi); }, prevWriteOffset);

					AppendSizeAtFront(pSi, [this](SessionInfomation* pSi)
					{SendCharacterAbility(pSi); }, prevWriteOffset);

					//SendWeather #ToDo

					AppendSizeAtFront(pSi, [this](SessionInfomation* pSi)
					{SendTime(pSi); }, prevWriteOffset);

					AppendSizeAtFront(pSi, [this](SessionInfomation* pSi)
					{SendInventory(pSi); }, prevWriteOffset);

					AppendSizeAtFront(pSi, [this](SessionInfomation* pSi)
					{SendHealth(pSi); }, prevWriteOffset);

					AppendSizeAtFront(pSi, [this](SessionInfomation* pSi)
					{SendExperience(pSi); }, prevWriteOffset);

					AppendSizeAtFront(pSi, [this](SessionInfomation* pSi)
					{SendEquippedItem(pSi); }, prevWriteOffset);

					AppendSizeAtFront(pSi, [this](SessionInfomation* pSi)
					{SendPlayerList(pSi); }, prevWriteOffset);

					AppendSizeAtFront(pSi, [this](SessionInfomation* pSi)
					{SendStatistics(pSi); }, prevWriteOffset);
				});
			}
			else
			{
				assert(false);
			}
		}
		else if(SessionState::start == ss)
		{
			// Protocol Version
			short protocolVersion = 0;
			rr = ReadByteByByte(message, protocolVersion);
			readOffSet += rr;
			message += rr;

			pSi->m_protocolVersion = protocolVersion;

			// Server Address
			std::string serverAddressStr;
			rr = ReadString(message, serverAddressStr);
			readOffSet += rr;
			message += rr;

			// Server Port
			uint16_t portNumber;
			rr = Read(message, portNumber);
			portNumber = ntohs(portNumber);
			readOffSet += rr;
			message += rr;

			// Next Stage
			char nextStage = 0;
			rr = ReadByteByByte(message, nextStage);
			readOffSet += rr;
			message += rr;

			pSi->m_sessionState = (SessionState)nextStage;
		}
		else if(SessionState::in_game == ss)
		{
			if(PacketType::ClientSettings == pt)
			{
				// Server Address
				std::string localeString;
				rr = ReadString(message, localeString);
				readOffSet += rr;
				message += rr;
			}
			else
			{
				assert(false);
			}
		}
		else
		{
			assert(false);
		}
	}
}