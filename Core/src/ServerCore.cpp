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

			for (int k = 0; k < SOMAXCONN; ++k)
			{
				if (INVALID_SOCKET == m_listenSocket)
				{
					continue;
				}

				int result = closesocket(m_listenSocket);
				assert(SOCKET_ERROR == result);
			}
		}

		std::pair<int, GBString> ServerCore::Start(const ServerCreateParameter& param)
		{
			m_createParameter = param;

			int coreCount = std::thread::hardware_concurrency();

			m_IOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 2 * coreCount);

			sockaddr_in service;

			WSADATA wsaData;
			int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
			if (NO_ERROR != result) {
				return std::make_pair(result, GBFUNCTION);
			}

			CreateIoCompletionPort((HANDLE)m_listenSocket, m_IOCP, 0, 0);

			m_listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (INVALID_SOCKET == m_listenSocket) {
				return std::make_pair(WSAGetLastError(), GBFUNCTION);
			}
			
			service.sin_family = AF_INET;
			InetPton(AF_INET, param.m_ipString.c_str(), &service.sin_addr.s_addr);
			service.sin_port = htons(param.m_port);

			result = bind(m_listenSocket, (SOCKADDR*)&service, sizeof(service));
			if (SOCKET_ERROR == result)
			{
				return std::make_pair(WSAGetLastError(), GBFUNCTION);
			}

			result = listen(m_listenSocket, SOMAXCONN);
			if (SOCKET_ERROR == result) {
				return std::make_pair(WSAGetLastError(), GBFUNCTION);
			}

			LPFN_ACCEPTEX lpfnAcceptEx = NULL;
			GUID GuidAcceptEx = WSAID_ACCEPTEX;
			DWORD returnedBytes;
			WSAOVERLAPPED olOverlap;

			const int outBufLength = 1024;
			char outputBuffer[1024];
			
			for(int k = 0; k < acceptedSocketArraySize; ++k)
			{
				result = WSAIoctl(m_listenSocket, SIO_GET_EXTENSION_FUNCTION_POINTER,
					&GuidAcceptEx, sizeof(GuidAcceptEx),
					&lpfnAcceptEx, sizeof(lpfnAcceptEx),
					&returnedBytes, NULL, NULL);
				if (result == SOCKET_ERROR)
				{
					return std::make_pair(WSAGetLastError(), GBFUNCTION);
				}

				m_acceptSocketArray[k] = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
				if (INVALID_SOCKET == m_acceptSocketArray[k])
				{
					return std::make_pair(WSAGetLastError(), GBFUNCTION);
				}

				memset(&olOverlap, 0, sizeof(olOverlap));

				BOOL result = lpfnAcceptEx(m_listenSocket, m_acceptSocketArray[k], outputBuffer,
					outBufLength - ((sizeof(sockaddr_in) + 16) * 2),
					sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16,
					&returnedBytes, &olOverlap);
				if (FALSE == result)
				{
					return std::make_pair(WSAGetLastError(), GBFUNCTION);
				}

				HANDLE associateResult = CreateIoCompletionPort((HANDLE)m_acceptSocketArray[k], m_IOCP, (u_long)0, 0);
				if (NULL == associateResult)
				{
					return std::make_pair(WSAGetLastError(), GBFUNCTION);
				}
			}

			return std::make_pair(NO_ERROR, InternalConstant::SUCCESS);
		}
	}
}
