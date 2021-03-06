#pragma once

#include <array>

#include "GBString.h"

namespace GenericBoson
{
	enum class UUIDType : uint8_t
	{
		Obsolete = 0,
		Standard = 1,
		MSLegacyGUID = 2,
		Reserved = 3,
	};

	class GBUUID
	{
	private: std::array<uint8_t, 16> m_UUID;
	private: UUIDType GetUUIDType() const;

	public: std::array<uint8_t, 16> ToRaw() const;
	public: void FromRaw(const std::array<uint8_t, 16>&);

	public: uint8_t CharacterToByte(char hexCharacter);
	public: bool FromString(const GBString& str);
	public: GBString ToString();
	};
}