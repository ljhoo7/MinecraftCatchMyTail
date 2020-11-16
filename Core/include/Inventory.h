#pragma once

#include <atomic>
#include <cstdint>
#include <vector>

#include "SlotBase.h"

namespace GenericBoson
{
	namespace ServerEngine
	{
		class Inventory
		{
		private: static std::atomic<unsigned char> g_IDFactory;
		public: Inventory()
		{
			m_ID = g_IDFactory.fetch_add(1) % 127;
			m_ID += 1;
		}
		public: unsigned char m_ID;
		public: int16_t m_numberOfSlots = 1;

		public: std::vector<SlotBase*> m_slotVector;

		public: int GetTotalSlotCount();
		};
	}
}