#pragma once

#include "Fermion.h"

namespace GenericBoson
{
	namespace ServerEngine
	{
		class Character : public Fermion
		{
		public: Character();
		public: virtual ~Character() = default;
		};
	}
}