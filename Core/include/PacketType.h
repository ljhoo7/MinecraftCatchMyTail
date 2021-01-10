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
		Difficulty = 13,
		Experience = 26,
		EquippedItemChange = 29,
		JoinGame = 35,
		PlayerList = 39,
		CharacterAbility = 44,
		Time = 61,
		Health = 66,
		SpawnSpot = 71,
		//Inventory = 71,
	};
}