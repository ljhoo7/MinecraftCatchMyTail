#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <cstdlib>

class TestClient
{
	SOCKET m_clientSocket = INVALID_SOCKET;
public:
	TestClient() = default;
	~TestClient()
	{
		closesocket(m_clientSocket);
		WSACleanup();
	}

	void Start();
};

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
}

int main()
{
	auto pTestClient = std::make_unique<TestClient>();

	pTestClient->Start();

	while (true)
	{
		Sleep(10);
	}

	return 0;
}