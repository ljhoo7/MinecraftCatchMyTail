#include "TestClient.h"

template<typename T>
void TestClient::Send(const T& param)
{
	WSABUF buf;
	buf.buf = (char*)&param;
	buf.len = 1000;
	DWORD bytesToSend = sizeof(T);
	WSAOVERLAPPED ol;

	WSASend(m_clientSocket, &buf, 1, &bytesToSend, NULL, &ol, NULL);
}

template<typename STRING>
void TestClient::SendString(const STRING& str)
{
	// String Length
	char inStringSize = str.length();
	Send(inStringSize);

	WSABUF buf;
	buf.buf = (char*)&str.c_str();
	buf.len = inStringSize;
	DWORD bytesToSend = inStringSize;
	WSAOVERLAPPED ol;

	WSASend(m_clientSocket, &buf, 1, &bytesToSend, NULL, &ol, NULL);
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
	if (INVALID_SOCKET == m_clientSocket) {
		std::cout << "Creating a socket Error : WSAGetLastError - " << result << std::endl;
	}

	sockaddr_in socketAddr;

	socketAddr.sin_family = AF_INET;
	socketAddr.sin_port = htons(25565);

	inet_pton(AF_INET, "127.0.0.1", &(socketAddr.sin_addr));

	int connectResult = WSAConnect(m_clientSocket, (sockaddr*)&socketAddr, sizeof(socketAddr), NULL, NULL, NULL, NULL);

	if (0 != connectResult)
	{
		std::cout << "Connection failed : WSAGetLastError : " << WSAGetLastError() << std::endl;
	}

	short protocolVersion;
	Send(protocolVersion);

	std::string serverAddrStr = "127.0.0.1";
	SendString(serverAddrStr);
}