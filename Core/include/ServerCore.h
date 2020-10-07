#include "GBString.h"
#include "Constant.h"

namespace GenericBoson
{
	namespace ServerEngine
	{
		struct ServerCreateParameter
		{
			std::string m_ipString;
			int m_port = 0;
		};

		class ServerCore
		{
			SOCKET m_listenSocket = INVALID_SOCKET;
			HANDLE m_IOCP = INVALID_HANDLE_VALUE;

			ServerCreateParameter m_createParameter;

		public:

			virtual ~ServerCore();

			std::pair<int, GBString> Start(const ServerCreateParameter& param);
		};
	}
}