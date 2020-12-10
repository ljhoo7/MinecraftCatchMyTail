#include "GBClient.h"

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
		// [1]
		char packetType = 0;
		WriteByteByByte(pGbBuffer, packetType);

		// [2]
		short protocolVersion = 340;
		WriteByteByByte(pGbBuffer, protocolVersion);

		// [3]
		std::string serverAddrStr = "127.0.0.1";
		InscribeStringToBuffer(serverAddrStr, pGbBuffer);

		// [4]
		unsigned short port = MINECRAFT_PORT_NUMBER;
		Write2BytesAsBigEndian(pGbBuffer, port);

		// [5]
		char nextStage = 2;
		WriteByteByByte(pGbBuffer, nextStage);
	});

	MakeAndSendPacket(&m_clientSocket, &gbBuffer, [this](GBBuffer* pGbBuffer)
	{
		// [1]
		char uncompressedSize = 0;
		WriteByteByByte(pGbBuffer, uncompressedSize);

		// [2]
		char packetType = 0;
		WriteByteByByte(pGbBuffer, packetType);

		// [3]
		std::string IDString = "tester";
		InscribeStringToBuffer(IDString, pGbBuffer);
	});

	int receivedBytes = 0;
	while(true)
	{
		receivedBytes = recv(m_clientSocket, gbBuffer.m_buffer, 1, NULL);

		if (SOCKET_ERROR == receivedBytes)
		{
			std::cout << "[recv length failed] WSAGetLastError : " << WSAGetLastError() << std::endl;
			return;
		}

		receivedBytes = recv(m_clientSocket, &gbBuffer.m_buffer[1], gbBuffer.m_buffer[0], NULL);

		if (SOCKET_ERROR == receivedBytes)
		{
			std::cout << "[recv payload failed] WSAGetLastError : " << WSAGetLastError() << std::endl;
			return;
		}

		// interpretation
	}
}