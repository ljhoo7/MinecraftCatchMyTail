#include "GBServer.h"

namespace GenericBoson
{
	void Server::SendStartCompress(ExpandedOverlapped& eol)
	{
		WriteByteByByte(eol, (int32_t)PacketType::StartCompression);
		WriteByteByByte(eol, InternalConstant::CompressThreshold);

		EnqueueAndIssueSend(eol);
	}

	void Server::SendLoginSuccess(ExpandedOverlapped& eol)
	{
		WriteByteByByte(eol, (int32_t)PacketType::LoginSuccess);

		// UUID #ToDo
		eol.m_uuid = "5550AEA5-0443-4C06-A1CB-CF916EA1623D";
		WriteString(eol, eol.m_uuid);
		WriteString(eol, eol.m_userName);

		EnqueueAndIssueSend(eol);
	}

	void Server::SendJoinGame(ExpandedOverlapped& eol)
	{
		WriteByteByByte(eol, (int32_t)PacketType::JoinGame);

		// Sending FermionID
		Write4BytesAsBigEndian(eol, eol.m_controllableCharacter.m_ID);

		uint8_t hardCoreFlag = 0;
		WriteByteByByte(eol, hardCoreFlag);

		Dimension demension = Dimension::overworld;
		Write4BytesAsBigEndian(eol, demension);

		uint8_t difficulty = 2; // 2 = Normal
		WriteByteByByte(eol, difficulty);

		uint8_t maxPlayerCount = 255;
		WriteByteByByte(eol, maxPlayerCount);

		std::string levelType = "default";
		WriteString(eol, levelType);

		uint8_t reducedDebugInfo = 0; // bool
		WriteByteByByte(eol, reducedDebugInfo);

		EnqueueAndIssueSend(eol);
	}

	void Server::SendSpawnSpot(ExpandedOverlapped& eol)
	{
		WriteByteByByte(eol, (int32_t)PacketType::SpawnSpot);

		GBVector3<int> spawnSpot(10, 10, 10);
		WriteIntGBVector3(eol, spawnSpot);

		EnqueueAndIssueSend(eol);
	}

	void Server::SendDifficulty(ExpandedOverlapped& eol)
	{
		WriteByteByByte(eol, (int32_t)PacketType::Difficulty);

		char difficulty = 1;
		WriteByteByByte(eol, difficulty);

		EnqueueAndIssueSend(eol);
	}

	void Server::SendCharacterAbility(ExpandedOverlapped& eol)
	{
		WriteByteByByte(eol, (int32_t)PacketType::CharacterAbility);

		WriteByteByByte(eol, eol.m_controllableCharacter.m_abilityState);

		float correctedFlyingMaxSpeed = 0.05f * eol.m_controllableCharacter.m_flyingMaxSpeed;
		Write4BytesAsBigEndian(eol, correctedFlyingMaxSpeed);

		float correctedSprintingMaxSpeed = 0.05f * eol.m_controllableCharacter.m_sprintingMaxSpeed;
		Write4BytesAsBigEndian(eol, correctedSprintingMaxSpeed);

		EnqueueAndIssueSend(eol);
	}

	void Server::SendTime(ExpandedOverlapped& eol)
	{
		WriteByteByByte(eol, (int32_t)PacketType::Time);
		Write8BytesAsBigEndian(eol, m_world.m_ageMs);

		// false == m_world.m_dayLightEnabled #ToDo

		Write8BytesAsBigEndian(eol, m_world.m_timeOfDay);

		EnqueueAndIssueSend(eol);
	}

	void Server::SendInventory(ExpandedOverlapped& eol)
	{
		WriteByteByByte(eol, (int32_t)PacketType::Inventory);
		WriteByteByByte(eol, eol.m_controllableCharacter.m_inventory.m_ID);
		WriteByteByByte(eol, (int16_t)eol.m_controllableCharacter.m_inventory.GetTotalSlotCount());

		for (auto& pSlot : eol.m_controllableCharacter.m_inventory.m_slotVector)
		{
			// #ToDo
			pSlot->WriteItem();
		}

		EnqueueAndIssueSend(eol);
	}

	void Server::SendHealth(ExpandedOverlapped& eol)
	{
		WriteByteByByte(eol, (int32_t)PacketType::Health);
		Write4BytesAsBigEndian(eol, eol.m_controllableCharacter.m_health);
		WriteByteByByte(eol, eol.m_controllableCharacter.m_foodLevel);
		Write4BytesAsBigEndian(eol, eol.m_controllableCharacter.m_foodSaturationLevel);

		EnqueueAndIssueSend(eol);
	}

	void Server::SendExperience(ExpandedOverlapped& eol)
	{
		WriteByteByByte(eol, (int32_t)PacketType::Experience);

		float xpPercentage = eol.m_controllableCharacter.GetXpPercentage();
		Write4BytesAsBigEndian(eol, xpPercentage);

		int level = eol.m_controllableCharacter.GetLevel();
		WriteByteByByte(eol, (int32_t)PacketType::Experience);
		WriteByteByByte(eol, eol.m_controllableCharacter.m_experience);

		EnqueueAndIssueSend(eol);
	}

	void Server::SendEquippedItem(ExpandedOverlapped& eol)
	{
		WriteByteByByte(eol, (int32_t)PacketType::EquippedItemChange);
		WriteByteByByte(eol, (int8_t)eol.m_controllableCharacter.m_inventory.m_equippedSlotID);

		EnqueueAndIssueSend(eol);
	}

	void Server::SendPlayerList(ExpandedOverlapped& eol)
	{
		WriteByteByByte(eol, (int32_t)PacketType::PlayerList);

		WriteByteByByte(eol, (int32_t)0);

		WriteByteByByte(eol, (int32_t)1);

		WriteString(eol, eol.m_uuid);

		WriteString(eol, eol.m_userName);

		// #ToDo
		// Send Property List Size
		WriteByteByByte(eol, (int32_t)0);

		WriteByteByByte(eol, (int32_t)m_world.m_gameMode);

		// #ToDo
		WriteByteByByte(eol, (int32_t)m_world.m_pingMs);

		WriteByteByByte(eol, (char)0);

		EnqueueAndIssueSend(eol);
	}


	void Server::ConsumeGatheredMessage(ExpandedOverlapped& eol, char* message, const uint32_t messageSize, uint32_t& readOffSet)
	{
		// Packet Type
		char packetType = 0;
		uint32_t readPacketTypeByteLength = ReadByteByByte(message, packetType);
		readOffSet += readPacketTypeByteLength;
		message += readPacketTypeByteLength;

		switch (eol.m_sessionState)
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

				eol.m_userName = userName;

				SendStartCompress(eol);
				SendLoginSuccess(eol);
				SendJoinGame(eol);
				SendSpawnSpot(eol);
				SendDifficulty(eol);
				SendCharacterAbility(eol);

				//SendWeather #ToDo

				SendTime(eol);
				SendInventory(eol);
				SendHealth(eol);
				SendExperience(eol);
				SendEquippedItem(eol);
				SendPlayerList(eol);

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

			eol.m_protocolVersion = protocolVersion;

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

			eol.m_sessionState = (SessionState)nextStage;
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