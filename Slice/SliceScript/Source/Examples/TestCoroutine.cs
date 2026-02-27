using System;
using System.Collections;
using SliceEngine;

namespace SliceEngine
{
    public class TestCoroutine : SliceBehaviour
    {
        private bool waitFlag = false;
        private Coroutine stoppedCoroutine;

        public override void OnCreate()
        {
            SliceLog.Log("==== Coroutine Stress Test Begin ====\n");

            CoroutineManager.StopAllCoroutines();

            StartCoroutine(SimpleRoutine());
            StartCoroutine(TimedRoutine());
            StartCoroutine(WaitUntilRoutine());
            stoppedCoroutine = StartCoroutine(StopMidRoutine());
            StartCoroutine(ExceptionRoutine());
            StartCoroutine(ParentRoutine());

            SimulateFrames(60, 0.016f);

            SliceLog.Log("\n==== Coroutine Stress Test End ====");
        }

        private void SimulateFrames(int frameCount, float dt)
        {
            for (int i = 0; i < frameCount; i++)
            {
                SliceLog.Log($"\n--- Frame {i} ---");

                // Flip WaitUntil flag at frame 60 (~1 sec)
                if (i == 60)
                {
                    SliceLog.Log(">>> waitFlag set TRUE");
                    waitFlag = true;
                }

                // Stop coroutine at frame 30
                if (i == 30)
                {
                    SliceLog.Log(">>> Stopping StopMidRoutine");
                    CoroutineManager.StopCoroutine(stoppedCoroutine);
                }

                // Pause owner at frame 100
                if (i == 100)
                {
                    SliceLog.Log(">>> Owner paused");
                    this.activeSelf = false;
                }

                // Resume owner at frame 130
                if (i == 130)
                {
                    SliceLog.Log(">>> Owner resumed");
                    this.activeSelf = true;
                }
                CoroutineManager.OnUpdate(dt);
                
            }
        }

        // ------------------------------------------------------------
        // TEST ROUTINES
        // ------------------------------------------------------------

        private IEnumerator SimpleRoutine()
        {
            SliceLog.Log("SimpleRoutine START");

            for (int i = 0; i < 3; i++)
            {
                SliceLog.Log($"SimpleRoutine step {i}");
                yield return null;
            }

            SliceLog.Log("SimpleRoutine END");
        }

        private IEnumerator TimedRoutine()
        {
            SliceLog.Log("TimedRoutine START");
            yield return new WaitForSeconds(1.0f);
            SliceLog.Log("TimedRoutine resumed after 1 second");
        }

        private IEnumerator WaitUntilRoutine()
        {
            SliceLog.Log("WaitUntilRoutine START");
            yield return new WaitUntil(() => waitFlag);
            SliceLog.Log("WaitUntilRoutine resumed (flag true)");
        }

        private IEnumerator StopMidRoutine()
        {
            SliceLog.Log("StopMidRoutine START");
            yield return new WaitForSeconds(2f);
            SliceLog.Log("StopMidRoutine SHOULD NOT PRINT");
        }

        private IEnumerator ExceptionRoutine()
        {
            SliceLog.Log("ExceptionRoutine START");
            yield return null;
            throw new Exception("Intentional coroutine exception");
        }

        private IEnumerator ParentRoutine()
        {
            SliceLog.Log("ParentRoutine START");
            StartCoroutine(ChildRoutine());
            yield return new WaitForSeconds(2f);
            SliceLog.Log("ParentRoutine END");
        }

        private IEnumerator ChildRoutine()
        {
            SliceLog.Log("ChildRoutine START");
            yield return new WaitForSeconds(2f);
            SliceLog.Log("ChildRoutine END");
        }
    }
}
