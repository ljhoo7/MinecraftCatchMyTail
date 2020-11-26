#include "TestClient.h"

const int BUFFER_SIZE = 1024;

template<typename T>
T* TestClient::AssignFromBuffer(char* buffer, int& writeOffset)
{
	assert(writeOffset + sizeof(T) < BUFFER_SIZE);

	size_t bytesToAssign = sizeof(T);

	T* pAddrToReturn = (T*)&buffer[writeOffset];

	writeOffset += bytesToAssign;

	return pAddrToReturn;
}

template<typename STRING>
void TestClient::InscribeStringToBuffer(const STRING& str, char* buffer, int& writeOffset)
{
	char* pStringLength = AssignFromBuffer<char>(buffer, writeOffset);
	*pStringLength = (char)str.length();

	assert(writeOffset + *pStringLength < BUFFER_SIZE);

	errno_t strncpyResult = strncpy_s(&buffer[writeOffset], BUFFER_SIZE - writeOffset, (char*)str.c_str(), *pStringLength);

	if (0 != strncpyResult)
	{
		std::cout << "[strncpy_s failed] return value : " << strncpyResult << std::endl;
		assert(false);
	}

	writeOffset += *pStringLength;
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

	int writeOffset = 0;
	char buffer[BUFFER_SIZE] = { 0, };
	std::string serverAddrStr = "127.0.0.1";

	char* pPacketLength = AssignFromBuffer<char>(buffer, writeOffset);
	*pPacketLength = sizeof(int32_t) + sizeof(short) + sizeof(char) + serverAddrStr.length() + sizeof(short) + sizeof(char);

	// [1]
	int32_t* pPacketType = AssignFromBuffer<int32_t>(buffer, writeOffset);
	*pPacketType = 0;

	// [2]
	short* pProtocolVersion = AssignFromBuffer<short>(buffer, writeOffset);
	*pProtocolVersion = 340;

	// [3]
	InscribeStringToBuffer(serverAddrStr, buffer, writeOffset);

	// [4]
	short* pPort = AssignFromBuffer<short>(buffer, writeOffset);
	*pPort = MINECRAFT_PORT_NUMBER;

	// [5]
	char* pNextStage = AssignFromBuffer<char>(buffer, writeOffset);
	*pNextStage = 340;

	int sendResult = send(m_clientSocket, buffer, writeOffset, NULL);

	if (SOCKET_ERROR == sendResult)
	{
		std::cout << "[WSASend failed] WSAGetLastError : " << WSAGetLastError() << std::endl;
	}
}