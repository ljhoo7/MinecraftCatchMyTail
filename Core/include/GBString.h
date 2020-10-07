#pragma once

#include <string>

namespace GenericBoson
{
	namespace ServerEngine
	{
#if defined(UNICODE)
		typedef std::wstring	GBString;
#define GBFUNCTION __FUNCTIONW__
#else //defined(UNICODE)
		typedef std::string GBString;
#define GBFUNCTION __FUNCTION__
#endif //defined(UNICODE)
	}
}