using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public static class Cursor
    {
        public enum STATE : int
        {
            DEFAULT = 0,
            HIDDEN = 1,
            CONFINED = 2,
            DISABLED = 3
        }

        public static STATE state
        {
            get
            {
                return (STATE)FunctionCalls.Input_GetCursorState();
            }

            set
            {
                FunctionCalls.Input_SetCursorState((int)value);
            }
        }
    }

    public class Input
    {
        public static bool IsKeyPressed(Keys key)
        {
            return FunctionCalls.Input_IsKeyPressed(key);
        }

        public static bool IsKeyDown(Keys key)
        {
            return FunctionCalls.Input_IsKeyDown(key);
        }

        public static bool IsKeyReleased(Keys keyCode)
        {
            return FunctionCalls.Input_IsKeyReleased(keyCode);
        }

        public static bool IsMousePressed(MouseButtons button)
        {
            return FunctionCalls.Input_IsMousePressed(button);
        }

        public static bool IsMouseDown(MouseButtons button)
        {
            return FunctionCalls.Input_IsMouseDown(button);
        }

        public static bool IsMouseReleased(MouseButtons button)
        {
            return FunctionCalls.Input_IsMouseReleased(button);
        }

        public static Vector2 GetMousePosition()
        {
            FunctionCalls.Input_GetMousePosition(out Vector2 position);
            return position;
        }

        public static Vector2 GetMouseDelta()
        {
            FunctionCalls.Input_GetMouseDelta(out Vector2 delta);
            return delta;
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