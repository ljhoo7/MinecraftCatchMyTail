#pragma once

#include <cstdint>

namespace GenericBoson
{
	namespace ServerEngine
	{
		// This is a box in Minecraft.
		class Fermion
		{
		public: Fermion();
		public: virtual ~Fermion() = default;

		// Fermion ID
		public: uint32_t m_ID = 0;
		};
	}
}