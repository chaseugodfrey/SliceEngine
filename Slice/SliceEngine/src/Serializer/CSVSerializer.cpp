#include <pch.h>
#include "CSVSerializer.h"

namespace SliceEngine
{
	namespace CSVSerializer
	{
        constexpr auto testPath("Assets/Tests/");

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

        namespace Tests
        {
            void Test1(bool cleanOutput)
            {
                SLICE_LOG("Test 1 Beginning...");
                // Test data
                csv input;
                input.col_keys = { "Unit Type", "Health", "Speed", "Cost", "Weapon" };
                input.row_keys = { "Commander", "Spearman", "Ranger", "Knight", "Wyrmling", "Swarmling", "Viper" };
                input.num_cols = static_cast<int>(input.col_keys.size());
                input.num_rows = static_cast<int>(input.row_keys.size());

                // Fill data
                input.data["Commander"]["Unit Type"] = "Commander";
                input.data["Commander"]["Health"] = "15";
                input.data["Commander"]["Speed"] = "0";
                input.data["Commander"]["Cost"] = "0";
                input.data["Commander"]["Weapon"] = "Commander_Sword";

                input.data["Spearman"]["Unit Type"] = "Spearman";
                input.data["Spearman"]["Health"] = "20";
                input.data["Spearman"]["Speed"] = "65";
                input.data["Spearman"]["Cost"] = "25";
                input.data["Spearman"]["Weapon"] = "Spearman_Spear";

                input.data["Ranger"]["Unit Type"] = "Ranger";
                input.data["Ranger"]["Health"] = "25";
                input.data["Ranger"]["Speed"] = "65";
                input.data["Ranger"]["Cost"] = "30";
                input.data["Ranger"]["Weapon"] = "Ranger_Crossbow";

                input.data["Knight"]["Unit Type"] = "Knight";
                input.data["Knight"]["Health"] = "35";
                input.data["Knight"]["Speed"] = "180";
                input.data["Knight"]["Cost"] = "50";
                input.data["Knight"]["Weapon"] = "Knight_Halberd";

                input.data["Wyrmling"]["Unit Type"] = "Wyrmling";
                input.data["Wyrmling"]["Health"] = "20";
                input.data["Wyrmling"]["Speed"] = "75";
                input.data["Wyrmling"]["Cost"] = "0";
                input.data["Wyrmling"]["Weapon"] = "Wyrmling_Claw";

                input.data["Swarmling"]["Unit Type"] = "Swarmling";
                input.data["Swarmling"]["Health"] = "5";
                input.data["Swarmling"]["Speed"] = "100";
                input.data["Swarmling"]["Cost"] = "0";
                input.data["Swarmling"]["Weapon"] = "Swarmling_Claw";

                input.data["Viper"]["Unit Type"] = "Viper";
                input.data["Viper"]["Health"] = "25";
                input.data["Viper"]["Speed"] = "0";
                input.data["Viper"]["Cost"] = "0";
                input.data["Viper"]["Weapon"] = "Viper_Spine_Thrower";

                Serialize(input, testPath + std::string("CSVTest1.csv"));
                csv output = Deserialize(testPath + std::string("CSVTest1.csv"));
                if (cleanOutput)
                {
                    std::filesystem::remove(testPath + std::string("CSVTest1.csv"));
                }
                SLICE_LOG("Test 1 Ended...");
            }

            void RunTests(bool cleanOutput)
            {
                Test1(cleanOutput);
                SLICE_LOG("Tests Completed, Examine Console Log for Errors");
            }
        }
	}
}