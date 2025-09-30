using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{ 
    public static class FunctionCalls
    {
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Transform_GetPosition(uint entityID, out Vector3 position);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Transform_SetPosition(uint entityID, ref Vector3 position);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool IsKeyPressed(Keys key);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool IsKeyDown(Keys key);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Log(string msg);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void LogWarn(string msg);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void LogError(string msg);

    }
}
