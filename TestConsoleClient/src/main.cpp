#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <cstdlib>

int main()
{
	WSADATA wsaData;
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (NO_ERROR != result)
	{
		std::cout << "WSAStartup Error : " << result << std::endl;
	}

	SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == clientSocket) {
		std::cout << "Creating a socket Error : WSAGetLastError - " << result << std::endl;
	}

	sockaddr_in service;
	InetPton(AF_INET, L"127.0.0.1", &service.sin_addr.s_addr);
	service.sin_port = htons(25565);

	int connectResult = connect(clientSocket, (sockaddr*)&service, sizeof(service));

	system("pause");

	return 0;
}