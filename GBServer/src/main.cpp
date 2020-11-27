#include <memory>
#include <cstdlib>
#include <WinSock2.h>
#include <iostream>

#include "../../Core/include/GBCore.h"

using namespace GenericBoson::ServerEngine;

int main()
{
	auto pServer = std::make_shared<Core>();

	ServerCreateParameter param;
	auto startResultPair = pServer->Start(param);

	if (NO_ERROR != startResultPair.first)
	{
		std::wcout << L"Server start failed at : " << startResultPair.second << " line" << std::endl;
		std::wcout << L"WSAGetLastError : " << startResultPair.first << std::endl;
	}

	while (true)
	{
		Sleep(10);
	}
}