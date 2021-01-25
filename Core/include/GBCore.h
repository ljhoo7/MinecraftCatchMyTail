#pragma once

#include "stdafx.h"

#include "GBString.h"
#include "Constant.h"
#include "GBBuffer.h"
#include "PacketType.h"
#include "GBVector.h"
#include "GBUUID.h"

#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <atomic>

namespace GenericBoson
{
	enum class SessionState : char
	{
		start = 0,
		status,
		login,
		in_game,
	};

	enum Dimension
	{
		nether = -1,
		overworld = 0,
		end = 1,
		notSet = 255,
	};

	struct ServerCreateParameter
	{
		GBString m_ipString = _GBT("127.0.0.1");
		int m_port = 25565;
	};

	class Core
	{
	private: enum class IO_TYPE : int64_t
		{
			ACCEPT = 1,
			RECEIVE,
			SEND,
		};

	protected: struct ExpandedOverlapped : public WSAOVERLAPPED
		{
			SOCKET m_socket = INVALID_SOCKET;
			IO_TYPE m_type = IO_TYPE::ACCEPT;
			uint32_t m_leftBytesToReceive = 0;

			// #ToDo
			// This must be exchanged with a circular lock-free queue.
			GBBuffer m_receiveBuffer;
			GBBuffer m_writeBuffer;
		};

		// If you remove '/100', you will get a compile time error "out of heap".
	public: static constexpr int EXTENDED_OVERLAPPED_ARRAY_SIZE = SOMAXCONN / sizeof(ExpandedOverlapped) / 200;
	private: SOCKET m_listenSocket = INVALID_SOCKET;
	private: HANDLE m_IOCP = INVALID_HANDLE_VALUE;

	private: ServerCreateParameter m_createParameter;

	private: volatile bool m_keepLooping = true;
	private: int m_threadPoolSize = 0;
	private: std::vector<std::thread> m_threadPool;

	private: std::queue<ExpandedOverlapped*> m_sendQueue;
	private: std::mutex m_mainLock;

	private: static std::atomic<uint32_t> m_fermionCounter;

	// for AcceptEx's recv buffer which is not using.
	// Warning : If this is not using, but this must exists till the end.
	private: char m_listenBuffer[BUFFER_SIZE];

	protected: template<typename T> T* AssignFromBufferForWrite(GBBuffer* pGbBuffer)
	{
		assert(pGbBuffer->m_writeOffset + sizeof(T) < BUFFER_SIZE);

		size_t bytesToAssign = sizeof(T);

		T* pAddrToReturn = (T*)&pGbBuffer->m_buffer[pGbBuffer->m_writeOffset];

		pGbBuffer->m_writeOffset += bytesToAssign;

		return pAddrToReturn;
	}

	protected: template<typename T> T* AssignFromBufferForRead(GBBuffer* pGbBuffer)
	{
		assert(pGbBuffer->m_readOffset + sizeof(T) < BUFFER_SIZE);

		size_t bytesToAssign = sizeof(T);

		T* pAddrToReturn = (T*)&pGbBuffer->m_buffer[pGbBuffer->m_readOffset];

		pGbBuffer->m_readOffset += bytesToAssign;

		return pAddrToReturn;
	}

	protected: template<typename T> uint32_t ReadByteByByte(char* buffer, T& value)
	{
		int shift = 0;
		uint32_t readByteLength = 0;

		memset(&value, 0, sizeof(T));

		// the read MSB means the sign of keeping going.
		unsigned char MSB = 0;
		do
		{
			readByteLength++;
			unsigned char byteForBuffer = *buffer;
			value = value | ((static_cast<T>(byteForBuffer & 0b0111'1111)) << shift);
			shift += 7;
			MSB = byteForBuffer & 0b1000'0000;
			buffer++;
		} while (0 != MSB);

		return readByteLength;
	}

	protected: uint32_t ReadUUID(char* buffer, GBUUID& value)
	{
		const uint32_t UUID_SIZE = 16;

		std::array<char, UUID_SIZE> tmpBuffer;
		memcpy_s(tmpBuffer.data(), UUID_SIZE, buffer, UUID_SIZE);

		value.FromRaw(tmpBuffer);

		return UUID_SIZE;
	}

	protected: template<typename STRING> uint32_t ReadString(char* buffer, STRING& outString)
	{
		uint32_t readByteLength = 0;

		char stringLength = 0;
		uint32_t rr = ReadByteByByte(buffer, stringLength);
		readByteLength += rr;
		buffer += rr;

		outString.reserve(stringLength);
		outString.assign(buffer, stringLength);

		readByteLength += stringLength;
		buffer += stringLength;

		return readByteLength;
	}

	protected: template<typename T> uint32_t Read(char* buffer, T& outValue)
	{
		outValue = *(T*)buffer;

		return sizeof(T);
	}

	protected: template<typename T> void WriteByteByByte(GBBuffer* pGbBuffer, T value)
	{
		char* buffer = &pGbBuffer->m_buffer[pGbBuffer->m_writeOffset];

		do
		{
			unsigned char MSB = 0;

			if (0b0111'1111 < value)
			{
				MSB = 0b1000'0000;
			}

			*buffer = (char)(value & 0b0111'1111);
			*buffer = (char)(value | MSB);

			buffer++;
			pGbBuffer->m_writeOffset++;

			value = value >> 7;
		} while (0 < value);
	}

	protected: template<typename STRING> void WriteString(GBBuffer* pGbBuffer, const STRING& inString)
	{
		uint32_t writeByteLength = 0;
		char* buffer = pGbBuffer->m_buffer;

		// String Length
		char inStringSize = (char)inString.length();
		WriteByteByByte(pGbBuffer, inStringSize);

		errno_t cpyStrResult = strncpy_s(buffer, 1024, inString.c_str(), inStringSize);
		pGbBuffer->m_writeOffset += writeByteLength;
		buffer += inStringSize;
	}

	protected: template<typename T> void Write(GBBuffer* pGbBuffer, const T& outValue)
	{
		*(T*)buffer = outValue;

		int writeLength = sizeof(T);
		pGbBuffer->m_writeBuffer.m_buffer += writeLength;
		pGbBuffer->m_writeBuffer.m_writeOffset += writeLength;
	}

	protected: const uint64_t BIT_FLAG_FOR_VECTOR_XZ = 0b0000'0000'0000'0000'0000'0000'0000'0000'0000'0011'1111'1111'1111'1111'1111'1111;
	protected: const uint64_t BIT_FLAG_FOR_VECTOR_Y = 0b0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'1111'1111'1111;

	protected: void Write2BytesAsBigEndian(GBBuffer* pGbBuffer, uint16_t value);
	protected: void Write4BytesAsBigEndian(GBBuffer* pGbBuffer, uint32_t value);
	protected: void Write8BytesAsBigEndian(GBBuffer* pGbBuffer, uint64_t value);
	protected: void WriteIntGBVector3(GBBuffer* pGbBuffer, const GBVector3<int>& value);

	protected: uint16_t Read2BytesAsBigEndian(GBBuffer* pGbBuffer);
	protected: uint32_t Read4BytesAsBigEndian(GBBuffer* pGbBuffer);
	protected: uint64_t Read8BytesAsBigEndian(GBBuffer* pGbBuffer);
	protected: uint32_t ReadIntGBVector3(GBBuffer* pGbBuffer, GBVector3<int>& value);

	protected: template<typename FUNCTION> void MakeAndSendPacket(SOCKET* pSocket, GBBuffer* pGbBuffer, const FUNCTION& func)
	{
		pGbBuffer->Reset();

		char* pPacketLength = AssignFromBufferForWrite<char>(pGbBuffer);

		func(pGbBuffer);

		*pPacketLength = (char)(pGbBuffer->m_writeOffset - 1);

		int sendResult = send(*pSocket, pGbBuffer->m_buffer, pGbBuffer->m_writeOffset, NULL);

		if (SOCKET_ERROR == sendResult)
		{
			std::cout << "[send failed] WSAGetLastError : " << WSAGetLastError() << std::endl;
			return;
		}
	}

	public: virtual ~Core();
	public: void ThreadFunction();
	public: int IssueRecv(ExpandedOverlapped* pEol, ULONG lengthToReceive);
	public: int IssueSend(ExpandedOverlapped* pEol);

	public: void EnqueueAndIssueSend(ExpandedOverlapped* pEol);

	// Consuming a gathered message.
	public: virtual void ConsumeGatheredMessage(ExpandedOverlapped* pEol, char* mescsage, const uint32_t messageSize, int& readOffSet) = 0;

	protected: virtual void* GetSessionInformationArray() = 0;

	public: std::pair<int, int> Start(const ServerCreateParameter& param);
	};
}