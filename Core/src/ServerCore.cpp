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

		int ServerCore::IssueRecv(ExpandedOverlapped* pEol, ULONG lengthToReceive)
		{
			pEol->m_type = IO_TYPE::RECEIVE;
			DWORD flag = 0, receivedBytes = 0;
			WSABUF wsaBuffer;
			wsaBuffer.len = lengthToReceive;			// packet length is 1 byte.
			wsaBuffer.buf = &pEol->m_receiveBuffer.m_buffer[pEol->m_receiveBuffer.m_readOffset];
			int recvResult = WSARecv(pEol->m_socket, &wsaBuffer, 1, &flag, &receivedBytes, pEol, nullptr);

			return recvResult;
		}

		int ServerCore::IssueSend(ExpandedOverlapped* pEol)
		{
			return -1;
		}

		void ServerCore::SendStartCompress(ExpandedOverlapped& eol, char* bufferToSend, uint32_t& writeOffSet)
		{
			uint32_t wr0 = WriteByteByByte(bufferToSend, (uint32_t)PacketType::StartCompression);
			writeOffSet += wr0;
			bufferToSend += wr0;

			uint32_t wr1 = WriteByteByByte(bufferToSend, InternalConstant::CompressThreshold);
			writeOffSet += wr1;
			bufferToSend += wr1;

			EnqueueAndIssueSend(eol);
		}

		void ServerCore::SendLoginSuccess(ExpandedOverlapped& eol, char* bufferToSend, uint32_t& writeOffSet)
		{
			uint32_t wr0 = WriteByteByByte(bufferToSend, (uint32_t)PacketType::LoginSuccess);
			writeOffSet += wr0;
			bufferToSend += wr0;

			// UUID #ToDo
			std::string tmpUUDI = "5550AEA5-0443-4C06-A1CB-CF916EA1623D";
			uint32_t wr1 = WriteString(bufferToSend, tmpUUDI);
			writeOffSet += wr1;
			bufferToSend += wr1;

			uint32_t wr2 = WriteString(bufferToSend, eol.m_userName);
			writeOffSet += wr2;
			bufferToSend += wr2;

			EnqueueAndIssueSend(eol);
		}

		void ServerCore::SendJoinGame(ExpandedOverlapped& eol, char* bufferToSend, uint32_t& writeOffSet)
		{
			uint32_t wr0 = WriteByteByByte(bufferToSend, (uint32_t)PacketType::JoinGame);
			writeOffSet += wr0;
			bufferToSend += wr0;

			// Sending FermionID
			uint32_t wr1 = Write4BytesAsBigEndian(bufferToSend, eol.m_controllableCharacter.m_ID);
			writeOffSet += wr1;
			bufferToSend += wr1;

			uint8_t hardCoreFlag = 0;
			uint32_t wr2 = WriteByteByByte(bufferToSend, hardCoreFlag);
			writeOffSet += wr2;
			bufferToSend += wr2;

			Dimension demension = Dimension::overworld;
			uint32_t wr3 = Write4BytesAsBigEndian(bufferToSend, demension);
			writeOffSet += wr3;
			bufferToSend += wr3;

			uint8_t difficulty = 2; // 2 = Normal
			uint32_t wr4 = WriteByteByByte(bufferToSend, difficulty);
			writeOffSet += wr4;
			bufferToSend += wr4;

			uint8_t maxPlayerCount = 255;
			uint32_t wr5 = WriteByteByByte(bufferToSend, maxPlayerCount);
			writeOffSet += wr5;
			bufferToSend += wr5;

			std::string levelType = "default";
			uint32_t wr6 = WriteString(bufferToSend, levelType);
			writeOffSet += wr6;
			bufferToSend += wr6;

			uint8_t reducedDebugInfo = 0; // bool
			uint32_t wr7 = WriteByteByByte(bufferToSend, reducedDebugInfo);
			writeOffSet += wr7;
			bufferToSend += wr7;

			EnqueueAndIssueSend(eol);
		}

		void ServerCore::SendSpawnSpot(ExpandedOverlapped& eol, char* bufferToSend, uint32_t& writeOffSet)
		{
			uint32_t wr0 = WriteByteByByte(bufferToSend, (uint32_t)PacketType::SpawnSpot);
			writeOffSet += wr0;
			bufferToSend += wr0;

			GBVector3<int> spawnSpot(10, 10, 10);
			uint32_t wr1 = WriteIntGBVector3(bufferToSend, spawnSpot);
			writeOffSet += wr1;
			bufferToSend += wr1;
		}

		void ServerCore::SendDifficulty(ExpandedOverlapped& eol, char* bufferToSend, uint32_t& writeOffSet)
		{
			uint32_t wr0 = WriteByteByByte(bufferToSend, (uint32_t)PacketType::Difficulty);
			writeOffSet += wr0;
			bufferToSend += wr0;

			char difficulty = 1;
			uint32_t wr1 = WriteByteByByte(bufferToSend, difficulty);
			writeOffSet += wr1;
			bufferToSend += wr1;
		}

		void ServerCore::SendCharacterAbility(ExpandedOverlapped& eol, char* bufferToSend, uint32_t& writeOffSet)
		{
			uint32_t wr0 = WriteByteByByte(bufferToSend, (uint32_t)PacketType::CharacterAbility);
			writeOffSet += wr0;
			bufferToSend += wr0;

			uint32_t wr1 = WriteByteByByte(bufferToSend, eol.m_controllableCharacter.m_abilityState);
			writeOffSet += wr1;
			bufferToSend += wr1;

			float correctedFlyingMaxSpeed = 0.05f * eol.m_controllableCharacter.m_flyingMaxSpeed;
			uint32_t wr2 = Write4BytesAsBigEndian(bufferToSend, correctedFlyingMaxSpeed);
			writeOffSet += wr2;
			bufferToSend += wr2;

			float correctedSprintingMaxSpeed = 0.05f * eol.m_controllableCharacter.m_sprintingMaxSpeed;
			uint32_t wr3 = Write4BytesAsBigEndian(bufferToSend, correctedSprintingMaxSpeed);
			writeOffSet += wr3;
			bufferToSend += wr3;
		}

		void ServerCore::SendTime(ExpandedOverlapped& eol, char* bufferToSend, uint32_t& writeOffSet)
		{
			uint32_t wr0 = WriteByteByByte(bufferToSend, (uint32_t)PacketType::Time);
			writeOffSet += wr0;
			bufferToSend += wr0;

			uint32_t wr1 = Write8BytesAsBigEndian(bufferToSend, m_world.m_ageMs);
			writeOffSet += wr1;
			bufferToSend += wr1;

			// false == m_world.m_dayLightEnabled #ToDo

			uint32_t wr2 = Write8BytesAsBigEndian(bufferToSend, m_world.m_timeOfDay);
			writeOffSet += wr2;
			bufferToSend += wr2;
		}

		void ServerCore::SendInventory(ExpandedOverlapped& eol, char* bufferToSend, uint32_t& writeOffSet)
		{
			uint32_t wr0 = WriteByteByByte(bufferToSend, (uint32_t)PacketType::Inventory);
			writeOffSet += wr0;
			bufferToSend += wr0;

			uint32_t wr1 = WriteByteByByte(bufferToSend, eol.m_controllableCharacter.m_inventory.m_ID);
			writeOffSet += wr1;
			bufferToSend += wr1;

			uint32_t wr2 = WriteByteByByte(bufferToSend, (int16_t)eol.m_controllableCharacter.m_inventory.GetTotalSlotCount());
			writeOffSet += wr2;
			bufferToSend += wr2;

			for (auto& pSlot : eol.m_controllableCharacter.m_inventory.m_slotVector)
			{
				// #ToDo
				pSlot->WriteItem();
			}
		}

		uint32_t ServerCore::WriteIntGBVector3(char* buffer, const GBVector3<int>& value)
		{
			const uint64_t bitFlag = 0b0000'0000'0000'0000'0000'0000'0000'0000'0000'0011'1111'1111'1111'1111'1111'1111;
			uint64_t spawnSpot = (uint64_t)(value.x & bitFlag) << 38; // 38 is the number of zero in bitFlag!
			spawnSpot |= (uint64_t)(value.y 
				& 0b0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'1111'1111'1111);
			spawnSpot |= (uint64_t)(value.z & bitFlag) << 26; // 26 is the number of one in bitFlag!
			return WriteByteByByte(buffer, spawnSpot);
		}

		uint32_t ServerCore::Write2BytesAsBigEndian(char* buffer, uint16_t value)
		{
			uint32_t valueConvertedToBigEndian = htons(value);
			return WriteByteByByte(buffer, valueConvertedToBigEndian);
		}

		uint32_t ServerCore::Write8BytesAsBigEndian(char* buffer, uint64_t value)
		{
			uint64_t highWord = htonl((uint32_t)value) << 32;
			uint64_t lowWord = htonl(value >> 32);
			uint64_t valueConvertedToBigEndian = highWord + lowWord;
			return WriteByteByByte(buffer, valueConvertedToBigEndian);
		}

		uint32_t ServerCore::Write4BytesAsBigEndian(char* buffer, uint32_t value)
		{
			uint32_t valueConvertedToBigEndian = htonl(value);
			return WriteByteByByte(buffer, valueConvertedToBigEndian);
		}

		void ServerCore::EnqueueAndIssueSend(ExpandedOverlapped& eol)
		{
			{
				std::lock_guard<std::mutex> lock(m_mainLock);

				m_sendQueue.push(&eol);
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

					// All write buffer must be set to zero because it will do bit-operations.
					memset(pEol->m_writeBuffer.m_buffer, 0, 1024);
				}

				Sleep(10);
			}
		}

		void ServerCore::ConsumeGatheredMessage(ExpandedOverlapped& eol, char* message, const uint32_t messageSize, uint32_t& readOffSet)
		{
			// Packet Type
			char packetType = 0;
			uint32_t readPacketTypeByteLength = ReadByteByByte(message, packetType);
			readOffSet += readPacketTypeByteLength;
			message += readPacketTypeByteLength;

			switch(eol.m_sessionState)
			{
			case SessionState::login:
			{
				switch ((PacketType)packetType)
				{
				case PacketType::LoginStart:
				{
					// Server Address
					std::string userName;
					uint32_t rr = ReadString(message, userName);
					readOffSet += rr;
					message += rr;

					eol.m_userName = userName;

					SendStartCompress(eol, eol.m_writeBuffer.m_buffer, eol.m_writeBuffer.m_writeOffset);
					SendLoginSuccess(eol, eol.m_writeBuffer.m_buffer, eol.m_writeBuffer.m_writeOffset);
					SendJoinGame(eol, eol.m_writeBuffer.m_buffer, eol.m_writeBuffer.m_writeOffset);
					SendSpawnSpot(eol, eol.m_writeBuffer.m_buffer, eol.m_writeBuffer.m_writeOffset);
					SendDifficulty(eol, eol.m_writeBuffer.m_buffer, eol.m_writeBuffer.m_writeOffset);
					SendCharacterAbility(eol, eol.m_writeBuffer.m_buffer, eol.m_writeBuffer.m_writeOffset);

					//SendWeather #ToDo

					SendTime(eol, eol.m_writeBuffer.m_buffer, eol.m_writeBuffer.m_writeOffset);
					SendInventory(eol, eol.m_writeBuffer.m_buffer, eol.m_writeBuffer.m_writeOffset);
					//SendHealth();
					//SendExp();
					//SendActiveSlot();
					//SendPlayerListAndAddPlayer();

					//eol.m_sessionState = authed;
				}
				break;
				default:
					assert(false);
				}
			}
			break;
			case SessionState::start:
			{
				// Protocol Version
				short protocolVersion = 0;
				uint32_t rr1 = ReadByteByByte(message, protocolVersion);
				readOffSet += rr1;
				message += rr1;

				eol.m_protocolVersion = protocolVersion;

				// Server Address
				std::string serverAddressStr;
				uint32_t rr2 = ReadString(message, serverAddressStr);
				readOffSet += rr2;
				message += rr2;

				// Server Port
				uint16_t portNumber;
				uint32_t rr3 = Read(message, portNumber);
				portNumber = ntohs(portNumber);
				readOffSet += rr3;
				message += rr3;

				// Next Stage
				char nextStage = 0;
				uint32_t rr4 = ReadByteByByte(message, nextStage);
				readOffSet += rr4;
				message += rr4;

				eol.m_sessionState = (SessionState)nextStage;
			}
			break;
			case SessionState::in_game:
			{
				switch ((PacketType)packetType)
				{
				case PacketType::ClientSettings:
				{
					// Server Address
					std::string localeString;
					uint32_t rr = ReadString(message, localeString);
					readOffSet += rr;
					message += rr;
				}
				break;
				default:
					assert(false);
				}
			}
			break;
			default:
				assert(false);
				break;
			}
		}

		template<typename T>
		uint32_t ServerCore::Read(char* buffer, T& outValue)
		{
			outValue = *(T*)buffer;
			
			return sizeof(T);
		}

		template<typename STRING>
		uint32_t ServerCore::ReadString(char* buffer, STRING& outString)
		{
			uint32_t readByteLength = 0;

			// String Length
			char stringLength = 0;
			uint32_t rr1 = ReadByteByByte(buffer, stringLength);
			readByteLength += rr1;
			buffer += rr1;

			outString.reserve(stringLength);
			outString.assign(buffer, stringLength);

			readByteLength += stringLength;
			buffer += stringLength;

			return readByteLength;
		}

		template<typename STRING>
		uint32_t ServerCore::WriteString(char* buffer, const STRING& inString)
		{
			uint32_t writeByteLength = 0;

			// String Length
			size_t inStringSize = inString.length();
			uint32_t wr1 = WriteByteByByte(buffer, inStringSize);
			writeByteLength += wr1;
			buffer += wr1;

			errno_t cpyStrResult = strncpy_s(buffer, 1024, inString.c_str(), inStringSize);
			writeByteLength += inStringSize;
			buffer += inStringSize;

			return writeByteLength;
		}

		template<typename T>
		uint32_t ServerCore::ReadByteByByte(char* buffer, T& value)
		{
			int shift = 0;
			uint32_t readByteLength = 0;

			// the read MSB means the sign of keeping going.
			unsigned char MSB = 0;
			do
			{
				readByteLength++;
				unsigned char byteForBuffer = *buffer;
				value = value | ((static_cast<T>(byteForBuffer & 0b0111'1111)) << shift);
				shift += 7;
				MSB = byteForBuffer & 0b1000'0000;
				buffer++;
			} while (0 != MSB);

			return readByteLength;
		}

		template<typename T>
		uint32_t ServerCore::Write(char* buffer, const T& outValue)
		{
			*(T*)buffer = outValue;

			return sizeof(T);
		}

		template<typename T>
		uint32_t ServerCore::WriteByteByByte(char* buffer, T value)
		{
			uint32_t writeByteLength = 0;

			do 
			{
				unsigned char MSB = 0;

				if (0b0111'1111 < value)
				{
					MSB = 0b1000'0000;
				}

				*buffer = (char)(value & 0b0111'1111);
				*buffer = (char)(value | MSB);

				buffer++;
				value = value >> 7;
			} while (0 < value);

			return writeByteLength;
		}

		void ServerCore::ThreadFunction()
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
						ConsumeGatheredMessage(*pEol, &pEol->m_receiveBuffer.m_buffer[pEol->m_receiveBuffer.m_readOffset], messageSize, pEol->m_receiveBuffer.m_readOffset);

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
			
			// Preparing the accept sockets.
			for (int k = 0; k < EXTENDED_OVERLAPPED_ARRAY_SIZE; ++k)
			{
				memset(&m_extendedOverlappedArray[k], 0, sizeof(ExpandedOverlapped));

				// for debugging
				//memset(m_extendedOverlappedArray[k].m_buffer, 255, 1024);

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

		std::atomic<uint32_t> ServerCore::m_fermionCounter = 1;
	}
}
