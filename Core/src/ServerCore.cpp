#include "stdafx.h"
#include "ServerCore.h"

namespace GenericBoson
{
	namespace ServerEngine
	{
		ServerCore::~ServerCore()
		{
			WSACleanup();
			
			if(INVALID_SOCKET != m_listenSocket)
			{
				int result = closesocket(m_listenSocket);
				assert(SOCKET_ERROR == result);
			}
		}

		std::pair<int, GBString> ServerCore::Start(const ServerCreateParameter& param)
		{
			m_createParameter = param;

			m_IOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 16);

			sockaddr_in service;

			WSADATA wsaData;
			int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
			if (NO_ERROR != result) {
				return std::make_pair(result, GBFUNCTION);
			}

			m_listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (INVALID_SOCKET == m_listenSocket) {
				return std::make_pair(WSAGetLastError(), GBFUNCTION);
			}
			
			service.sin_family = AF_INET;
			service.sin_addr.s_addr = inet_addr(param.m_ipString.c_str());
			service.sin_port = htons(param.m_port);

			result = bind(m_listenSocket, (SOCKADDR*)&service, sizeof(service));
			if (SOCKET_ERROR == result)
			{
				return std::make_pair(WSAGetLastError(), GBFUNCTION);
			}

			//listen

			// acceptex

			//CreateIoCompletionPort((HANDLE)m_socket, m_IOCP, 0, 0);

			return std::make_pair(NO_ERROR, InternalConstant::SUCCESS);
		}
	}
}
