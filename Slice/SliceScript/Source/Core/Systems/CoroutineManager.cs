using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.CompilerServices;

namespace SliceEngine
{
    public class CoroutineManager
    {
        public class Coroutine
        {
            public IEnumerator Enumerator;
            public SliceBehaviour Owner;
            public object CurrentYield = null;
        }   

        private static readonly List<Coroutine> coroutines = new List<Coroutine>();
        private static readonly List<Coroutine> newCoroutines = new List<Coroutine>();

        public static void StartCoroutine(Coroutine coroutine)
        {
            if (coroutine.Enumerator == null || coroutine.Owner == null)
                return;

            newCoroutines.Add(coroutine);
        }

        public static void StartCoroutine(IEnumerator routine, SliceBehaviour owner)
        {
            if (routine == null || owner == null)
                return;

            newCoroutines.Add(new Coroutine
            {
                Enumerator = routine,
                Owner = owner,
                CurrentYield = null
            });
        }

        public static void StopCoroutine(Coroutine coroutine)
        {
            coroutines.RemoveAll(c => c == coroutine);
            newCoroutines.RemoveAll(c => c == coroutine);
        }

        public static void StopCoroutine(IEnumerator routine, SliceBehaviour owner)
        {
            coroutines.RemoveAll(c => c.Enumerator == routine && c.Owner == owner);
            newCoroutines.RemoveAll(c => c.Enumerator == routine && c.Owner == owner);
        }

        public static void StopAllCoroutines(SliceBehaviour owner = null)
        {
            if (owner == null)
            {
                coroutines.Clear();
                newCoroutines.Clear();
                return;
            }

            // Remove only coroutines belonging to this owner
            coroutines.RemoveAll(c => c.Owner == owner);
            newCoroutines.RemoveAll(c => c.Owner == owner);
        }

        public void OnUpdate(float dt)
        {
            // Register new coroutines
            if (newCoroutines.Count > 0)
            {
                coroutines.AddRange(newCoroutines);
                newCoroutines.Clear();
            }

            for (int i = coroutines.Count - 1; i >= 0; i--)
            {
                var c = coroutines[i];

                // Owner destroyed? Stop coroutine
                if (c.Owner == null)
                {
                    coroutines.RemoveAt(i);
                    continue;
                }

                // Owner inactive? Pause coroutine
                if (!c.Owner.activeSelf)
                    continue;

                // Handle WaitForSeconds
                if (c.CurrentYield is WaitForSeconds wait)
                {
                    if (!wait.Tick(dt))
                        continue; // still waiting
                }

                bool alive = c.Enumerator.MoveNext();

                if (!alive)
                {
                    // Coroutine finished
                    coroutines.RemoveAt(i);
                    continue;
                }

                c.CurrentYield = c.Enumerator.Current;
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
