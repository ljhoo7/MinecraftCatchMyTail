#pragma once

#include <string>

namespace GenericBoson
{
	namespace ServerEngine
	{
#if defined(UNICODE)
		typedef std::wstring	GBString;
#define GBFUNCTION __FUNCTIONW__
#define _GBT(PARAM) L##PARAM
#else //defined(UNICODE)
		typedef std::string GBString;
#define GBFUNCTION __FUNCTION__
#define _GBT(PARAM) PARAM
#endif //defined(UNICODE)
	}
}