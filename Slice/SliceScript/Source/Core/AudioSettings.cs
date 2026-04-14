using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class AudioSettings
    {
        public static void PlaySFX(string message)
        {
            // Console.WriteLine($"[LOG] {message}");
            FunctionCalls.Audio_PlaySFX(message, ref Vector3.Zero, 0);
        }

        public static GameObject PlaySFXWithGO(string message)
        {
            uint goID = FunctionCalls.Audio_PlaySFXWithGO(message, ref Vector3.Zero, 0);
            GameObject go = new GameObject(goID);

            return go;
        }

        public static void PlaySFX(string message, Vector3 position)
        {
            // Console.WriteLine($"[LOG] {message}");
            FunctionCalls.Audio_PlaySFX(message, ref position, 0);
        }

        public static void PlaySFX(string message, GameObject parent)
        {
            // Console.WriteLine($"[LOG] {message}");
            FunctionCalls.Audio_PlaySFX(message, ref Vector3.Zero, parent.mID);
        }

        public static void PlaySFX(string message, GameObject parent, Vector3 localPosition)
        {
            // Console.WriteLine($"[LOG] {message}");
            FunctionCalls.Audio_PlaySFX(message, ref localPosition, parent.mID);
        }

    }
}
