#pragma once

#include "GBString.h"
#include "Constant.h"

#include <vector>
#include <thread>

namespace GenericBoson
{
	namespace ServerEngine
	{
		struct ServerCreateParameter
		{
			GBString m_ipString = _GBT("127.0.0.1");
			int m_port = 25565;
		};

		class ServerCore
		{
			// If you remove '/100', you will get a compile time error "out of heap".
			static const constexpr int acceptedSocketArraySize = 10;//SOMAXCONN / sizeof(SOCKET) / 100;
			SOCKET m_listenSocket = INVALID_SOCKET, m_acceptSocketArray[acceptedSocketArraySize] = { INVALID_SOCKET, };
			HANDLE m_IOCP = INVALID_HANDLE_VALUE;

			ServerCreateParameter m_createParameter;

			volatile bool m_keepLooping = true;
			int m_threadPoolSize = 0;
			std::vector<std::thread> m_threadPool;

			enum class IO_TYPE : int64_t
			{
				ACCEPT = 1,
				RECEIVE,
				SEND,
			};

			struct ExpandedOverlapped : public WSAOVERLAPPED
			{
				SOCKET* m_socket = nullptr;
				IO_TYPE m_type = IO_TYPE::ACCEPT;
				char m_buffer[1024];
			};

		public:

			virtual ~ServerCore();

			void ThreadFunction();
			std::pair<int, int> Start(const ServerCreateParameter& param);
		};
	}
}