#pragma once

#include <cstdint>

namespace GenericBoson
{
	namespace ServerEngine
	{
		enum class PacketType : uint32_t
		{
			LoginStart = 0,
			LoginSuccess = 2,
			StartCompression = 3,
			ClientSettings = 4,
			JoinGame = 5,
			Difficulty = 9,
			CharacterAbility = 38,
			SpawnSpot = 55,
		};
	}
}