#ifndef ASSET_DESCRIPTOR
#define ASSET_DESCRIPTOR

#include <filesystem>
#include "xresource_guid.h"
namespace Asset {
	//easy enum serializing
	template<typename Enum>
	class EnumReader {
		Enum& e_;
		friend std::istream& operator>>(std::istream& in, const EnumReader& val) {
			typename std::underlying_type<Enum>::type asInt;
			if (in >> asInt) val.e_ = static_cast<Enum>(asInt);
			return in;
		}
	public:
		EnumReader(Enum& v) : e_(v) {};
		/*std::ifstream& operator>>(std::ifstream& ifs, EnumReader const& er) {
			static_assert(std::is_integral<Enum>);
			int tmp_int{};
			ifs >> tmp_int;
			v.e = static_cast<Enum>(tmp_int);
		}*/
	};

	template<typename Enum>
	EnumReader<Enum> readEnum(Enum& e) {
		return EnumReader<Enum>(e);
	}
	struct Descriptor_Base {
		std::filesystem::path file_path;
		xresource::full_guid guid;
		virtual void Serialize(std::filesystem::path const&) = 0;
		virtual void Deserialize(std::filesystem::path const&) = 0;
	};
}


#endif