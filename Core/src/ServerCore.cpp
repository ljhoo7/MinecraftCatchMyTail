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

			for (int k = 0; k < m_threadPoolSize; ++k)
			{
				if (false == m_threadPool[k].joinable())
				{
					continue;
				}

				m_threadPool[k].join();
			}
		}

		void ServerCore::ThreadFunction()
		{
			DWORD recievedBytes;
			IO_TYPE ioType;
			WSAOVERLAPPED olOverlap;

			memset(&olOverlap, 0, sizeof(olOverlap));

			while(true == m_keepLooping)
			{
				BOOL result = GetQueuedCompletionStatus(m_IOCP, &recievedBytes, (PULONG_PTR)&ioType, (LPOVERLAPPED*)&olOverlap, INFINITE);

				static int cnt = 0;
				std::cout << cnt++ << std::endl;
			}
		}

		std::pair<int, int> ServerCore::Start(const ServerCreateParameter& param)
		{
			m_createParameter = param;

			m_threadPoolSize = 2 * std::thread::hardware_concurrency();

			m_IOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, (u_long)0, 0);

			if (NULL == m_IOCP)
			{
				return std::make_pair(WSAGetLastError(), __LINE__);
			}

			for (int k = 0; k < m_threadPoolSize; ++k)
			{
				m_threadPool.emplace_back([this]()
				{
					this->ThreadFunction();
				});
			}

			sockaddr_in service;

			WSADATA wsaData;
			int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
			if (NO_ERROR != result) {
				return std::make_pair(result, __LINE__);
			}

			m_listenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);
			if (INVALID_SOCKET == m_listenSocket) {
				return std::make_pair(WSAGetLastError(), __LINE__);
			}

			HANDLE associateListenSocketResult = CreateIoCompletionPort((HANDLE)m_listenSocket, m_IOCP, (u_long)IO_TYPE::ACCEPT, 0);

			if (NULL == associateListenSocketResult)
			{
				return std::make_pair(WSAGetLastError(), __LINE__);
			}
			
			service.sin_family = AF_INET;
			InetPton(AF_INET, param.m_ipString.c_str(), &service.sin_addr.s_addr);
			service.sin_port = htons(param.m_port);

			result = bind(m_listenSocket, (SOCKADDR*)&service, sizeof(service));
			if (SOCKET_ERROR == result)
			{
				return std::make_pair(WSAGetLastError(), __LINE__);
			}

			DWORD tmpListenBuffer;

			result = listen(m_listenSocket, SOMAXCONN);
			if (SOCKET_ERROR == result) {
				return std::make_pair(WSAGetLastError(), __LINE__);
			}

			LPFN_ACCEPTEX lpfnAcceptEx = NULL;
			GUID GuidAcceptEx = WSAID_ACCEPTEX;
			DWORD returnedBytes;
			WSAOVERLAPPED olOverlap;

			const int outBufLength = 1024;
			char outputBuffer[1024];
			
			for (int k = 0; k < acceptedSocketArraySize; ++k)
			{
				result = WSAIoctl(m_listenSocket, SIO_GET_EXTENSION_FUNCTION_POINTER,
					&GuidAcceptEx, sizeof(GuidAcceptEx),
					&lpfnAcceptEx, sizeof(lpfnAcceptEx),
					&returnedBytes, NULL, NULL);
				if (result == SOCKET_ERROR)
				{
					return std::make_pair(WSAGetLastError(), __LINE__);
				}

				m_acceptSocketArray[k] = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);
				if (INVALID_SOCKET == m_acceptSocketArray[k])
				{
					return std::make_pair(WSAGetLastError(), __LINE__);
				}

				memset(&olOverlap, 0, sizeof(olOverlap));

				BOOL result = lpfnAcceptEx(m_listenSocket, m_acceptSocketArray[k], outputBuffer,
					0,//outBufLength - ((sizeof(sockaddr_in) + 16) * 2),
					sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16,
					&returnedBytes, &olOverlap);
				int lastSocketError = WSAGetLastError();
				if (FALSE == result && ERROR_IO_PENDING != lastSocketError)
				{
					return std::make_pair(lastSocketError, __LINE__);
				}

				HANDLE associateAcceptSocketResult = CreateIoCompletionPort((HANDLE)m_acceptSocketArray[k], m_IOCP, (u_long)0, 0);
				if (NULL == associateAcceptSocketResult)
				{
					return std::make_pair(WSAGetLastError(), __LINE__);
				}
			}

			return std::make_pair(NO_ERROR, 0);
		}
	}
}
