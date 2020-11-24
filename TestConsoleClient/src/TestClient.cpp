#include "TestClient.h"

template<typename T>
void TestClient::Send(const T& param)
{
	WSABUF buf;
	buf.buf = (char*)&param;
	DWORD bytesToSend = sizeof(T);
	buf.len = bytesToSend;
	WSAOVERLAPPED ol;

	int sendResult = WSASend(m_clientSocket, &buf, 1, &bytesToSend, NULL, &ol, NULL);

	if (0 != sendResult)
	{
		std::cout << "[WSASend failed] WSAGetLastError : " << WSAGetLastError() << std::endl;
	}
}

template<typename STRING>
void TestClient::SendString(const STRING& str)
{
	// String Length
	char inStringSize = str.length();
	Send(inStringSize);

	WSABUF buf;
	char* buffer = (char*)str.c_str();
	buf.buf = buffer;
	buf.len = inStringSize;
	DWORD bytesToSend = inStringSize;
	WSAOVERLAPPED ol;

	int sendResult = WSASend(m_clientSocket, &buf, 1, &bytesToSend, NULL, &ol, NULL);

	if (0 != sendResult)
	{
		std::cout << "[WSASend failed] WSAGetLastError : " << WSAGetLastError() << std::endl;
	}
}

void TestClient::Start()
{
	WSADATA wsaData;
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (NO_ERROR != result)
	{
		std::cout << "WSAStartup Error : " << result << std::endl;
	}

	m_clientSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == m_clientSocket)
	{
		std::cout << "[SocketCreating failed] WSAGetLastError : " << result << std::endl;
	}

	sockaddr_in socketAddr;

	socketAddr.sin_family = AF_INET;
	socketAddr.sin_port = htons(MINECRAFT_PORT_NUMBER);

	inet_pton(AF_INET, "127.0.0.1", &(socketAddr.sin_addr));

	int connectResult = WSAConnect(m_clientSocket, (sockaddr*)&socketAddr, sizeof(socketAddr), NULL, NULL, NULL, NULL);

	if (0 != connectResult)
	{
		std::cout << "[Connection failed] WSAGetLastError : " << WSAGetLastError() << std::endl;
	}

	int32_t packetType = 0;
	Send(packetType);

	short protocolVersion = 340;		// 340 = 1.12.2
	Send(protocolVersion);

	std::string serverAddrStr = "127.0.0.1";
	SendString(serverAddrStr);

	Send(MINECRAFT_PORT_NUMBER);

	char nextStage = 0;		// 0 == LOGIN_START
	Send(0);
}