#pragma once

#include <cstdint>

namespace GenericBoson
{
	class World
	{
	public: enum class Weather : int
	{
		Sunny = 0,
		Rainy = 1,
		Stormy = 2,
	};

	public: enum class GameMode : int32_t
	{
		Invalid = -1,
		Survival = 0,
		Creative = 1,
		Adventure = 2,
		Spectator = 3,
	};

	public: int64_t m_ageMs;
	public: int64_t m_timeOfDay;
	public: bool m_dayLightEnabled = true;

			// #ToDo
	public: short m_pingMs = 2;

	public: GameMode m_gameMode = GameMode::Survival;
	};
}