#pragma once

#include "GBString.h"
#include "Constant.h"
#include "GBBuffer.h"
#include "PacketType.h"
#include "Character.h"
#include "GBVector.h"

#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <atomic>

namespace GenericBoson
{
	namespace ServerEngine
	{
		enum class SessionState : char
		{
			start = 0,
			status,
			login,
			in_game,
		};

		enum Dimension
		{
			nether = -1,
			overworld = 0,
			end = 1,
			notSet = 255,
		};

		struct ServerCreateParameter
		{
			GBString m_ipString = _GBT("127.0.0.1");
			int m_port = 25565;
		};

		class ServerCore
		{
		private: enum class IO_TYPE : int64_t
			{
				ACCEPT = 1,
				RECEIVE,
				SEND,
			};

		private: struct ExpandedOverlapped : public WSAOVERLAPPED
			{
				SOCKET m_socket = INVALID_SOCKET;
				IO_TYPE m_type = IO_TYPE::ACCEPT;
				uint32_t m_leftBytesToReceive = 0;

				// #ToDo
				// This must be exchanged with a circular lock-free queue.
				GBBuffer m_receiveBuffer;
				GBBuffer m_writeBuffer;

				std::string m_userName;

				SessionState m_sessionState = SessionState::start;

				Character m_controllableCharacter;

				short m_protocolVersion = 0;
			};

			// If you remove '/100', you will get a compile time error "out of heap".
		private: static constexpr int EXTENDED_OVERLAPPED_ARRAY_SIZE = SOMAXCONN / sizeof(ExpandedOverlapped) / 200;
		private: SOCKET m_listenSocket = INVALID_SOCKET;
		private: ExpandedOverlapped m_extendedOverlappedArray[EXTENDED_OVERLAPPED_ARRAY_SIZE];
		private: HANDLE m_IOCP = INVALID_HANDLE_VALUE;

		private: ServerCreateParameter m_createParameter;

		private: volatile bool m_keepLooping = true;
		private: int m_threadPoolSize = 0;
		private: std::vector<std::thread> m_threadPool;

		private: std::queue<ExpandedOverlapped*> m_sendQueue;
		private: std::mutex m_mainLock;

		private: static std::atomic<uint32_t> m_fermionCounter;

		// for AcceptEx's recv buffer which is not using.
		// Warning : If this is not using, but this must exists till the end.
		private: char m_listenBuffer[1024];

		private: template<typename T> uint32_t ReadByteByByte(char* buffer, T& value);
		private: template<typename STRING> uint32_t ReadString(char* buffer, STRING& outString);
		private: template<typename T> uint32_t Read(char* buffer, T& outValue);
		private: template<typename T> uint32_t WriteByteByByte(char* buffer, T value);
		private: template<typename STRING> uint32_t WriteString(char* buffer, const STRING& inString);
		private: template<typename T> uint32_t Write(char* buffer, const T& outValue);
		private: uint32_t Write2BytesAsBigEndian(char* buffer, uint16_t value);
		private: uint32_t Write4BytesAsBigEndian(char* buffer, uint32_t value);
		private: uint32_t Write8BytesAsBigEndian(char* buffer, uint64_t value);
		private: uint32_t WriteIntGBVector3(char* buffer, const GBVector3<int>& value);

		public: virtual ~ServerCore();
		public: void ThreadFunction();
		public: int IssueRecv(ExpandedOverlapped* pEol, ULONG lengthToReceive);
		public: int IssueSend(ExpandedOverlapped* pEol);

		public: void SendStartCompress(ExpandedOverlapped& eol, char* bufferToSend, uint32_t& writeOffSet);
		public: void SendLoginSuccess(ExpandedOverlapped& eol, char* bufferToSend, uint32_t& writeOffSet);
		public: void SendJoinGame(ExpandedOverlapped& eol, char* bufferToSend, uint32_t& writeOffSet);
		public: void SendSpawnSpot(ExpandedOverlapped& eol, char* bufferToSend, uint32_t& writeOffSet);
		public: void SendDifficulty(ExpandedOverlapped& eol, char* bufferToSend, uint32_t& writeOffSet);
		public: void SendCharacterAbility(ExpandedOverlapped& eol, char* bufferToSend, uint32_t& writeOffSet);
		public: void EnqueueAndIssueSend(ExpandedOverlapped& eol);

			// Consuming a gathering completed message.
		public: void ConsumeGatheredMessage(ExpandedOverlapped& eol, char* mescsage, const uint32_t messageSize, uint32_t& readOffSet);

		public: std::pair<int, int> Start(const ServerCreateParameter& param);
		};
	}
}