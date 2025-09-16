#include <pch.h>
#include "CSVSerializer.h"

namespace SliceEngine
{
	namespace CSVSerializer
	{

        void Serialize(csv const& input, std::filesystem::path const& filePath)
        {
            std::ofstream ofs(filePath);
            if (!ofs.is_open())
            {
                Logger::LogError("CSVSerializer", "Unable to write CSV to path: " + filePath.string());
                return;
            }

            //Col Headers
            for (size_t c = 0; c < input.col_keys.size(); ++c)
            {
                ofs << input.col_keys[c];
                if (c + 1 < input.col_keys.size())
                {
                    ofs << ",";
                }
            }
            ofs << "\n";

            //Rows
            for (size_t r = 0; r < input.row_keys.size(); ++r)
            {
                const std::string& row_key = input.row_keys[r];
                ofs << row_key;

                for (size_t c = 0; c < input.col_keys.size(); ++c)
                {
                    const std::string& col_key = input.col_keys[c];

                    auto row_it = input.data.find(row_key);
                    if (row_it != input.data.end())
                    {
                        auto col_it = row_it->second.find(col_key);
                        if (col_it != row_it->second.end())
                        {
                            ofs << "," << col_it->second;
                        }
                    }
                }
                ofs << "\n";
            }

            ofs.close();
        }

		csv Deserialize(std::filesystem::path const& filePath)
		{
			std::ifstream ifs(filePath);

			csv result;

			if (!ifs.is_open())
			{
				Logger::LogError("CSVSerializer", "CSV File in " + filePath.string() + " cannot be opened/found.");
				return result;
			}

            std::string cur_line;

            //Column headers are the keys for the x axis
            if (std::getline(ifs, cur_line))
            {
                std::istringstream iss(cur_line);
                std::string word;

                bool first = true;
                while (std::getline(iss, word, ','))
                {
                    result.col_keys.push_back(word);
                }

                result.num_cols = static_cast<int>(result.col_keys.size());
            }

            //Remaining rows
            while (std::getline(ifs, cur_line))
            {
                std::istringstream iss(cur_line);
                std::string word;

                //First word in row = row key
                if (!std::getline(iss, word, ','))
                    continue;

                std::string row_key = word;
                result.row_keys.push_back(row_key);

                int col_index = 0;

                //Remaining words = values
                while (std::getline(iss, word, ','))
                {
                    if (col_index < result.num_cols)
                    {
                        const std::string& col_key = result.col_keys[col_index];
                        result.data[row_key][col_key] = word;
                    }
                    col_index++;
                }

                result.num_rows++;
            }

            ifs.close();
            return result;
		}

		void Append(std::filesystem::path const& filePath, std::string const& string_to_append)
		{
			std::ofstream file(filePath, std::ios::app);

			if (file.is_open())
			{
				file << string_to_append << "\n";
				file.close();
			}
			else
			{
				Logger::LogError("CSVSerializer", "CSV File in " + filePath.string() + " cannot be opened/found.");
			}
		}

        void Print(csv const& input)
        {
            for (const auto& [row_key, row_map] : input.data)
            {
                Logger::Log("CSVSerializer::Print", row_key + ":");

                for (const auto& [col_key, value] : row_map)
                {
                    Logger::Log("CSVSerializer::Print", " " + col_key + " = " + value);
                }
            }
        }

		void Test()
		{			
            csv test = Deserialize("Assets/CSVTest1.csv");
            csv empty;
            Serialize(test, "Assets/CSVTest1.csv");
		}
	}
}