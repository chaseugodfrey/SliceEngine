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
#include <compare>
#include <rttr/registration.h>

namespace SliceEngine::Utils
{
	constexpr uint64_t FNV_Prime = 1099511628211ULL;
	constexpr uint64_t FNV_Offset = 14695981039346656037ULL;

	inline uint64_t HashString(const std::string& str)
	{
		uint64_t hash = FNV_Offset;
		for (char c : str) {
			hash ^= static_cast<uint64_t>(c);
			hash *= FNV_Prime;
		}
		return hash;
	}
}


namespace SliceEngine
{
	struct GUID
	{
	public:
		GUID() : mValue(0) {}
		explicit GUID(uint64_t value) : mValue(value) {}
		
		// this shit is cool wtf 4 for the price of 1
		auto operator<=>(const GUID& other) const = default;

		static GUID Generate()
		{
			static std::mt19937_64 rng = []() {
				std::random_device rd;
				auto now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
				std::seed_seq seq{ static_cast<uint64_t>(rd()), static_cast<uint64_t>(now)};
				return std::mt19937_64(seq);
			}();

			static std::uniform_int_distribution<uint64_t> dist;
			return GUID(dist(rng) | (1ULL << 63)); // Ensure first bit is 1
		}

		static GUID Generate(std::string const& name, uint64_t typeID)
		{
			std::string combined = std::to_string(typeID) + ":" + name;
			return GUID(Utils::HashString(combined) | (1ULL << 63));
		}

		static GUID FromString(std::string str)
		{
			return GUID(static_cast<uint64_t>(std::stoull(str)));
		}

		static GUID null()
		{
			return GUID(0);
		}

		bool IsValid() const { return mValue != 0; }

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