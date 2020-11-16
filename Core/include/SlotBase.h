#pragma once

namespace GenericBoson
{
	namespace ServerEngine
	{
		class SlotBase
		{
		public: virtual int GetSlotCount() = 0;
		public: virtual int WriteItem() = 0;
		};
	}
}