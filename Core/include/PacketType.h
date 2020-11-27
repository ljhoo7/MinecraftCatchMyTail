#pragma once

#include <cstdint>

namespace GenericBoson
{
	enum class PacketType : uint32_t
	{
		LoginStart = 0,
		LoginSuccess = 2,
		StartCompression = 3,
		ClientSettings = 4,
		JoinGame = 5,
		Difficulty = 9,
		Experience = 26,
		EquippedItemChange = 29,
		CharacterAbility = 38,
		PlayerList = 39,
		SpawnSpot = 55,
		Time = 61,
		Health = 66,
		Inventory = 71,
	};
}