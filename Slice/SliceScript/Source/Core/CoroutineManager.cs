using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.CompilerServices;

namespace SliceEngine
{
    public class CoroutineManager
    {
        private class CoroutineState
        {
            public IEnumerator Enumerator;
            public object CurrentYield;
        }

        private static readonly List<CoroutineState> coroutines = new List<CoroutineState>();
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

        public void OnUpdate(float dt)
        {
            if (coroutines.Count == 0 && newCoroutines.Count == 0)
                return;

            foreach (var r in newCoroutines)
            {
                coroutines.Add(new CoroutineState { Enumerator = r, CurrentYield = null });
            }
            newCoroutines.Clear();


            // Step all coroutines
            for (int i = coroutines.Count - 1; i >= 0; i--)
            {
                var state = coroutines[i];
                var e = state.Enumerator;

                if (state.CurrentYield is WaitForSeconds wait)
                {
                    // Tick the wait
                    if (!wait.Tick(dt))
                        continue; // still waiting, skip this frame
                }

                // Move to next yield
                bool alive = e.MoveNext();

                if (!alive)
                {
                    // Coroutine finished
                    coroutines.RemoveAt(i);
                    continue;
                }

                state.CurrentYield = e.Current;
            }

        }
    }
    public class WaitForSeconds
    {
        public float TimeRemaining { get; private set; }

        public WaitForSeconds(float seconds)
        {
            TimeRemaining = seconds;
        }

        public bool Tick(float dt)
        {
            TimeRemaining -= dt;
            return TimeRemaining <= 0.0f;
        }
    }
}
