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
		Stastistics = 7,
		Difficulty = 13,
		Inventory = 20,
		EquippedItemChange = 29,
		JoinGame = 35,
		CharacterAbility = 44,
		PlayerList = 46,
		Stastics = 57,
		HeldItemChange = 58,
		Experience = 64,
		Health = 65,
		SpawnSpot = 70,
		Time = 71,
	};
}