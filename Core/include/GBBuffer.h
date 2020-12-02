#pragma once

#include <cstdint>

namespace GenericBoson
{
	struct GBBuffer
	{
	public: char m_buffer[1024] = { 0, };
	public: int m_writeOffset = 0;
	public: int m_readOffset = 0;
	};
}