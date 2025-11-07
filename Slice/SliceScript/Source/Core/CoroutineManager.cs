using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.CompilerServices;

namespace SliceEngine
{
    public class CoroutineManager : SliceBehaviour
    {
        private static readonly List<IEnumerator> coroutines = new List<IEnumerator>();
        private static readonly List<IEnumerator> newCoroutines = new List<IEnumerator>();

        public static void StartCoroutine(IEnumerator routine)
        {
            if (routine != null)
                newCoroutines.Add(routine);
        }

        public static void StopAllCoroutines()
        {
            coroutines.Clear();
            newCoroutines.Clear();
        }

        public override void OnUpdate(float dt)
        {
            // Add new coroutines started this frame
            if (newCoroutines.Count > 0)
            {
                coroutines.AddRange(newCoroutines);
                newCoroutines.Clear();
            }

            
            // Step all coroutines
            for (int i = coroutines.Count - 1; i >= 0; i--)
            {
                var c = coroutines[i];

                bool alive = c.MoveNext();
                if (!alive)
                    coroutines.RemoveAt(i);
            }

            Console.WriteLine("Coroutines is running");
        }

        // Called from C++ each frame
        //[MethodImpl(MethodImplOptions.InternalCall)]
        //public static extern void Internal_Update(float deltaTime);

        //public static class CoroutineBridge
        //{
        //    // This method will be called by C++ somewhere
        //    public static void OnUpdate(float deltaTime)
        //    {
        //        CoroutineManager.Update(deltaTime);
        //    }
        //}

    }
}
