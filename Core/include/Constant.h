#pragma once

#include <tchar.h>
#include <cstdint>

namespace GenericBoson
{
	namespace ServerEngine
	{
		struct InternalConstant
		{
			static const TCHAR* SUCCESS;
			
			static const uint32_t CompressThreshold = 256;
		};
	}
}