namespace GenericBoson
{
	namespace ServerEngine
	{
		struct ServerCreateParameter
		{
			int m_port = 0;
		};

		class ServerCore
		{
			ServerCore() = delete;
			ServerCore(const ServerCreateParameter& param);

			SOCKET m_socket;
			HANDLE m_IOCP;

			ServerCreateParameter m_createParameter;

		public:

			static ServerCore* Create(const ServerCreateParameter& param);
		};
	}
}