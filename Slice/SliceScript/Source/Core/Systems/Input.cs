using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class Input
    {
        public static bool IsKeyPressed(Keys key)
        {
            return FunctionCalls.IsKeyPressed(key);
        }

        public static bool IsKeyDown(Keys key)
        {
            return FunctionCalls.IsKeyDown(key);
        }
    }
}
