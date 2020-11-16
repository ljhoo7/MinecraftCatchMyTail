#include "Inventory.h"

namespace GenericBoson
{
	namespace ServerEngine
	{
		std::atomic<unsigned char> Inventory::g_IDFactory = 0;

		int Inventory::GetTotalSlotCount()
		{
			int totalCount = 0;
			for (auto& pSlot : m_slotVector)
			{
				totalCount += pSlot->GetSlotCount();
			}

			return totalCount;
		}
	}
}