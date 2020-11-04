#pragma once

#include <cstdint>

namespace GenericBoson
{
	namespace ServerEngine
	{
		struct GBBuffer
		{
		public: char m_buffer[1024] = { 0, };
		public: uint32_t m_writeOffset = 0;
		public: uint32_t m_readOffset = 0;
		};
	}
}