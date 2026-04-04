using System;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using SliceScript.Source.Core.Systems;
using System.Runtime.CompilerServices;

/**
 * This is intended for storing preferences like
 * Gamma, volume, etc
 * Also stores defaults if need to restore default
 * Gamma
 * Master Vol
 * SFX Vol
 * BGM Vol
 */
namespace SliceEngine
{
    public class PreferenceSettings
    {
        private static string filePath;

        public static void Initialize()
        {
            Dictionary<string, string> preferences = new Dictionary<string, string>();
            Console.WriteLine("AHAHAHAHHAHA");
            // Load preference or create if doesn't exist
            filePath = "ProjectSettings/GamePreference.txt";
            SliceLog.Log("Loading preferences from Filepath: " + filePath);
            Console.WriteLine("BABBABABBABABA");

            if (File.Exists(filePath))
            {
                string[] lines = File.ReadAllLines(filePath);
                foreach (string line in lines)
                {
                    string[] keyValue = line.Split('=');
                    if (keyValue.Length == 2)
                    {
                        string key = keyValue[0].Trim();
                        string value = keyValue[1].Trim();

                        preferences[key] = value;
                        // Store the preference in a dictionary or apply it directly
                        SliceLog.Log($"Loaded preference: {key} = {value}");
                    }
                }
            }
            else
            {
                // We only got 4 settings to save
                // We just hardcode the default by reading from the engine values
                SliceLog.Log("Could not find the GamePreferences file from: " + filePath);
                preferences["Gamma"] = Camera.Gamma.ToString();
                preferences["Master Volume"] = AudioManager.GetMasterVolume().ToString();
                preferences["BGM Volume"] = AudioManager.GetCategoryVolume("BGM").ToString();
                preferences["SFX Volume"] = AudioManager.GetCategoryVolume("SFX").ToString();

                string[] lines = new string[4];
                int counter = 0;
                foreach (var item in preferences)
                {
                    lines[counter] = item.Key + "=" + item.Value;
                    counter++;
                }

                File.WriteAllLines(filePath, lines);
            }

            // Set the values again
            foreach(var item in preferences)
            {
                switch(item.Key)
                {
                    case "Gamma":
                        {
                            Camera.Gamma =  float.Parse(item.Value);
                        }
                        break;
                    case "Master Volume":
                        {
                            AudioManager.SetMasterVolume(float.Parse(item.Value));  
                        }
                        break;
                    case "BGM Volume":
                        {
                            AudioManager.SetCategoryVolume("BGM", float.Parse(item.Value));
                        }
                        break;
                    case "SFX Volume":
                        {
                            AudioManager.SetCategoryVolume("SFX", float.Parse(item.Value));
                        }
                        break;
                }
            }
        }

        public static void RestoreDefaults()
        {
            Camera.Gamma = 45.0f;

            

            AudioManager.SetMasterVolume(1.0f);
            AudioManager.SetCategoryVolume("BGM",1.0f);
            AudioManager.SetCategoryVolume("SFX",1.0f);
            
        }

        public static void SavePreferences()
        {
            if (string.IsNullOrEmpty(filePath))
            {
                filePath = Application.GetFilePath("GamePreference.txt");
            }

            Dictionary<string, string> preferences = new Dictionary<string, string>();

            // We only got 4 settings to save
            // We just hardcode the default by reading from the engine values
            preferences["Gamma"] = Camera.Gamma.ToString();
            preferences["Master Volume"] = AudioManager.GetMasterVolume().ToString();
            preferences["BGM Volume"] = AudioManager.GetCategoryVolume("BGM").ToString();
            preferences["SFX Volume"] = AudioManager.GetCategoryVolume("SFX").ToString();

            string[] lines = new string[4];
            int counter = 0;
            foreach (var item in preferences)
            {
                lines[counter] = item.Key + "=" + item.Value;
                counter++;
            }

            File.WriteAllLines(filePath, lines);
        }
    }
}
