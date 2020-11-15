#pragma once

#include <atomic>

#include "Fermion.h"

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
		};

		class Character : public Fermion
		{
		public: static const char GOD1_BITMASK;
		public: static const char FLYING_BITMASK;
		public: static const char GOD2_BITMASK;
		public: static const char CANFLY_BITMASK;

		public: char m_abilityState = 0;
		public: float m_flyingMaxSpeed = 1.0f;
		public: float m_sprintingMaxSpeed = 1.3f;

		public: Inventory m_inventory;

		public: bool IsGod()
		{
			char result = m_abilityState & GOD1_BITMASK;
			result |= (m_abilityState & GOD2_BITMASK);

			return result;
		}

		public: bool IsFlying()
		{
			return m_abilityState & FLYING_BITMASK;
		}

		public: bool CanFly()
		{
			return m_abilityState & CANFLY_BITMASK;
		}

		public: Character();
		public: virtual ~Character() = default;
		};
	}
}