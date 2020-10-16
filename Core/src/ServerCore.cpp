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
			u_long completionKey;
			ExpandedOverlapped *expendedOverlapped;

			while(true == m_keepLooping)
			{
				memset(&expendedOverlapped, 0, sizeof(expendedOverlapped));

				BOOL result = GetQueuedCompletionStatus(m_IOCP, &recievedBytes, (PULONG_PTR)&completionKey, (OVERLAPPED**)&expendedOverlapped, INFINITE);

				switch (expendedOverlapped->m_type)
				{
				case IO_TYPE::ACCEPT:
				{
					expendedOverlapped->m_type = IO_TYPE::RECEIVE;
					DWORD flag = 0, receivedBytes = 0;
					WSABUF wsaBuffer;
					wsaBuffer.len = 1024;
					wsaBuffer.buf = expendedOverlapped->m_buffer;
					WSARecv(*expendedOverlapped->m_socket, &wsaBuffer, 1, &flag, &receivedBytes, expendedOverlapped, nullptr);
				}
				break;
				case IO_TYPE::RECEIVE:
				{
					std::cout << "Test" << std::endl;
				}
				break;
				case IO_TYPE::SEND:
				{

				}
				break;
				default:
					assert(false);
				}
			}
		}

		std::pair<int, int> ServerCore::Start(const ServerCreateParameter& param)
		{
			// Copying the parameter as member variable.
			m_createParameter = param;

			// Getting the core number of this machine. 
			m_threadPoolSize = 2 * std::thread::hardware_concurrency();

			// Making a IOCP kernel object.
			m_IOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, (u_long)0, 0);

			if (NULL == m_IOCP)
			{
				return std::make_pair(WSAGetLastError(), __LINE__);
			}

			// Initializing the thread pool.
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

			// Making the listening socket
			m_listenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);
			if (INVALID_SOCKET == m_listenSocket) {
				return std::make_pair(WSAGetLastError(), __LINE__);
			}

			// Associate the listening socket with the IOCP.
			HANDLE associateListenSocketResult = CreateIoCompletionPort((HANDLE)m_listenSocket, m_IOCP, (u_long)0, 0);

			if (NULL == associateListenSocketResult)
			{
				return std::make_pair(WSAGetLastError(), __LINE__);
			}
			
			service.sin_family = AF_INET;
			InetPton(AF_INET, param.m_ipString.c_str(), &service.sin_addr.s_addr);
			service.sin_port = htons(param.m_port);

			// Binding a endpoint to the listening socket.
			result = bind(m_listenSocket, (SOCKADDR*)&service, sizeof(service));
			if (SOCKET_ERROR == result)
			{
				return std::make_pair(WSAGetLastError(), __LINE__);
			}

			DWORD tmpListenBuffer;

			// Make the listening socket listen.
			result = listen(m_listenSocket, SOMAXCONN);
			if (SOCKET_ERROR == result) {
				return std::make_pair(WSAGetLastError(), __LINE__);
			}

			LPFN_ACCEPTEX lpfnAcceptEx = NULL;
			GUID GuidAcceptEx = WSAID_ACCEPTEX;
			DWORD returnedBytes;
			
			const int outBufLength = 1024;
			char outputBuffer[1024];

			// Getting the AcceptEx function pointer.
			result = WSAIoctl(m_listenSocket, SIO_GET_EXTENSION_FUNCTION_POINTER,
				&GuidAcceptEx, sizeof(GuidAcceptEx),
				&lpfnAcceptEx, sizeof(lpfnAcceptEx),
				&returnedBytes, NULL, NULL);
			if (result == SOCKET_ERROR)
			{
				return std::make_pair(WSAGetLastError(), __LINE__);
			}
			
			// Preparing the accept sockets.
			for (int k = 0; k < acceptedSocketArraySize; ++k)
			{
				// Creating an accept socket.
				m_acceptSocketArray[k] = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);
				if (INVALID_SOCKET == m_acceptSocketArray[k])
				{
					return std::make_pair(WSAGetLastError(), __LINE__);
				}

				ExpandedOverlapped* expandedOverlap = new ExpandedOverlapped();

				memset(expandedOverlap, 0, sizeof(ExpandedOverlapped));

				expandedOverlap->m_type = IO_TYPE::ACCEPT;
				expandedOverlap->m_socket = &m_acceptSocketArray[k];

				// Posting an accept operation.
				BOOL result = lpfnAcceptEx(m_listenSocket, m_acceptSocketArray[k], outputBuffer,
					0,//outBufLength - ((sizeof(sockaddr_in) + 16) * 2),
					sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16,
					&returnedBytes, expandedOverlap);
				int lastSocketError = WSAGetLastError();
				if (FALSE == result && ERROR_IO_PENDING != lastSocketError)
				{
					return std::make_pair(lastSocketError, __LINE__);
				}

				// Associate this accept socket withd IOCP.
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
