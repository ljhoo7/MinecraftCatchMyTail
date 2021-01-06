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
		
		ClientConsumeGatheredMessage(tmpBuffer, tmpBuffer.m_buffer[0]);
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

void TestClient::ClientConsumeGatheredMessage(GBBuffer& buffer, uint32_t receivedMessageSize)
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

	while(true)
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
			uint32_t playerUniqueID = Read4BytesAsBigEndian(&buffer);
			receivedMessageSize -= sizeof(playerUniqueID);

			char hardMode = 0;
			rr = ReadByteByByte(&buffer.m_buffer[buffer.m_readOffset], hardMode);
			buffer.m_readOffset += rr;
			receivedMessageSize -= rr;

			uint32_t dimesion = Read4BytesAsBigEndian(&buffer);
			receivedMessageSize -= sizeof(dimesion);

			char difficulty = 0;
			rr = ReadByteByByte(&buffer.m_buffer[buffer.m_readOffset], difficulty);
			buffer.m_readOffset += rr;
			receivedMessageSize -= rr;

			char maxPlayerCount = 0;
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
			uint64_t endianChangedVector;
			rr = ReadByteByByte(&buffer.m_buffer[buffer.m_readOffset], endianChangedVector);
			buffer.m_readOffset += rr;
			receivedMessageSize -= rr;
		}
		else if (PacketType::Difficulty == pt)
		{
			short endianChangedDifficulty;
			rr = ReadByteByByte(&buffer.m_buffer[buffer.m_readOffset], endianChangedDifficulty);
			buffer.m_readOffset += rr;
			receivedMessageSize -= rr;
		}
		else if (PacketType::CharacterAbility == pt)
		{
			short endianChangedFlags;
			rr = ReadByteByByte(&buffer.m_buffer[buffer.m_readOffset], endianChangedFlags);
			buffer.m_readOffset += rr;
			receivedMessageSize -= rr;

			uint32_t tmpVariable;
			float endianChangedFlyingMaxSpeed;
			rr = ReadByteByByte(&buffer.m_buffer[buffer.m_readOffset], tmpVariable);
			endianChangedFlyingMaxSpeed = (float)tmpVariable;
			buffer.m_readOffset += rr;
			receivedMessageSize -= rr;

			float endianChangedNormalMaxSpeed;
			rr = ReadByteByByte(&buffer.m_buffer[buffer.m_readOffset], tmpVariable);
			endianChangedFlyingMaxSpeed = (float)tmpVariable;
			buffer.m_readOffset += rr;
			receivedMessageSize -= rr;
		}

		assert(0 <= receivedMessageSize);

		if (0 == receivedMessageSize)
		{
			break;
		}
	}
}