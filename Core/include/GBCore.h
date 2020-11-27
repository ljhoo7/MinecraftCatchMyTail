#pragma once

#include "GBString.h"
#include "Constant.h"
#include "GBBuffer.h"
#include "PacketType.h"
#include "Character.h"
#include "GBVector.h"
#include "World.h"

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

			std::string m_uuid;
			std::string m_userName;

			SessionState m_sessionState = SessionState::start;

			Character m_controllableCharacter;

			short m_protocolVersion = 0;
		};

	public: World m_world;

		// If you remove '/100', you will get a compile time error "out of heap".
	private: static constexpr int EXTENDED_OVERLAPPED_ARRAY_SIZE = SOMAXCONN / sizeof(ExpandedOverlapped) / 200;
	private: SOCKET m_listenSocket = INVALID_SOCKET;
	private: ExpandedOverlapped m_extendedOverlappedArray[EXTENDED_OVERLAPPED_ARRAY_SIZE];
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
	private: char m_listenBuffer[1024];

	protected: template<typename T> uint32_t ReadByteByByte(char* buffer, T& value);
	protected: template<typename STRING> uint32_t ReadString(char* buffer, STRING& outString);
	protected: template<typename T> uint32_t Read(char* buffer, T& outValue);
	protected: template<typename T> void WriteByteByByte(ExpandedOverlapped& eol, T value);
	protected: template<typename STRING> void WriteString(ExpandedOverlapped& eol, const STRING& inString);
	protected: template<typename T> void Write(ExpandedOverlapped& eol, const T& outValue);
	protected: void Write2BytesAsBigEndian(ExpandedOverlapped& eol, uint16_t value);
	protected: void Write4BytesAsBigEndian(ExpandedOverlapped& eol, uint32_t value);
	protected: void Write8BytesAsBigEndian(ExpandedOverlapped& eol, uint64_t value);
	protected: void WriteIntGBVector3(ExpandedOverlapped& eol, const GBVector3<int>& value);

	public: virtual ~Core();
	public: void ThreadFunction();
	public: int IssueRecv(ExpandedOverlapped* pEol, ULONG lengthToReceive);
	public: int IssueSend(ExpandedOverlapped* pEol);

	public: void EnqueueAndIssueSend(ExpandedOverlapped& eol);

		// Consuming a gathering completed message.
	public: virtual void ConsumeGatheredMessage(ExpandedOverlapped& eol, char* mescsage, const uint32_t messageSize, uint32_t& readOffSet) = 0;

	public: std::pair<int, int> Start(const ServerCreateParameter& param);
	};
}