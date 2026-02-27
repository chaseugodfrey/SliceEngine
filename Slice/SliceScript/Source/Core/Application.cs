using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class Application
    {
        public static string RootPath => FunctionCalls.Application_GetFilePath();

        //public static string streamingAssetsPath => Path.Combine(RootPath, "Assets");

        public static string GetFilePath(string relativePath)
        {
            // Ensures the path is always anchored to the EXE location
            return Path.Combine(RootPath, relativePath);
        }
    }
}
