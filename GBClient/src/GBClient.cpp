#include "GBClient.h"

TestClient::~TestClient()
{
	closesocket(m_clientSocket);
	WSACleanup();
}

void TestClient::Start()
{
	WSADATA wsaData;
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (NO_ERROR != result)
	{
		std::cout << "WSAStartup Error : " << result << std::endl;
	}

	m_clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == m_clientSocket)
	{
		std::cout << "[SocketCreating failed] WSAGetLastError : " << WSAGetLastError() << std::endl;
	}

	sockaddr_in socketAddr;

	socketAddr.sin_family = AF_INET;
	socketAddr.sin_port = htons(MINECRAFT_PORT_NUMBER);

	inet_pton(AF_INET, "127.0.0.1", &(socketAddr.sin_addr));

	int connectResult = connect(m_clientSocket, (sockaddr*)&socketAddr, sizeof(socketAddr));

	if (0 != connectResult)
	{
		std::cout << "[Connection failed] WSAGetLastError : " << WSAGetLastError() << std::endl;
		return;
	}

	GBBuffer gbBuffer;

	MakeAndSendPacket(&m_clientSocket, &gbBuffer, [this](GBBuffer* pGbBuffer)
	{
		char packetType = 0;
		WriteByteByByte(pGbBuffer, packetType);

		short protocolVersion = 340;
		WriteByteByByte(pGbBuffer, protocolVersion);

		std::string serverAddrStr = "127.0.0.1";
		errno_t strncpyResult = WriteString(pGbBuffer, serverAddrStr);

		if (0 != strncpyResult)
		{
			std::cout << "[strncpy_s failed] return value : " << strncpyResult << std::endl;
			assert(false);
		}

		unsigned short port = MINECRAFT_PORT_NUMBER;
		Write2BytesAsBigEndian_Without_Sign(pGbBuffer, port);

		char nextStage = 2;
		WriteByteByByte(pGbBuffer, nextStage);
	});

	MakeAndSendPacket(&m_clientSocket, &gbBuffer, [this](GBBuffer* pGbBuffer)
	{
		char packetType = 0;
		WriteByteByByte(pGbBuffer, packetType);

		std::string IDString = "tester";
		errno_t strncpyResult = WriteString(pGbBuffer, IDString);

		if (0 != strncpyResult)
		{
			std::cout << "[strncpy_s failed] return value : " << strncpyResult << std::endl;
			assert(false);
		}
	});

	int receivedBytes = 0;
	while(true)
	{
		GBBuffer tmpBuffer;

		receivedBytes = recv(m_clientSocket, tmpBuffer.m_buffer, 1, NULL);
		tmpBuffer.m_readOffset += 1;

		if (SOCKET_ERROR == receivedBytes)
		{
			std::cout << "[recv length failed] WSAGetLastError : " << WSAGetLastError() << std::endl;
			return;
		}

		GatheringMessage(&tmpBuffer.m_buffer[1], tmpBuffer.m_buffer[0]);
		
		ClientConsumeGatheredMessage(tmpBuffer, tmpBuffer.m_buffer[0]);
	}
}

// 'char' is -127 to 127. So use uint8_t!
void TestClient::GatheringMessage(char* message, uint8_t leftBytesToRecieve)
{
	while (0 < leftBytesToRecieve)
	{
		int receivedBytes = recv(m_clientSocket, message, leftBytesToRecieve, NULL);

		assert(receivedBytes < 256);

		if (SOCKET_ERROR == receivedBytes)
		{
			std::cout << "[recv payload failed] WSAGetLastError : " << WSAGetLastError() << std::endl;
			return;
		}

		leftBytesToRecieve -= (uint8_t)receivedBytes;
		message += receivedBytes;

		assert(0 <= leftBytesToRecieve);
	}

}

// Second parameter type must be uint8_t! char is -127 < char < 127.
void TestClient::ClientConsumeGatheredMessage(GBBuffer& buffer, uint8_t receivedMessageSize)
{
	char uncompressedSize = 0;
	uint32_t rr = 0;
	if (SessionState::in_game == m_sessionState)
	{
		// In Compression mode,
		// If current state is IN_GAME,
		// The header should be 'MessageSize + DataSize'.
		// MessageSize : size of all fields below.
		// DataSize : Zero means below not compressed.

		
		rr = ReadByteByByte(&buffer.m_buffer[buffer.m_readOffset], uncompressedSize);
		buffer.m_readOffset += rr;
		receivedMessageSize -= rr;
	}

	if (0 != uncompressedSize)
	{
		// Uncompress();
		assert(false); // temporary
	}

	while (true)
	{
		char packetType = 0;
		rr = ReadByteByByte(&buffer.m_buffer[buffer.m_readOffset], packetType);
		buffer.m_readOffset += rr;
		receivedMessageSize -= rr;

		PacketType pt = (PacketType)packetType;
		if (PacketType::StartCompression == pt)
		{
			uint32_t compressionThreashold = 0;
			rr = ReadByteByByte(&buffer.m_buffer[buffer.m_readOffset], compressionThreashold);
			buffer.m_readOffset += rr;
			receivedMessageSize -= rr;

			m_sessionState = SessionState::in_game;
		}
		else if (PacketType::LoginSuccess == pt)
		{
			std::string clientUUIDString;
			rr = ReadString(&buffer.m_buffer[buffer.m_readOffset], clientUUIDString);
			buffer.m_readOffset += rr;
			receivedMessageSize -= rr;

			std::string userNameString;
			rr = ReadString(&buffer.m_buffer[buffer.m_readOffset], userNameString);
			buffer.m_readOffset += rr;
			receivedMessageSize -= rr;
		}
		else if (PacketType::JoinGame == pt)
		{
			uint32_t playerUniqueID = Read4BytesAsBigEndian_Without_Sign(&buffer);
			receivedMessageSize -= sizeof(playerUniqueID);

			uint8_t hardMode = 0;
			rr = ReadByteByByte(&buffer.m_buffer[buffer.m_readOffset], hardMode);
			buffer.m_readOffset += rr;
			receivedMessageSize -= rr;

			uint32_t dimesion = Read4BytesAsBigEndian_Without_Sign(&buffer);
			receivedMessageSize -= sizeof(dimesion);

			uint8_t difficulty = 0;
			rr = ReadByteByByte(&buffer.m_buffer[buffer.m_readOffset], difficulty);
			buffer.m_readOffset += rr;
			receivedMessageSize -= rr;

			uint8_t maxPlayerCount = 0;
			rr = ReadByteByByte(&buffer.m_buffer[buffer.m_readOffset], maxPlayerCount);
			buffer.m_readOffset += rr;
			receivedMessageSize -= rr;

			std::string levelType;
			rr = ReadString(&buffer.m_buffer[buffer.m_readOffset], levelType);
			buffer.m_readOffset += rr;
			receivedMessageSize -= rr;

			bool reducedDebugInfo = false;
			rr = ReadByteByByte(&buffer.m_buffer[buffer.m_readOffset], reducedDebugInfo);
			buffer.m_readOffset += rr;
			receivedMessageSize -= rr;
		}
		else if (PacketType::SpawnSpot == pt)
		{
			GBVector3<int> spawnSpot(0, 0, 0);
			rr = ReadIntGBVector3(&buffer, spawnSpot);
			receivedMessageSize -= rr;
		}
		else if (PacketType::Difficulty == pt)
		{
			uint8_t difficulty;
			rr = ReadByteByByte(&buffer.m_buffer[buffer.m_readOffset], difficulty);
			buffer.m_readOffset += rr;
			receivedMessageSize -= rr;
		}
		else if (PacketType::CharacterAbility == pt)
		{
			uint8_t endianChangedFlags;
			rr = ReadByteByByte(&buffer.m_buffer[buffer.m_readOffset], endianChangedFlags);
			buffer.m_readOffset += rr;
			receivedMessageSize -= rr;

			float flyingMaxSpeed = ReadFloat(&buffer);
			receivedMessageSize -= 4;

			float normalMaxSpeed = ReadFloat(&buffer);
			receivedMessageSize -= 4;
		}
		else if (PacketType::Time == pt)
		{
			uint64_t endianChangedWorldAge = Read8BytesAsBigEndian_Without_Sign(&buffer);
			receivedMessageSize -= sizeof(endianChangedWorldAge);

			uint64_t endianChangedTimeOfDay = Read8BytesAsBigEndian_Without_Sign(&buffer);
			receivedMessageSize -= sizeof(endianChangedTimeOfDay);
		}
		else if (PacketType::Inventory == pt)
		{
			uint8_t inventoryID;
			rr = ReadByteByByte(&buffer.m_buffer[buffer.m_readOffset], inventoryID);
			buffer.m_readOffset += rr;
			receivedMessageSize -= rr;

			short slotNum;
			rr = ReadByteByByte(&buffer.m_buffer[buffer.m_readOffset], slotNum);
			buffer.m_readOffset += rr;
			receivedMessageSize -= rr;

			for (int k = 0; k < slotNum; ++k)
			{
				short itemType = Read2BytesAsBigEndian_Without_Sign(&buffer);
				receivedMessageSize -= sizeof(itemType);

				if (-1 == itemType)
				{
					// The tem is empty
					continue;
				}
			}
		}
		else if (PacketType::Health == pt)
		{
			float health = ReadFloat(&buffer);
			receivedMessageSize -= 4;

			uint32_t foodLevel;
			rr = ReadByteByByte(&buffer.m_buffer[buffer.m_readOffset], foodLevel);
			buffer.m_readOffset += rr;
			receivedMessageSize -= rr;

			float foodSaturationLevel = ReadFloat(&buffer);
			receivedMessageSize -= 4;
		}
		else if (PacketType::Experience == pt)
		{
			float xpPercent = ReadFloat(&buffer);
			receivedMessageSize -= 4;

			uint32_t xpLevel;
			rr = ReadByteByByte(&buffer.m_buffer[buffer.m_readOffset], xpLevel);
			buffer.m_readOffset += rr;
			receivedMessageSize -= rr;

			uint32_t currentXp;
			rr = ReadByteByByte(&buffer.m_buffer[buffer.m_readOffset], currentXp);
			buffer.m_readOffset += rr;
			receivedMessageSize -= rr;
		}
		else if (PacketType::HeldItemChange == pt)
		{
			uint8_t equippedSlotNum;
			rr = ReadByteByByte(&buffer.m_buffer[buffer.m_readOffset], equippedSlotNum);
			buffer.m_readOffset += rr;
			receivedMessageSize -= rr;
		}
		else if (PacketType::PlayerList == pt)
		{
			uint32_t zero;
			rr = ReadByteByByte(&buffer.m_buffer[buffer.m_readOffset], zero);
			buffer.m_readOffset += rr;
			receivedMessageSize -= rr;

			uint32_t one;
			rr = ReadByteByByte(&buffer.m_buffer[buffer.m_readOffset], one);
			buffer.m_readOffset += rr;
			receivedMessageSize -= rr;

			GBUUID playerUUID;
			rr = ReadUUID(&buffer.m_buffer[buffer.m_readOffset], playerUUID);
			buffer.m_readOffset += rr;
			receivedMessageSize -= rr;

			std::string playerListName;
			rr = ReadString(&buffer.m_buffer[buffer.m_readOffset], playerListName);
			buffer.m_readOffset += rr;
			receivedMessageSize -= rr;

			uint32_t propertiesSize;
			rr = ReadByteByByte(&buffer.m_buffer[buffer.m_readOffset], propertiesSize);
			buffer.m_readOffset += rr;
			receivedMessageSize -= rr;

			for (int k = 0; k < propertiesSize; ++k)
			{
				std::string nameProperty;
				rr = ReadString(&buffer.m_buffer[buffer.m_readOffset], playerListName);
				buffer.m_readOffset += rr;
				receivedMessageSize -= rr;

				std::string valueProperty;
				rr = ReadString(&buffer.m_buffer[buffer.m_readOffset], valueProperty);
				buffer.m_readOffset += rr;
				receivedMessageSize -= rr;

				bool signature;
				rr = ReadByteByByte(&buffer.m_buffer[buffer.m_readOffset], signature);
				buffer.m_readOffset += rr;
				receivedMessageSize -= rr;

				if (true == signature)
				{
					std::string signatureString;
					rr = ReadString(&buffer.m_buffer[buffer.m_readOffset], signatureString);
					buffer.m_readOffset += rr;
					receivedMessageSize -= rr;
				}
			}

			uint32_t effectiveGameMode;
			rr = ReadByteByByte(&buffer.m_buffer[buffer.m_readOffset], effectiveGameMode);
			buffer.m_readOffset += rr;
			receivedMessageSize -= rr;

			uint32_t ping;
			rr = ReadByteByByte(&buffer.m_buffer[buffer.m_readOffset], ping);
			buffer.m_readOffset += rr;
			receivedMessageSize -= rr;

			bool onlyForFalse;
			rr = ReadByteByByte(&buffer.m_buffer[buffer.m_readOffset], onlyForFalse);
			buffer.m_readOffset += rr;
			receivedMessageSize -= rr;
		}
		else if (PacketType::Statistics == pt)
		{
			uint32_t storeSizeSum;
			rr = ReadByteByByte(&buffer.m_buffer[buffer.m_readOffset], storeSizeSum);
			buffer.m_readOffset += rr;
			receivedMessageSize -= rr;

			std::map<std::string, uint32_t> staticsMap;
			for (int k = 0; k < storeSizeSum; ++k)
			{
				std::string keyString;
				rr = ReadString(&buffer.m_buffer[buffer.m_readOffset], keyString);
				buffer.m_readOffset += rr;
				receivedMessageSize -= rr;

				uint32_t value;
				rr = ReadByteByByte(&buffer.m_buffer[buffer.m_readOffset], value);
				buffer.m_readOffset += rr;
				receivedMessageSize -= rr;

				staticsMap.emplace(keyString, value);
			}
		}
		else if (PacketType::UnlockRecipe == pt)
		{
			uint32_t zero;
			rr = ReadByteByByte(&buffer.m_buffer[buffer.m_readOffset], zero);
			buffer.m_readOffset += rr;
			receivedMessageSize -= rr;

			bool trueOnly;
			rr = ReadByteByByte(&buffer.m_buffer[buffer.m_readOffset], trueOnly);
			buffer.m_readOffset += rr;
			receivedMessageSize -= rr;

			bool falseOnly;
			rr = ReadByteByByte(&buffer.m_buffer[buffer.m_readOffset], falseOnly);
			buffer.m_readOffset += rr;
			receivedMessageSize -= rr;

			uint32_t isZero;
			rr = ReadByteByByte(&buffer.m_buffer[buffer.m_readOffset], isZero);
			buffer.m_readOffset += rr;
			receivedMessageSize -= rr;

			if (1 == isZero)
			{
				rr = ReadByteByByte(&buffer.m_buffer[buffer.m_readOffset], m_recipeID);
				buffer.m_readOffset += rr;
				receivedMessageSize -= rr;
			}

			uint32_t isZero2;
			rr = ReadByteByByte(&buffer.m_buffer[buffer.m_readOffset], isZero2);
			buffer.m_readOffset += rr;
			receivedMessageSize -= rr;

			assert(isZero == isZero2);

			uint32_t recipeIDConfirm;
			if(1 == isZero2)
			{
				rr = ReadByteByByte(&buffer.m_buffer[buffer.m_readOffset], recipeIDConfirm);
				buffer.m_readOffset += rr;
				receivedMessageSize -= rr;
			}

			assert(recipeIDConfirm == m_recipeID);
		}

		assert(0 <= receivedMessageSize);

		if (0 == receivedMessageSize)
		{
			break;
		}
	}
}