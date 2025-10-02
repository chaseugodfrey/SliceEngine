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
		bool operator==(const GUID& other) const { return mValue == other.mValue; }
		bool operator!=(const GUID& other) const { return mValue != other.mValue; }
		//bool operator<(const GUID& other) const { return mValue < other.mValue; }
		//bool operator<=(const GUID& other) const { return mValue <= other.mValue; }
		//bool operator>(const GUID& other) const { return mValue > other.mValue; }
		//bool operator>=(const GUID& other) const { return mValue >= other.mValue; }
		
		// this shit is cool wtf 4 for the price of 1
		bool operator<=>(const GUID& other) const { return mValue >= other.mValue; }

		static GUID Generate()
		{
			static std::mt19937_64 rng(std::random_device{}());
			static std::uniform_int_distribution<uint64_t> dist;
			return GUID(dist(rng) | (1ULL << 63)); // Ensure first bit is 1
		}

		static GUID FromString(std::string str)
		{
			return GUID(static_cast<uint64_t>(std::stoull(str)));
		}

		uint64_t GetGUID() const { return mValue; }

	private:
		uint64_t mValue;
	};

	//RTTR_REGISTRATION
	//{
	//	rttr::registration::class_<GUID>("GUID")
	//		.constructor<>()
	//		.constructor<uint64_t>()
	//		.property_readonly("Value", &GUID::GetGUID);
	//}
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