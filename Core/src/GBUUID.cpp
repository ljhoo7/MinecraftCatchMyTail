#include "GBUUID.h"

namespace GenericBoson
{
	std::array<uint8_t, 16> GBUUID::ToRaw() const
	{
		std::array<uint8_t, 16> returnValue(m_UUID);

		if (UUIDType::MSLegacyGUID != GetUUIDType())
		{
			return returnValue;
		}

		// #ToDo
		throw std::exception();
	}

	void GBUUID::FromRaw(const std::array<uint8_t, 16>& rawValue)
	{
		m_UUID = rawValue;

		if (UUIDType::MSLegacyGUID != GetUUIDType())
		{
			return;
		}

		// #ToDo
		throw std::exception();
	}

	bool GBUUID::FromString(const GBString& str)
	{
		GBString copiedString;
		size_t strLength = str.length();

		copiedString.reserve(strLength);

		if (36 == strLength)
		{
			int barIndexArray[] { 8, 13, 18, 23 };

			for (auto barIndex : barIndexArray)
			{
				if ('-' != str[barIndex])
				{
					return false;
				}
			}

			for (auto iChar : str)
			{
				if ('-' == iChar)
				{
					continue;
				}

				copiedString.push_back(iChar);
			}
		}
		else if (32 == strLength)
		{
			copiedString.assign(str);
		}
		else
		{
			return false;
		}

		for(int k = 0; k < m_UUID.size(); ++k)
		{
			uint8_t highNibble = m_UUID[k];
			uint8_t lowNibble = m_UUID[k];
			if ((highNibble > 0x1111) || (lowNibble > 0x1111))
			{
				// Invalid
				return false;
			}

			m_UUID[k] = (uint8_t)((highNibble << 4) | lowNibble);
		}
	}

	GBString GBUUID::ToString()
	{
		GBString stringToReturn;

		// For guaranteed return value copy elision, You must use above C++17
		// static_assert(201700L < __cplusplus); // not working
		return stringToReturn;
	}

	UUIDType GBUUID::GetUUIDType() const
	{
		// The top 3 bits of 8th byte
		const char topThreeBits = static_cast<char>((m_UUID[8] >> 5) & 0b111);

		if (0 == (topThreeBits & 0b100))
		{
			return UUIDType::Obsolete;
		}
		else if (0 == (topThreeBits & 0b010))
		{
			return UUIDType::Standard;
		}
		else if (0 == (topThreeBits & 0b001))
		{
			return UUIDType::MSLegacyGUID;
		}
		else
		{
			return UUIDType::Reserved;
		}
	}
}