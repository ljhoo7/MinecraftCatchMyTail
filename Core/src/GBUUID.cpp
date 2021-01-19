#include "GBUUID.h"

namespace GenericBoson
{
	std::array<char, 16> GBUUID::ToRaw() const
	{
		std::array<char, 16> returnValue(m_UUID);

		if (UUIDType::MSLegacyGUID != GetUUIDType())
		{
			return returnValue;
		}

		// #ToDo
		throw std::exception();
	}

	void GBUUID::FromRaw(const std::array<char, 16>& rawValue)
	{
		m_UUID = rawValue;

		if (UUIDType::MSLegacyGUID != GetUUIDType())
		{
			return;
		}

		// #ToDo
		throw std::exception();
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