/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			GUID.h
 author:		Gideon Francis
 email:			g.francis@digipen.edu
 brief:			Handles GUID

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#ifndef GUID_H
#define GUID_H

#include <cstdint>
#include <functional>
#include <random>
#include <rttr/registration.h>

namespace SliceEngine
{
	struct GUID
	{
	public:
		GUID() : mValue(0) {}
		explicit GUID(uint64_t value) : mValue(value) {}
		//bool operator==(const GUID& other) const { return mValue == other.mValue; }
		//bool operator!=(const GUID& other) const { return mValue != other.mValue; }
		//bool operator<(const GUID& other) const { return mValue < other.mValue; }
		//bool operator<=(const GUID& other) const { return mValue <= other.mValue; }
		//bool operator>(const GUID& other) const { return mValue > other.mValue; }
		//bool operator>=(const GUID& other) const { return mValue >= other.mValue; }
		
		// this shit is cool wtf 4 for the price of 1
		//bool operator<=>(const GUID& other) const = default;

		friend inline bool operator==(const GUID& lhs, const GUID& rhs)
		{
			return lhs.GetGUID() == rhs.GetGUID();
		}

		friend inline bool operator!=(const GUID& lhs, const GUID& rhs)
		{
			return !(lhs == rhs);
		}

		friend inline bool operator<(const GUID& lhs, const GUID& rhs)
		{
			return lhs.GetGUID() < rhs.GetGUID();
		}

		friend inline bool operator>(const GUID& lhs, const GUID& rhs)
		{
			return !(lhs < rhs);
		}

		static GUID Generate()
		{
			static std::mt19937_64 rng(std::random_device{}());
			static std::uniform_int_distribution<uint64_t> dist;
			return GUID(dist(rng) | (1ULL << 63)); // Ensure first bit is 1
		}

		static GUID Generate(std::string const& name, uint64_t typeID)
		{
			std::string combined = std::to_string(typeID) + ":" + name;
			std::hash<std::string> hasher;
			uint64_t nameHash = hasher(combined);
			return GUID(nameHash | (1ULL << 63));
		}

		static GUID FromString(std::string str)
		{
			return GUID(static_cast<uint64_t>(std::stoull(str)));
		}

		static GUID null()
		{
			return GUID(0);
		}

		std::string toString() const
		{
			return std::to_string(this->mValue);
		}

		uint64_t GetGUID() const { return mValue; }

	private:
		uint64_t mValue;
	};


}


template<>
struct std::hash<SliceEngine::GUID>
{
	std::size_t operator()(const SliceEngine::GUID& guid) const noexcept
	{
		return std::hash<uint64_t>{}(guid.GetGUID());
	}
};

#endif // !GUID_H