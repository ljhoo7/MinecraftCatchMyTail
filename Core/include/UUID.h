#pragma once

#include <array>

namespace GenericBoson
{
	enum class UUIDType : char
	{
		Obsolete = 0,
		Standard = 1,
		MSLegacyGUID = 2,
		Reserved = 3,
	};

	class GBUUID
	{
	private: std::array<char, 16> m_UUID;
	private: UUIDType GetUUIDType() const;

	public: std::array<char, 16> ToRaw() const;
	public: void FromRaw(const std::array<char, 16>&);
	};
}