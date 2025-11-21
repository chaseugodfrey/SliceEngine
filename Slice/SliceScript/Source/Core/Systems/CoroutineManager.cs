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

        public void OnCreate()
        {
            coroutines.Clear();
            newCoroutines.Clear();
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

                if (c.CurrentYield is IYieldInstruction yi)
                {
                    // Update if it needs deltaTime
                    if (yi is WaitForSeconds wfs) wfs.Update(dt);

                    if (!yi.IsDone()) continue; // still waiting
                }

                bool alive = c.Enumerator.MoveNext();

                if (!alive)
                {
                    // Coroutine finished
                    coroutines.RemoveAt(i);
                    continue;
                }

                // Store current yield instruction
                if (c.Enumerator.Current is IYieldInstruction yieldInstr)
                    c.CurrentYield = yieldInstr;
                else
                    c.CurrentYield = c.Enumerator.Current;
            }
        }

        public static bool IsRunning(Coroutine coroutine)
        {
            return coroutines.Contains(coroutine) || newCoroutines.Contains(coroutine);
        }
    }

    public interface IYieldInstruction
    {
        bool IsDone();
    }
    public class WaitForSeconds : IYieldInstruction
    {
        private float timeRemaining;

        public WaitForSeconds(float seconds)
        {
            timeRemaining = seconds;
        }

        public bool IsDone()
        {
            return timeRemaining <= 0f;
        }
        public void Update(float dt)
        {
            timeRemaining -= dt;
        }
    }

    public class WaitUntil : IYieldInstruction
    {
        private readonly Func<bool> predicate;

        public WaitUntil(Func<bool> predicate)
        {
            this.predicate = predicate;
        }

        public bool IsDone()
        {
            return predicate();
        }
    }
    public class WaitWhile : IYieldInstruction
    {
        private readonly Func<bool> predicate;

        public WaitWhile(Func<bool> predicate)
        {
            this.predicate = predicate;
        }

        public bool IsDone()
        {
            return !predicate(); // complete when predicate is false
        }
    }

    public class WaitForCoroutine : IYieldInstruction
    {
        private readonly CoroutineManager.Coroutine target;

        public WaitForCoroutine(CoroutineManager.Coroutine coroutine)
        {
            target = coroutine;
        }

        public bool IsDone()
        {
            return !CoroutineManager.IsRunning(target);
        }
    }

    public class WaitForEndOfFrame : IYieldInstruction
    {
        private bool hasWaited = false;

        public bool IsDone()
        {
            if (!hasWaited)
            {
                hasWaited = true;
                return false;
            }
            return true;
        }
    }
}
