#ifndef CSVSERIALIZER_H
#define CSVSERIALIZER_H

#include <iostream>
#include <unordered_map>
#include <fstream>

namespace SliceEngine
{
	namespace CSVSerializer
	{
		struct csv
		{
			int num_rows{};
			int num_cols{};

			//for preserving original order for serialization after it was scrambled in the map for fast accessing speed
			std::vector<std::string> row_keys;
			std::vector<std::string> col_keys;

			std::unordered_map<std::string, std::unordered_map<std::string, std::string>> data;
		};

		void Serialize(csv const& input, std::filesystem::path const& filePath);

		csv Deserialize(std::filesystem::path const& filePath);

		void Append(std::filesystem::path const& filePath, std::string const& string_to_append);

		//Debug
		void Print(csv const& input);

		//Uses Assets/Test.csv in deserializing it and 
		void Test();
	}
}

#endif