#include "GBString.h"
#include "Constant.h"

namespace GenericBoson
{
	namespace ServerEngine
	{
		struct ServerCreateParameter
		{
			GBString m_ipString;
			int m_port = 0;
		};

		class ServerCore
		{
			// If you remove '/100', you will get a compile time error "out of heap".
			static const constexpr int acceptedSocketArraySize = SOMAXCONN / sizeof(SOCKET) / 100;
			SOCKET m_listenSocket = INVALID_SOCKET, m_acceptSocketArray[acceptedSocketArraySize] = { INVALID_SOCKET, };
			HANDLE m_IOCP = INVALID_HANDLE_VALUE;

			ServerCreateParameter m_createParameter;

			int m_threadPoolSize = 0;
			
			void ThreadFunction();
			std::vector<std::thread> m_threadPool;

			enum class IO_TYPE : int64_t
			{
				ACCEPT,
				RECEIVE,
				SEND,
			};

			struct ExpandedOverlapped : public OVERLAPPED
			{

			};

		public:

			virtual ~ServerCore();

			std::pair<int, GBString> Start(const ServerCreateParameter& param);
		};
	}
}