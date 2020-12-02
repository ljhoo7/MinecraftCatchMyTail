#include "stdafx.h"
#include "GBCore.h"

namespace GenericBoson
{
	Core::~Core()
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

	int Core::IssueRecv(ExpandedOverlapped* pEol, ULONG lengthToReceive)
	{
		pEol->m_type = IO_TYPE::RECEIVE;
		DWORD flag = 0, receivedBytes = 0;
		WSABUF wsaBuffer;
		wsaBuffer.len = lengthToReceive;			// packet length is 1 byte.
		wsaBuffer.buf = &pEol->m_receiveBuffer.m_buffer[pEol->m_receiveBuffer.m_readOffset];
		int recvResult = WSARecv(pEol->m_socket, &wsaBuffer, 1, &flag, &receivedBytes, pEol, nullptr);

		return recvResult;
	}

	int Core::IssueSend(ExpandedOverlapped* pEol)
	{
		return -1;
	}

	void Core::WriteIntGBVector3(GBBuffer* pGbBuffer, const GBVector3<int>& value)
	{
		const uint64_t bitFlag = 0b0000'0000'0000'0000'0000'0000'0000'0000'0000'0011'1111'1111'1111'1111'1111'1111;
		uint64_t spawnSpot = (uint64_t)(value.x & bitFlag) << 38; // 38 is the number of zero in bitFlag!
		spawnSpot |= (uint64_t)(value.y 
			& 0b0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'1111'1111'1111);
		spawnSpot |= (uint64_t)(value.z & bitFlag) << 26; // 26 is the number of one in bitFlag!
		WriteByteByByte(pGbBuffer, spawnSpot);
	}

	void Core::Write2BytesAsBigEndian(GBBuffer* pGbBuffer, uint16_t value)
	{
		uint32_t valueConvertedToBigEndian = htons(value);
		WriteByteByByte(pGbBuffer, valueConvertedToBigEndian);
	}

	void Core::Write8BytesAsBigEndian(GBBuffer* pGbBuffer, uint64_t value)
	{
		uint64_t highWord = htonl((uint32_t)value) << 32;
		uint64_t lowWord = htonl(value >> 32);
		uint64_t valueConvertedToBigEndian = highWord + lowWord;
		WriteByteByByte(pGbBuffer, valueConvertedToBigEndian);
	}

	void Core::Write4BytesAsBigEndian(GBBuffer* pGbBuffer, uint32_t value)
	{
		uint32_t valueConvertedToBigEndian = htonl(value);
		WriteByteByByte(pGbBuffer, valueConvertedToBigEndian);
	}

	void Core::EnqueueAndIssueSend(ExpandedOverlapped* pEol)
	{
		{
			std::lock_guard<std::mutex> lock(m_mainLock);

			m_sendQueue.push(pEol);
		}

		while (false == m_sendQueue.empty())
		{
			{
				auto pEol = m_sendQueue.front();
				m_sendQueue.pop();
				WSABUF sendBuf;
				sendBuf.buf = pEol->m_writeBuffer.m_buffer;
				sendBuf.len = pEol->m_writeBuffer.m_writeOffset;
				DWORD sentBytes = 0;
				int sendResult = WSASend(pEol->m_socket, &sendBuf, 1, &sentBytes, NULL, pEol, NULL);

				if (0 != sendResult)
				{
					std::cout << "[WSASend failed] WSAGetLastError : " << WSAGetLastError() << std::endl;
					assert(false);
				}

				// All write buffer must be set to zero because it will do bit-operations.
				memset(pEol->m_writeBuffer.m_buffer, 0, 1024);
			}

			Sleep(10);
		}
	}

	void Core::ThreadFunction()
	{
		DWORD receivedBytes;
		u_long completionKey;
		ExpandedOverlapped *pEol = nullptr;

		while(true == m_keepLooping)
		{
			BOOL result = GetQueuedCompletionStatus(m_IOCP, &receivedBytes, (PULONG_PTR)&completionKey, (OVERLAPPED**)&pEol, INFINITE);

			switch (pEol->m_type)
			{
			case IO_TYPE::ACCEPT:
			{
				int issueRecvResult = IssueRecv(pEol, 1);
			}
			break;
			case IO_TYPE::RECEIVE:
			{
				if (0 == receivedBytes)
				{
					// Disconnected.
					// #ToDo
					return;
				}

				// Getting the size of a message.
				if (0 == pEol->m_leftBytesToReceive)
				{
					assert(0 == pEol->m_receiveBuffer.m_readOffset);
					assert(1 == receivedBytes);

					pEol->m_receiveBuffer.m_readOffset++;
					pEol->m_leftBytesToReceive = pEol->m_receiveBuffer.m_buffer[0];
					pEol->m_receiveBuffer.m_writeOffset++;

					int issueRecvResult = IssueRecv(pEol, pEol->m_leftBytesToReceive);

					break;
				}

				pEol->m_receiveBuffer.m_writeOffset += receivedBytes;

				// Gathering a message completed.
				if (pEol->m_leftBytesToReceive + 1 == pEol->m_receiveBuffer.m_writeOffset)
				{
					uint32_t messageSize = pEol->m_receiveBuffer.m_writeOffset - pEol->m_receiveBuffer.m_readOffset;
					assert(0 < messageSize);
					ConsumeGatheredMessage(pEol, &pEol->m_receiveBuffer.m_buffer[pEol->m_receiveBuffer.m_readOffset], messageSize, pEol->m_receiveBuffer.m_readOffset);

					// Reset
					pEol->m_receiveBuffer.m_readOffset = 0;
					pEol->m_receiveBuffer.m_writeOffset = 0;
					pEol->m_leftBytesToReceive = 0;

					int issueRecvResult = IssueRecv(pEol, 1);
					break;
				}
				else if (pEol->m_leftBytesToReceive + 1 < pEol->m_receiveBuffer.m_writeOffset)
				{
					break;
				}
				else
				{
					throw new std::exception("Internal logic error.");
				}
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

	std::pair<int, int> Core::Start(const ServerCreateParameter& param)
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

		// Make the listening socket listen.
		result = listen(m_listenSocket, SOMAXCONN);
		if (SOCKET_ERROR == result) {
			return std::make_pair(WSAGetLastError(), __LINE__);
		}

		LPFN_ACCEPTEX lpfnAcceptEx = NULL;
		GUID GuidAcceptEx = WSAID_ACCEPTEX;
		DWORD returnedBytes;

		// Getting the AcceptEx function pointer.
		result = WSAIoctl(m_listenSocket, SIO_GET_EXTENSION_FUNCTION_POINTER,
			&GuidAcceptEx, sizeof(GuidAcceptEx),
			&lpfnAcceptEx, sizeof(lpfnAcceptEx),
			&returnedBytes, NULL, NULL);
		if (result == SOCKET_ERROR)
		{
			return std::make_pair(WSAGetLastError(), __LINE__);
		}

		ExpandedOverlapped* m_extendedOverlappedArray = nullptr;
		m_extendedOverlappedArray = (ExpandedOverlapped*)GetSessionInformationArray();
			
		// Preparing the accept sockets.
		for (int k = 0; k < EXTENDED_OVERLAPPED_ARRAY_SIZE; ++k)
		{
			// Creating an accept socket.
			m_extendedOverlappedArray[k].m_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);
			if (INVALID_SOCKET == m_extendedOverlappedArray[k].m_socket)
			{
				return std::make_pair(WSAGetLastError(), __LINE__);
			}

			m_extendedOverlappedArray[k].m_type = IO_TYPE::ACCEPT;

			// Posting an accept operation.
			BOOL result = lpfnAcceptEx(m_listenSocket, m_extendedOverlappedArray[k].m_socket, m_listenBuffer, 0,
				sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16,
				&returnedBytes, &m_extendedOverlappedArray[k]);
			int lastSocketError = WSAGetLastError();
			if (FALSE == result && ERROR_IO_PENDING != lastSocketError)
			{
				return std::make_pair(lastSocketError, __LINE__);
			}

			// Associate this accept socket withd IOCP.
			HANDLE associateAcceptSocketResult = CreateIoCompletionPort((HANDLE)m_extendedOverlappedArray[k].m_socket, m_IOCP, (u_long)0, 0);
			if (NULL == associateAcceptSocketResult)
			{
				return std::make_pair(WSAGetLastError(), __LINE__);
			}
		}

		return std::make_pair(NO_ERROR, 0);
	}

	std::atomic<uint32_t> Core::m_fermionCounter = 1;
}
