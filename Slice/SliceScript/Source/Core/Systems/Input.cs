using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
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

        public static bool IsKeyReleased(Keys keyCode)
        {
            return FunctionCalls.IsKeyReleased(keyCode);
        }

        public static bool IsMousePressed(MouseButtons button)
        {
            return FunctionCalls.IsMousePressed(button);
        }

        public static bool IsMouseDown(MouseButtons button)
        {
            return FunctionCalls.IsMouseDown(button);
        }

        public static bool IsMouseReleased(MouseButtons button)
        {
            return FunctionCalls.IsMouseReleased(button);
        }

        public static Vector2 GetMousePosition()
        {
            FunctionCalls.GetMousePosition(out Vector3 position);
            Vector2 pos;
            pos.x = position.x;
            pos.y = position.y;
            return pos;
        }

        public static void AM_EnableMap(string map, bool enable)
        {
            FunctionCalls.AM_EnableMap(map, enable);
        }

        public static bool AM_PerformedThisFrame(string map, string action)
        {
            return FunctionCalls.AM_PerformedThisFrame(map, action);
        }

        public static float AM_GetValue1D(string map, string action)
        {
            return FunctionCalls.AM_GetValue1D(map, action);
        }

        public static void AM_GetValue2D(string map, string action, out Vector2 value)
        {
            FunctionCalls.AM_GetValue2D(map, action, out value);
        }

    }
}