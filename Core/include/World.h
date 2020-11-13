#pragma once

#include <cstdint>

namespace GenericBoson
{
	namespace ServerEngine
	{
		class World
		{
		public: enum class Weather : int
		{
			Sunny = 0,
			Rainy = 1,
			Stormy = 2,
		};

		public: int64_t m_ageMs;
		public: int64_t m_timeOfDay;
		public: bool m_dayLightEnabled = true;
		};
	}
}