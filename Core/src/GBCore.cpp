#include "stdafx.h"
#include "GBCore.h"

namespace GenericBoson
{
	namespace ServerEngine
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

		void Core::SendStartCompress(ExpandedOverlapped& eol)
		{
			WriteByteByByte(eol, (int32_t)PacketType::StartCompression);
			WriteByteByByte(eol, InternalConstant::CompressThreshold);

			EnqueueAndIssueSend(eol);
		}

		void Core::SendLoginSuccess(ExpandedOverlapped& eol)
		{
			WriteByteByByte(eol, (int32_t)PacketType::LoginSuccess);

			// UUID #ToDo
			eol.m_uuid = "5550AEA5-0443-4C06-A1CB-CF916EA1623D";
			WriteString(eol, eol.m_uuid);
			WriteString(eol, eol.m_userName);

			EnqueueAndIssueSend(eol);
		}

		void Core::SendJoinGame(ExpandedOverlapped& eol)
		{
			WriteByteByByte(eol, (int32_t)PacketType::JoinGame);

			// Sending FermionID
			Write4BytesAsBigEndian(eol, eol.m_controllableCharacter.m_ID);

			uint8_t hardCoreFlag = 0;
			WriteByteByByte(eol, hardCoreFlag);

			Dimension demension = Dimension::overworld;
			Write4BytesAsBigEndian(eol, demension);

			uint8_t difficulty = 2; // 2 = Normal
			WriteByteByByte(eol, difficulty);

			uint8_t maxPlayerCount = 255;
			WriteByteByByte(eol, maxPlayerCount);

			std::string levelType = "default";
			WriteString(eol, levelType);

			uint8_t reducedDebugInfo = 0; // bool
			WriteByteByByte(eol, reducedDebugInfo);

			EnqueueAndIssueSend(eol);
		}

		void Core::SendSpawnSpot(ExpandedOverlapped& eol)
		{
			WriteByteByByte(eol, (int32_t)PacketType::SpawnSpot);

			GBVector3<int> spawnSpot(10, 10, 10);
			WriteIntGBVector3(eol, spawnSpot);

			EnqueueAndIssueSend(eol);
		}

		void Core::SendDifficulty(ExpandedOverlapped& eol)
		{
			WriteByteByByte(eol, (int32_t)PacketType::Difficulty);

			char difficulty = 1;
			WriteByteByByte(eol, difficulty);

			EnqueueAndIssueSend(eol);
		}

		void Core::SendCharacterAbility(ExpandedOverlapped& eol)
		{
			WriteByteByByte(eol, (int32_t)PacketType::CharacterAbility);

			WriteByteByByte(eol, eol.m_controllableCharacter.m_abilityState);

			float correctedFlyingMaxSpeed = 0.05f * eol.m_controllableCharacter.m_flyingMaxSpeed;
			Write4BytesAsBigEndian(eol, correctedFlyingMaxSpeed);

			float correctedSprintingMaxSpeed = 0.05f * eol.m_controllableCharacter.m_sprintingMaxSpeed;
			Write4BytesAsBigEndian(eol, correctedSprintingMaxSpeed);

			EnqueueAndIssueSend(eol);
		}

		void Core::SendTime(ExpandedOverlapped& eol)
		{
			WriteByteByByte(eol, (int32_t)PacketType::Time);
			Write8BytesAsBigEndian(eol, m_world.m_ageMs);

			// false == m_world.m_dayLightEnabled #ToDo

			Write8BytesAsBigEndian(eol, m_world.m_timeOfDay);

			EnqueueAndIssueSend(eol);
		}

		void Core::SendInventory(ExpandedOverlapped& eol)
		{
			WriteByteByByte(eol, (int32_t)PacketType::Inventory);
			WriteByteByByte(eol, eol.m_controllableCharacter.m_inventory.m_ID);
			WriteByteByByte(eol, (int16_t)eol.m_controllableCharacter.m_inventory.GetTotalSlotCount());

			for (auto& pSlot : eol.m_controllableCharacter.m_inventory.m_slotVector)
			{
				// #ToDo
				pSlot->WriteItem();
			}

			EnqueueAndIssueSend(eol);
		}

		void Core::SendHealth(ExpandedOverlapped& eol)
		{
			WriteByteByByte(eol, (int32_t)PacketType::Health);
			Write4BytesAsBigEndian(eol, eol.m_controllableCharacter.m_health);
			WriteByteByByte(eol, eol.m_controllableCharacter.m_foodLevel);
			Write4BytesAsBigEndian(eol, eol.m_controllableCharacter.m_foodSaturationLevel);

			EnqueueAndIssueSend(eol);
		}

		void Core::SendExperience(ExpandedOverlapped& eol)
		{
			WriteByteByByte(eol, (int32_t)PacketType::Experience);

			float xpPercentage = eol.m_controllableCharacter.GetXpPercentage();
			Write4BytesAsBigEndian(eol, xpPercentage);

			int level = eol.m_controllableCharacter.GetLevel();
			WriteByteByByte(eol, (int32_t)PacketType::Experience);
			WriteByteByByte(eol, eol.m_controllableCharacter.m_experience);

			EnqueueAndIssueSend(eol);
		}

		void Core::SendEquippedItem(ExpandedOverlapped& eol)
		{
			WriteByteByByte(eol, (int32_t)PacketType::EquippedItemChange);
			WriteByteByByte(eol, (int8_t)eol.m_controllableCharacter.m_inventory.m_equippedSlotID);

			EnqueueAndIssueSend(eol);
		}

		void Core::SendPlayerList(ExpandedOverlapped& eol)
		{
			WriteByteByByte(eol, (int32_t)PacketType::PlayerList);

			WriteByteByByte(eol, (int32_t)0);

			WriteByteByByte(eol, (int32_t)1);

			WriteString(eol, eol.m_uuid);

			WriteString(eol, eol.m_userName);

			// #ToDo
			// Send Property List Size
			WriteByteByByte(eol, (int32_t)0);

			WriteByteByByte(eol, (int32_t)m_world.m_gameMode);

			// #ToDo
			WriteByteByByte(eol, (int32_t)m_world.m_pingMs);

			WriteByteByByte(eol, (char)0);

			EnqueueAndIssueSend(eol);
		}

		void Core::WriteIntGBVector3(ExpandedOverlapped& eol, const GBVector3<int>& value)
		{
			const uint64_t bitFlag = 0b0000'0000'0000'0000'0000'0000'0000'0000'0000'0011'1111'1111'1111'1111'1111'1111;
			uint64_t spawnSpot = (uint64_t)(value.x & bitFlag) << 38; // 38 is the number of zero in bitFlag!
			spawnSpot |= (uint64_t)(value.y 
				& 0b0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'1111'1111'1111);
			spawnSpot |= (uint64_t)(value.z & bitFlag) << 26; // 26 is the number of one in bitFlag!
			WriteByteByByte(eol, spawnSpot);
		}

		void Core::Write2BytesAsBigEndian(ExpandedOverlapped& eol, uint16_t value)
		{
			uint32_t valueConvertedToBigEndian = htons(value);
			WriteByteByByte(eol, valueConvertedToBigEndian);
		}

		void Core::Write8BytesAsBigEndian(ExpandedOverlapped& eol, uint64_t value)
		{
			uint64_t highWord = htonl((uint32_t)value) << 32;
			uint64_t lowWord = htonl(value >> 32);
			uint64_t valueConvertedToBigEndian = highWord + lowWord;
			WriteByteByByte(eol, valueConvertedToBigEndian);
		}

		void Core::Write4BytesAsBigEndian(ExpandedOverlapped& eol, uint32_t value)
		{
			uint32_t valueConvertedToBigEndian = htonl(value);
			WriteByteByByte(eol, valueConvertedToBigEndian);
		}

		void Core::EnqueueAndIssueSend(ExpandedOverlapped& eol)
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

		void Core::ConsumeGatheredMessage(ExpandedOverlapped& eol, char* message, const uint32_t messageSize, uint32_t& readOffSet)
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

					SendStartCompress(eol);
					SendLoginSuccess(eol);
					SendJoinGame(eol);
					SendSpawnSpot(eol);
					SendDifficulty(eol);
					SendCharacterAbility(eol);

					//SendWeather #ToDo

					SendTime(eol);
					SendInventory(eol);
					SendHealth(eol);
					SendExperience(eol);
					SendEquippedItem(eol);
					SendPlayerList(eol);

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
		uint32_t Core::Read(char* buffer, T& outValue)
		{
			outValue = *(T*)buffer;
			
			return sizeof(T);
		}

		template<typename STRING>
		uint32_t Core::ReadString(char* buffer, STRING& outString)
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
		void Core::WriteString(ExpandedOverlapped& eol, const STRING& inString)
		{
			uint32_t writeByteLength = 0;
			char* buffer = eol.m_writeBuffer.m_buffer;

			// String Length
			char inStringSize = (char)inString.length();
			WriteByteByByte(eol, inStringSize);

			errno_t cpyStrResult = strncpy_s(buffer, 1024, inString.c_str(), inStringSize);
			eol.m_writeBuffer.m_writeOffset += writeByteLength;
			buffer += inStringSize;
		}

		template<typename T>
		uint32_t Core::ReadByteByByte(char* buffer, T& value)
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
		void Core::Write(ExpandedOverlapped& eol, const T& outValue)
		{
			*(T*)buffer = outValue;

			int writeLength = sizeof(T);
			eol.m_writeBuffer.m_buffer += writeLength;
			eol.m_writeBuffer.m_writeOffset += writeLength;
		}

		template<typename T>
		void Core::WriteByteByByte(ExpandedOverlapped& eol, T value)
		{
			char* buffer = eol.m_writeBuffer.m_buffer;

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
				eol.m_writeBuffer.m_writeOffset++;

				value = value >> 7;
			} while (0 < value);
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

		std::atomic<uint32_t> Core::m_fermionCounter = 1;
	}
}
