#pragma once

#include <cstdint>

namespace GenericBoson
{
	const int BUFFER_SIZE = 1024;

	struct GBBuffer
	{
	public: char m_buffer[BUFFER_SIZE] = { 0, };
	public: int m_writeOffset = 1;		// first char is message length.
	public: int m_readOffset = 0;

	public: inline void Reset()
	{
		m_writeOffset = 0;
		m_readOffset = 0;
	}
	};
}