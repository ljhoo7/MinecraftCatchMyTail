#pragma once

#include <cstdint>

namespace GenericBoson
{
	const int BUFFER_SIZE = 2048;

	struct GBBuffer
	{
	public: char m_buffer[BUFFER_SIZE] = { 0, };
	public: int m_writeOffset = 0;
	public: int m_readOffset = 0;

	public: inline void Reset()
	{
		m_writeOffset = 0;
		m_readOffset = 0;
	}
	};
}