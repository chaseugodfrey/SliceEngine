using SliceEngine;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceScript.Source.Core.Systems
{
    public class AudioManager
    {
        public static void StopAllSounds()
        {
            FunctionCalls.Audio_StopAllSound();
        }

        public static float GetCategoryVolume(string category)
        {
            return FunctionCalls.Audio_GetCategoryVolume(category);
        }

        public static void SetCategoryVolume(string category, float volume)
        {
            FunctionCalls.Audio_SetCategoryVolume(category, ref volume);
        }

        public static void SetMasterVolume(float volume)
        {
            FunctionCalls.Audio_SetMasterVolume(volume);
        }

        public static float GetMasterVolume()
        {
            return FunctionCalls.Audio_GetMasterVolume();
        }
    }
}
