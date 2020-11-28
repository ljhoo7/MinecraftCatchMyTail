#pragma once

namespace GenericBoson
{
	class SlotBase
	{
	public: virtual int GetSlotCount() = 0;
	public: virtual int WriteItem() = 0;
	};
}