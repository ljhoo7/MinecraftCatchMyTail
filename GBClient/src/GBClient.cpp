#include "GBClient.h"

TestClient::~TestClient()
{
	closesocket(m_clientSocket);
	WSACleanup();
}

template<typename STRING>
void TestClient::InscribeStringToBuffer(const STRING& str, GBBuffer* pGbBuffer)
{
	char stringLength = (char)str.length();
	WriteByteByByte(pGbBuffer, stringLength);

	assert(pGbBuffer->m_writeOffset + stringLength < BUFFER_SIZE);

	errno_t strncpyResult = strncpy_s(&pGbBuffer->m_buffer[pGbBuffer->m_writeOffset], BUFFER_SIZE - pGbBuffer->m_writeOffset, (char*)str.c_str(), stringLength);

	if (0 != strncpyResult)
	{
		std::cout << "[strncpy_s failed] return value : " << strncpyResult << std::endl;
		assert(false);
	}

	pGbBuffer->m_writeOffset += stringLength;
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
		InscribeStringToBuffer(serverAddrStr, pGbBuffer);

		unsigned short port = MINECRAFT_PORT_NUMBER;
		Write2BytesAsBigEndian(pGbBuffer, port);

		char nextStage = 2;
		WriteByteByByte(pGbBuffer, nextStage);
	});

	MakeAndSendPacket(&m_clientSocket, &gbBuffer, [this](GBBuffer* pGbBuffer)
	{
		char packetType = 0;
		WriteByteByByte(pGbBuffer, packetType);

		std::string IDString = "tester";
		InscribeStringToBuffer(IDString, pGbBuffer);
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
		
		ClientConsumeGatheredMessage(&tmpBuffer.m_buffer[1], tmpBuffer.m_buffer[0], tmpBuffer.m_readOffset);
	}
}

void TestClient::GatheringMessage(char* message, uint32_t leftBytesToRecieve)
{
	while (0 < leftBytesToRecieve)
	{
		int receivedBytes = recv(m_clientSocket, message, leftBytesToRecieve, NULL);

		if (SOCKET_ERROR == receivedBytes)
		{
			std::cout << "[recv payload failed] WSAGetLastError : " << WSAGetLastError() << std::endl;
			return;
		}

		leftBytesToRecieve -= receivedBytes;
		message += receivedBytes;

		assert(0 <= leftBytesToRecieve);
	}

}

void TestClient::ClientConsumeGatheredMessage(char* message, uint32_t messageSize, int& readOffSet)
{
	while(true)
	{
		char packetType = 0;
		uint32_t rr = ReadByteByByte(message, packetType);
		readOffSet += rr;
		message += rr;
		messageSize -= rr;

		PacketType pt = (PacketType)packetType;
		if (PacketType::StartCompression == pt)
		{
			uint32_t compressionThreashold = 0;
			rr = ReadByteByByte(message, compressionThreashold);
			readOffSet += rr;
			message += rr;
			messageSize -= rr;
		}
		else if (PacketType::LoginSuccess == pt)
		{
			std::string clientUUIDString;
			rr = ReadString(message, clientUUIDString);
			readOffSet += rr;
			message += rr;
			messageSize -= rr;

			std::string userNameString;
			rr = ReadString(message, userNameString);
			readOffSet += rr;
			message += rr;
			messageSize -= rr;
		}
		else if (PacketType::JoinGame == pt)
		{
			uint32_t playerUniqueID = 0;
			rr = ReadByteByByte(message, playerUniqueID);
			readOffSet += rr;
			message += rr;
			messageSize -= rr;

			char hardMode = 0;
			rr = ReadByteByByte(message, hardMode);
			readOffSet += rr;
			message += rr;
			messageSize -= rr;

			int32_t dimesion = 0;
			rr = ReadByteByByte(message, dimesion);
			readOffSet += rr;
			message += rr;
			messageSize -= rr;

			char difficulty = 0;
			rr = ReadByteByByte(message, difficulty);
			readOffSet += rr;
			message += rr;
			messageSize -= rr;

			char maxPlayerCount = 0;
			rr = ReadByteByByte(message, maxPlayerCount);
			readOffSet += rr;
			message += rr;
			messageSize -= rr;

			std::string levelType;
			rr = ReadString(message, levelType);
			readOffSet += rr;
			message += rr;
			messageSize -= rr;

			bool reducedDebugInfo = false;
			rr = ReadByteByByte(message, reducedDebugInfo);
			readOffSet += rr;
			message += rr;
			messageSize -= rr;
		}
		else if (PacketType::SpawnSpot == pt)
		{
			uint64_t endianChangedVector;
			rr = ReadByteByByte(message, endianChangedVector);
			readOffSet += rr;
			message += rr;
			messageSize -= rr;
		}
		else if (PacketType::Difficulty == pt)
		{
			short endianChangedDifficulty;
			rr = ReadByteByByte(message, endianChangedDifficulty);
			readOffSet += rr;
			message += rr;
			messageSize -= rr;
		}
		else if (PacketType::CharacterAbility == pt)
		{
			short endianChangedFlags;
			rr = ReadByteByByte(message, endianChangedFlags);
			readOffSet += rr;
			message += rr;
			messageSize -= rr;

			uint32_t tmpVariable;
			float endianChangedFlyingMaxSpeed;
			rr = ReadByteByByte(message, tmpVariable);
			endianChangedFlyingMaxSpeed = (float)tmpVariable;
			readOffSet += rr;
			message += rr;
			messageSize -= rr;

			float endianChangedNormalMaxSpeed;
			rr = ReadByteByByte(message, tmpVariable);
			endianChangedFlyingMaxSpeed = (float)tmpVariable;
			readOffSet += rr;
			message += rr;
			messageSize -= rr;
		}

		assert(0 <= messageSize);

		if (0 == messageSize)
		{
			break;
		}
	}
}