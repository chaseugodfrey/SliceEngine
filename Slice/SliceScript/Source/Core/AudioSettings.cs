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
            FunctionCalls.Audio_PlaySFX(message);
        }
    }
}
