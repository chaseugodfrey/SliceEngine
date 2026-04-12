using System;
using System.Collections.Generic;
using System.Diagnostics.CodeAnalysis;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class Time
    {
        // Fuck you microsoft
        [SuppressMessage("Style", "IDE1006:Naming Styles", Justification = "Unity-style lowercase naming for familiarity")]
        public static float deltaTime { get; private set; }

        [SuppressMessage("Style", "IDE1006:Naming Styles", Justification = "Unity-style lowercase naming for familiarity")]
        public static float time { get; private set; }

        [SuppressMessage("Style", "IDE1006:Naming Styles", Justification = "Unity-style lowercase naming for familiarity")]
        public static float deltaTimeUnscaled 
        {
            get
            {
                return FunctionCalls.Time_GetDeltaTimeUnscaled();
            }
        }

        [SuppressMessage("Style", "IDE1006:Naming Styles", Justification = "Unity-style lowercase naming for familiarity")]
        public static float timeScale
        {
            get
            {
                return FunctionCalls.Time_GetTimeScale();
            }
            set
            {
                FunctionCalls.Time_SetTimeScale(value);
            }
        }

        //public static int frameCount = 0;

        public static float fixedDeltaTime = 1f/60f;
        //public static float fixedTime = 0.0f;

        // Idk if we want to do this for timeScale
        // public static float timeScale = 1.0f;

        //public static float scaledDeltaTime => deltaTime * timeScale;
        public static float fps => deltaTime > 0 ? 1f / deltaTime : 0f;

        public void OnCreate()
        {
            time = 0.0f;
            //fixedTime = 0.0f;
        }

        // Done this way to prevent overhead caused by reaching all the way into the engine to get its delta time in framrate manager
        public void OnUpdate(float dt)
        {
            deltaTime = dt;
            time += dt;
        }        
        // Not working idk why
        //public void OnFixedUpdate(float dt)
        //{
        //    fixedDeltaTime = dt;
        //    fixedTime += fixedDeltaTime;
        //}       
    }
}
