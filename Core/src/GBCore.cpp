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
		// Warning : Don't change the below type 'int64_t' to 'uint64_t'.
		int64_t
		spawnSpot = (int64_t)(value.x & BIT_FLAG_FOR_VECTOR_XZ) << 38; // 38 is the number of zero in bitFlag!
		spawnSpot |= (int64_t)(value.y & BIT_FLAG_FOR_VECTOR_Y) << 26; // 26 is the number of one in bitFlag!
		spawnSpot |= (int64_t)(value.z & BIT_FLAG_FOR_VECTOR_XZ);
		Write8BytesAsBigEndian_Signed(pGbBuffer, spawnSpot);
	}

	uint32_t Core::ReadIntGBVector3(GBBuffer* pGbBuffer, GBVector3<int>& value)
	{
		uint32_t readByteLength = 0;

		// Warning : Don't change the below type 'int64_t' to 'uint64_t'.
		int64_t vectorChunk = Read8BytesAsBigEndian_Signed(pGbBuffer);
		readByteLength += sizeof(vectorChunk);

		uint32_t xRaw = (vectorChunk >> 38) & BIT_FLAG_FOR_VECTOR_XZ;
		uint32_t yRaw = (vectorChunk >> 26) & BIT_FLAG_FOR_VECTOR_Y;
		uint32_t zRaw = (vectorChunk & BIT_FLAG_FOR_VECTOR_XZ);

		value.x = (int)xRaw;
		value.y = (int)yRaw;
		value.z = (int)zRaw;

		// If it should be converted into negative, do it now.
		if ((xRaw & 0x02000000) != 0)
		{
			value.x = 0x04000000 - value.x;
			value.x *= -1;
		}

		if ((yRaw & 0x0800) != 0)
		{
			value.y = 0x01000 - value.y;
			value.y *= -1;
		}

		if ((zRaw & 0x02000000) != 0)
		{
			value.z = 0x04000000 - value.z;
			value.z *= -1;
		}

		return readByteLength;
	}

	void Core::Write2BytesAsBigEndian_Without_Sign(GBBuffer* pGbBuffer, uint16_t value)
	{
		uint32_t valueConvertedToBigEndian = htons(value);

		uint16_t* pTwoBytes = AssignFromBufferForWrite<uint16_t>(pGbBuffer);
		*pTwoBytes = valueConvertedToBigEndian;
	}

	void Core::Write4BytesAsBigEndian_Without_Sign(GBBuffer* pGbBuffer, uint32_t value)
	{
		uint32_t valueConvertedToBigEndian = htonl(value);

		uint32_t* pFourBytes = AssignFromBufferForWrite<uint32_t>(pGbBuffer);
		*pFourBytes = valueConvertedToBigEndian;
	}

	void Core::Write8BytesAsBigEndian_Without_Sign(GBBuffer* pGbBuffer, uint64_t value)
	{
		/*uint64_t highWord = htonl((uint32_t)value) << 32;
		uint64_t lowWord = htonl(value >> 32);
		uint64_t valueConvertedToBigEndian = highWord + lowWord;*/
		
		uint64_t valueConvertedToBigEndian = htonll(value);

		uint64_t* pEightBytes = AssignFromBufferForWrite<uint64_t>(pGbBuffer);
		*pEightBytes = valueConvertedToBigEndian;
	}

	void Core::Write2BytesAsBigEndian_Signed(GBBuffer* pGbBuffer, int16_t value)
	{
		Write2BytesAsBigEndian_Without_Sign(pGbBuffer, value);
	}

	void Core::Write4BytesAsBigEndian_Signed(GBBuffer* pGbBuffer, int32_t value)
	{
		Write4BytesAsBigEndian_Without_Sign(pGbBuffer, value);
	}

	void Core::Write8BytesAsBigEndian_Signed(GBBuffer* pGbBuffer, int64_t value)
	{
		Write8BytesAsBigEndian_Without_Sign(pGbBuffer, value);
	}

	int8_t Core::Read1BytesAsBigEndian_Signed(GBBuffer* pGbBuffer)
	{
		int8_t ret = 0;
		uint8_t tmpUnsigned = Read1BytesAsBigEndian_Without_Sign(pGbBuffer);
		memcpy_s(&ret, sizeof(ret), &tmpUnsigned, sizeof(tmpUnsigned));

		return ret;
	}

	int16_t Core::Read2BytesAsBigEndian_Signed(GBBuffer* pGbBuffer)
	{
		int16_t ret = 0;
		uint16_t tmpUnsigned = Read2BytesAsBigEndian_Without_Sign(pGbBuffer);
		memcpy_s(&ret, sizeof(ret), &tmpUnsigned, sizeof(tmpUnsigned));

		return ret;
	}

	int32_t Core::Read4BytesAsBigEndian_Signed(GBBuffer* pGbBuffer)
	{
		int32_t ret = 0;
		uint32_t tmpUnsigned = Read4BytesAsBigEndian_Without_Sign(pGbBuffer);
		memcpy_s(&ret, sizeof(ret), &tmpUnsigned, sizeof(tmpUnsigned));

		return ret;
	}

	int64_t Core::Read8BytesAsBigEndian_Signed(GBBuffer* pGbBuffer)
	{
		int64_t ret = 0;
		uint64_t tmpUnsigned = Read8BytesAsBigEndian_Without_Sign(pGbBuffer);
		memcpy_s(&ret, sizeof(ret), &tmpUnsigned, sizeof(tmpUnsigned));

		return ret;
	}

	uint8_t Core::Read1BytesAsBigEndian_Without_Sign(GBBuffer* pGbBuffer)
	{
		uint8_t* pOneBytes = AssignFromBufferForRead<uint8_t>(pGbBuffer);
		return *pOneBytes;
	}

	uint16_t Core::Read2BytesAsBigEndian_Without_Sign(GBBuffer* pGbBuffer)
	{
		uint16_t* pTwoBytes = AssignFromBufferForRead<uint16_t>(pGbBuffer);
		return ntohs(*pTwoBytes);
	}

	uint32_t Core::Read4BytesAsBigEndian_Without_Sign(GBBuffer* pGbBuffer)
	{
		uint32_t* pFourBytes = AssignFromBufferForRead<uint32_t>(pGbBuffer);
		return ntohl(*pFourBytes);
	}

	void Core::WriteFloat(GBBuffer* pGbBuffer, float value)
	{
		uint32_t tmpBytes = 0;
		memcpy_s(&tmpBytes, sizeof(tmpBytes), &value, sizeof(value));
		Write4BytesAsBigEndian_Without_Sign(pGbBuffer, tmpBytes);
	}

	float Core::ReadFloat(GBBuffer* pGbBuffer)
	{
		float returnValue;
		uint32_t endianChangedFlyingMaxSpeed = Read4BytesAsBigEndian_Without_Sign(pGbBuffer);

		// Strangely, 'returnValue = (float)endianChangedFlyingMaxSpeed' does not work correctly.
		memcpy_s(&returnValue, 4, &endianChangedFlyingMaxSpeed, 4);

		return returnValue;
	}

	uint64_t Core::Read8BytesAsBigEndian_Without_Sign(GBBuffer* pGbBuffer)
	{
		uint64_t* pEightBytes = AssignFromBufferForRead<uint64_t>(pGbBuffer);

		return ntohll(*pEightBytes);
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
