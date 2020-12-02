#include "GBClient.h"

const int BUFFER_SIZE = 1024;

template<typename T>
T* TestClient::AssignFromBuffer(GBBuffer* pGbBuffer)
{
	assert(pGbBuffer->m_writeOffset + sizeof(T) < BUFFER_SIZE);

	size_t bytesToAssign = sizeof(T);

	T* pAddrToReturn = (T*)&pGbBuffer->m_buffer[pGbBuffer->m_writeOffset];

	pGbBuffer->m_writeOffset += bytesToAssign;

	return pAddrToReturn;
}

template<typename STRING>
void TestClient::InscribeStringToBuffer(const STRING& str, GBBuffer* pGbBuffer)
{
	char* pStringLength = AssignFromBuffer<char>(pGbBuffer);
	char stringLength = (char)str.length();
	WriteByteByByte(pGbBuffer, stringLength);

	assert(pGbBuffer->m_writeOffset + *pStringLength < BUFFER_SIZE);

	errno_t strncpyResult = strncpy_s(&pGbBuffer->m_buffer[pGbBuffer->m_writeOffset], BUFFER_SIZE - pGbBuffer->m_writeOffset, (char*)str.c_str(), *pStringLength);

	if (0 != strncpyResult)
	{
		std::cout << "[strncpy_s failed] return value : " << strncpyResult << std::endl;
		assert(false);
	}

	pGbBuffer->m_writeOffset += *pStringLength;
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
	}

	GBBuffer gbBuffer;
	std::string serverAddrStr = "127.0.0.1";

	char* pPacketLength = AssignFromBuffer<char>(&gbBuffer);
	char packetLength = sizeof(int32_t) + sizeof(short) + sizeof(char) + serverAddrStr.length() + sizeof(short) + sizeof(char);
	WriteByteByByte(&gbBuffer, packetLength);

	// [1]
	char* pPacketType = AssignFromBuffer<char>(&gbBuffer);
	char packetType = 0;
	WriteByteByByte(&gbBuffer, packetType);

	// [2]
	char* pProtocolVersion = AssignFromBuffer<char>(&gbBuffer);
	char protocolVersion = 340;
	WriteByteByByte(&gbBuffer, protocolVersion);

	// [3]
	InscribeStringToBuffer(serverAddrStr, &gbBuffer);

	// [4]
	short* pPort = AssignFromBuffer<short>(&gbBuffer);
	short port = MINECRAFT_PORT_NUMBER;
	WriteByteByByte(&gbBuffer, port);

	// [5]
	char* pNextStage = AssignFromBuffer<char>(&gbBuffer);
	char nextStage = 340;
	WriteByteByByte(&gbBuffer, nextStage);

	int sendResult = send(m_clientSocket, gbBuffer.m_buffer, gbBuffer.m_writeOffset, NULL);

	if (SOCKET_ERROR == sendResult)
	{
		std::cout << "[WSASend failed] WSAGetLastError : " << WSAGetLastError() << std::endl;
	}
}