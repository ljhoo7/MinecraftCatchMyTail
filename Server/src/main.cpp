#include <memory>
#include <cstdlib>
#include <WinSock2.h>

#include "../../Core/include/ServerCore.h"

using namespace GenericBoson::ServerEngine;

int main()
{
	auto pServer = std::make_unique<ServerCore>();

	ServerCreateParameter param;
	pServer->Start(param);

	system("pause");
}