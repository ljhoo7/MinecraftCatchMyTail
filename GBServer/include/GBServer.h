#pragma once

#include "../../Core/include/GBCore.h"
#include "Character.h"

namespace GenericBoson
{
	class Server : public Core
	{
	public: void SendStartCompress(ExpandedOverlapped& eol);
	public: void SendLoginSuccess(ExpandedOverlapped& eol);
	public: void SendJoinGame(ExpandedOverlapped& eol);
	public: void SendSpawnSpot(ExpandedOverlapped& eol);
	public: void SendDifficulty(ExpandedOverlapped& eol);
	public: void SendCharacterAbility(ExpandedOverlapped& eol);
	public: void SendTime(ExpandedOverlapped& eol);
	public: void SendInventory(ExpandedOverlapped& eol);
	public: void SendHealth(ExpandedOverlapped& eol);
	public: void SendExperience(ExpandedOverlapped& eol);
	public: void SendEquippedItem(ExpandedOverlapped& eol);
	public: void SendPlayerList(ExpandedOverlapped& eol);

	private: void ConsumeGatheredMessage(ExpandedOverlapped& eol, char* message, const uint32_t messageSize, uint32_t& readOffSet) override;
	};
}