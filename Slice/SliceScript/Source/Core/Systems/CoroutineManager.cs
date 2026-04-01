using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.CompilerServices;

namespace SliceEngine
{
    public class Coroutine
    {
        public IEnumerator Enumerator;
        public SliceBehaviour Owner;
        public object CurrentYield = null;
        public bool isActive = true;
    }

    public class CoroutineManager
    { 
        private static readonly List<Coroutine> coroutines = new List<Coroutine>();
        private static readonly List<Coroutine> newCoroutines = new List<Coroutine>();

        public static Coroutine StartCoroutine(Coroutine coroutine)
        {
            if (coroutine == null || coroutine.Enumerator == null || coroutine.Owner == null)
                return null;

            newCoroutines.Add(coroutine);
            return coroutine;
        }

        public static Coroutine StartCoroutine(IEnumerator routine, SliceBehaviour owner)
        {
            if (routine == null || owner == null)
                return null;

            Coroutine c = new Coroutine
            {
                Enumerator = routine,
                Owner = owner,
                CurrentYield = null
            };

            newCoroutines.Add(c);
            return c;
        }

        public static void StopCoroutine(Coroutine coroutine)
        {
            if (coroutine == null) return;
            coroutine.isActive = false;
        }

        public static void StopCoroutine(IEnumerator routine, SliceBehaviour owner)
        {
            foreach (var c in coroutines)
            {
                if (c.Enumerator == routine && c.Owner == owner)
                    c.isActive = false;
            }

            foreach (var c in newCoroutines)
            {
                if (c.Enumerator == routine && c.Owner == owner)
                    c.isActive = false;
            }
        }

        public static void StopAllCoroutines(SliceBehaviour owner = null)
        {
            if (owner == null)
            {
                foreach (var c in coroutines)
                    c.isActive = false;

                foreach (var c in newCoroutines)
                    c.isActive = false;

                return;
            }

            foreach (var c in coroutines)
                if (c.Owner == owner)
                    c.isActive = false;

            foreach (var c in newCoroutines)
                if (c.Owner == owner)
                    c.isActive = false;
        }

        public static void OnEnd()
        {
            foreach (var c in coroutines)
                c.isActive = false;

            foreach (var c in newCoroutines)
                c.isActive = false;

            coroutines.Clear();
            newCoroutines.Clear();
        }
        public static void OnCreate()
        {
            coroutines.Clear();
            newCoroutines.Clear();
        }

        public static void OnUpdate(float dt)
        {
            newCoroutines.RemoveAll(c => !c.isActive);

            // Register new coroutines
            if (newCoroutines.Count > 0)
            {
                coroutines.AddRange(newCoroutines);
                newCoroutines.Clear();
            }

            for (int i = coroutines.Count - 1; i >= 0; i--)
            {
                var c = coroutines[i];
               
                if (!c.isActive)
                    continue;

                // Owner destroyed? Stop coroutine
                if (c.Owner == null)
                {
                    c.isActive = false;
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

                try
                {
                    bool alive = c.Enumerator.MoveNext();

                    if (!alive)
                    {
                        // Coroutine finished
                        c.isActive = false;
                        continue;
                    }

                    // Store current yield instruction
                    if (c.Enumerator.Current is IYieldInstruction yieldInstr)
                        c.CurrentYield = yieldInstr;
                    else
                        c.CurrentYield = c.Enumerator.Current;
                }
                catch (Exception e)
                {
                    SliceLog.Error("Faulty Coroutine Caught and Removed: " + e.ToString());
                    c.isActive = false;
                }
            }
            coroutines.RemoveAll(c => !c.isActive);
        }

        public static void OnEntityDestroy(uint owner)
        {
            EntityDestroyed(owner);
        }

        public static bool IsRunning(Coroutine coroutine)
        {
            return coroutines.Contains(coroutine) || newCoroutines.Contains(coroutine);
        }

        public static void EntityDestroyed(uint owner)
        {
            for (int i = coroutines.Count - 1; i >= 0; i--)
            {
                var c = coroutines[i];
                if (c.Owner?.gameObject != null && c.Owner.gameObject.mID == owner)
                {
                    coroutines.RemoveAt(i);
                }
            }
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
        private readonly Coroutine target;

        public WaitForCoroutine(Coroutine coroutine)
        {
            target = coroutine;
        }

        public bool IsDone()
        {
            if (target == null) return true;
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
