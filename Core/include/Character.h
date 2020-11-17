#pragma once

#include "Fermion.h"
#include "Inventory.h"

namespace GenericBoson
{
	namespace ServerEngine
	{
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

		public: static const float MAX_HEALTH;
		public: static const int MAX_FOOD_LEVEL;

		public: float m_health = MAX_HEALTH;
		public: int m_foodLevel = MAX_FOOD_LEVEL;
		public: float m_foodSaturationLevel = 5.0f;

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