#include "Character.h"

namespace GenericBoson
{
	Character::Character()
		: Fermion()
	{

	}

	float Character::GetXpPercentage()
	{
		// #ToDo
		return m_experience;
	}

	int Character::GetLevel()
	{
		// #ToDo
		return 1;
	}

	const char Character::GOD1_BITMASK = 0b0000'0001;
	const char Character::FLYING_BITMASK = 0b0000'0010;
	const char Character::GOD2_BITMASK = 0b1000'0000;
	const char Character::CANFLY_BITMASK = 0b0000'0100;

	const float Character::MAX_HEALTH = 20.0f;
	const int Character::MAX_FOOD_LEVEL = 20;
}