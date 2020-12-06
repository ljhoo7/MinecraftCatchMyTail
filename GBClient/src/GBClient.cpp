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
	std::string serverAddrStr = "127.0.0.1";
	char addStrSize = (char)serverAddrStr.length();

	char* pPacketLength = AssignFromBuffer<char>(&gbBuffer);
	
	// [1]
	char packetType = 0;
	WriteByteByByte(&gbBuffer, packetType);

	// [2]
	short protocolVersion = 340;
	WriteByteByByte(&gbBuffer, protocolVersion);

	// [3]
	InscribeStringToBuffer(serverAddrStr, &gbBuffer);

	// [4]
	unsigned short port = MINECRAFT_PORT_NUMBER;
	Write2BytesAsBigEndian(&gbBuffer, port);

	// [5]
	char nextStage = 2;
	WriteByteByByte(&gbBuffer, nextStage);

	*pPacketLength = (char)(gbBuffer.m_writeOffset - 1);

	int sendResult = send(m_clientSocket, gbBuffer.m_buffer, gbBuffer.m_writeOffset, NULL);

	if (SOCKET_ERROR == sendResult)
	{
		std::cout << "[send failed] WSAGetLastError : " << WSAGetLastError() << std::endl;
		return;
	}

	int receivedBytes = 0;

	while(true)
	{
		receivedBytes = recv(m_clientSocket, gbBuffer.m_buffer, 1, NULL);

		if (SOCKET_ERROR == receivedBytes)
		{
			std::cout << "[recv failed] WSAGetLastError : " << WSAGetLastError() << std::endl;
			return;
		}

		receivedBytes = recv(m_clientSocket, &gbBuffer.m_buffer[1], gbBuffer.m_buffer[0], NULL);

		if (SOCKET_ERROR == receivedBytes)
		{
			std::cout << "[recv failed] WSAGetLastError : " << WSAGetLastError() << std::endl;
			return;
		}

		// interpretation
	}
}