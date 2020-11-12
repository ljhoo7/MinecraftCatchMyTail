#include "Character.h"

namespace GenericBoson
{
	namespace ServerEngine
	{
		Character::Character()
			: Fermion()
		{

		}

		const char Character::GOD1_BITMASK = 0b0000'0001;
		const char Character::FLYING_BITMASK = 0b0000'0010;
		const char Character::GOD2_BITMASK = 0b1000'0000;
		const char Character::CANFLY_BITMASK = 0b0000'0100;
	}
}