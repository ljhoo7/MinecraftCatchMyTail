#pragma once

#include <string>

namespace GenericBoson
{
	namespace ServerEngine
	{
		typedef std::wstring	GBStringW;
		typedef std::string		GBStringA;
#if defined(UNICODE)
		typedef GBStringW	GBString;
#define GBFUNCTION __FUNCTIONW__
#define _GBT(PARAM) L##PARAM
#else //defined(UNICODE)
		typedef GBStringA	GBString;
#define GBFUNCTION __FUNCTION__
#define _GBT(PARAM) PARAM
#endif //defined(UNICODE)
	}
}