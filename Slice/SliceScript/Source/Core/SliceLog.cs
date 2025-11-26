using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;


namespace SliceEngine
{ 
    public class SliceLog
    {
        enum WarningLevel
        {
            INFO = 0,
            DEBUG = 1,
            WARNING = 2,
            ERROR = 3,
            CRITICAL = 4
        }

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

        [MethodImpl(MethodImplOptions.NoInlining)]
        public static void Console(
            params object[] args)
        {
            var st = new StackTrace(true);  // capture file/line info

            List<string> callStackLines = new List<string>();

            for (int i = 1; i < st.FrameCount; i++)
            {
                var frame = st.GetFrame(i);
                var method = frame.GetMethod();

                string file = frame.GetFileName() ?? "<unknown>";
                int line = frame.GetFileLineNumber();
                string caller = $"{method.DeclaringType?.FullName}.{method.Name}";

                callStackLines.Add($"{caller}  ({file}:{line})");
            }

            string msg = "";
            if (args != null && args.Length > 0)
                msg += " " + string.Join(" ", args);

            FunctionCalls.Debug_Console(callStackLines.ToArray(), msg, (int)WarningLevel.DEBUG);
        }
    }
}

