#pragma once

#include "GBString.h"
#include "Constant.h"
#include "GBBuffer.h"

#include <vector>
#include <thread>
#include <queue>
#include <mutex>

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

				SessionState m_sessionState = SessionState::start;

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

			// for AcceptEx's recv buffer which is not using.
			// Warning : If this is not using, but this must exists till the end.
		private: char m_listenBuffer[1024];

		private: template<typename T> uint32_t ReadByteByByte(char* buffer, T& value);
		private: template<typename STRING> uint32_t ReadString(char* buffer, STRING& outString);
		private: template<typename T> uint32_t Read(char* buffer, T& outValue);
		private: template<typename T> uint32_t WriteByteByByte(char* buffer, T value);
		private: template<typename T> uint32_t Write(char* buffer, const T& outValue);

		public: virtual ~ServerCore();
		public: void ThreadFunction();
		public: int IssueRecv(ExpandedOverlapped* pEol, ULONG lengthToReceive);
		public: int IssueSend(ExpandedOverlapped* pEol);

		public: void SendLoginSuccess();

			// Consuming a gathering completed message.
		public: void ConsumeGatheredMessage(ExpandedOverlapped& eol, char* message, const uint32_t messageSize, uint32_t& readOffSet);

		public: std::pair<int, int> Start(const ServerCreateParameter& param);
		};
	}
}