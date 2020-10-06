#include "stdafx.h"
#include "..\include\ServerCore.h"

namespace GenericBoson
{
	namespace ServerEngine
	{
		ServerCore::ServerCore(const ServerCreateParameter& param)
		{
			m_createParameter = param;

			m_IOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 16);

			// listen

			// acceptex

			CreateIoCompletionPort((HANDLE)m_socket, m_IOCP, 0, 0);
		}

		ServerCore* ServerCore::Create(const ServerCreateParameter& param)
		{
			return new ServerCore(param);
		}
	}
}
