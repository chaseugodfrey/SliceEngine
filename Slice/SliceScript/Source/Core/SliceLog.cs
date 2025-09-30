using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{ 
    public class SliceLog
    {
        public static void Log(string message)
        {
           // Console.WriteLine($"[LOG] {message}");
           FunctionCalls.Log(message);
        }
        public static void Warn(string message)
        {
            //Console.WriteLine($"[WARN] {message}");
            FunctionCalls.LogWarn(message);
        }
        public static void Error(string message)
        {
            FunctionCalls.LogError(message);
            //Console.WriteLine($"[ERROR] {message}");
        }
    }
}
