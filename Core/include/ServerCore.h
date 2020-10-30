#pragma once

#include "GBString.h"
#include "Constant.h"

#include <vector>
#include <thread>

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
			enum class IO_TYPE : int64_t
			{
				ACCEPT = 1,
				RECEIVE,
				SEND,
			};

			struct ExpandedOverlapped : public WSAOVERLAPPED
			{
				SOCKET m_socket = INVALID_SOCKET;
				IO_TYPE m_type = IO_TYPE::ACCEPT;
				uint32_t m_leftBytesToReceive = 0, m_writeOffset = 0, m_readOffset = 0;

				// #ToDo
				// This must be exchanged with a circular lock-free queue.
				char m_buffer[1024];

				SessionState m_sessionState = SessionState::start;

				short m_protocolVersion = 0;
			};

			// If you remove '/100', you will get a compile time error "out of heap".
			static constexpr int EXTENDED_OVERLAPPED_ARRAY_SIZE = SOMAXCONN / sizeof(ExpandedOverlapped) / 100;
			SOCKET m_listenSocket = INVALID_SOCKET;
			ExpandedOverlapped m_extendedOverlappedArray[EXTENDED_OVERLAPPED_ARRAY_SIZE];
			HANDLE m_IOCP = INVALID_HANDLE_VALUE;

			ServerCreateParameter m_createParameter;

			volatile bool m_keepLooping = true;
			int m_threadPoolSize = 0;
			std::vector<std::thread> m_threadPool;

			// for AcceptEx's recv buffer which is not using.
			// Warning : If this is not using, but this must exists till the end.
			char m_listenBuffer[1024];

			template<typename T>
			uint32_t ReadByteByByte(char* buffer, T& value);
			
			template<typename STRING>
			uint32_t ReadString(char* buffer, STRING& outString);

			template<typename T>
			uint32_t Read(char* buffer, T& outValue);
		public:

			virtual ~ServerCore();

			void ThreadFunction();
			int IssueRecv(ExpandedOverlapped* pEol, ULONG lengthToReceive);
			int IssueSend(ExpandedOverlapped* pEol);

			// Consuming a gathering completed message.
			void ConsumeGatheredMessage(ExpandedOverlapped& eol, char* message, const uint32_t messageSize, uint32_t& readOffSet);

			std::pair<int, int> Start(const ServerCreateParameter& param);
		};
	}
}