using SliceEngine;
using SliceScript.Source.Core;
using System;
using System.Collections.Generic;

namespace SliceEngine
{
    public class L1FallingPillar : SliceBehaviour
    {
        public Transform go;

        public Vector3 startOffset;
        public Vector3 endOffset;

        public float moveDuration = 2f;

        public float startPauseDuration = 1f;
        public float middlePauseDuration = 1f;
        public float endPauseDuration = 1f;

        public bool isMoving = true;
        public bool isLoop = true;
        public bool isMoveOnContactWithPlayer = true;

        private Vector3 basePosition;
        private float timer = 0f;

        public override void OnCreate()
        {
            go = GetComponent<Transform>();
            basePosition = go.Position;
        }

        public override void OnUpdate(float dt)
        {
            if (!isMoving)
            {
                return;
            }

            timer += dt;
            
            float cycleLength =
                moveDuration + startPauseDuration +
                moveDuration + middlePauseDuration +
                moveDuration + endPauseDuration +
                moveDuration + middlePauseDuration;

            if (!isLoop && timer >= cycleLength)
            {
                go.Position = basePosition; // final resting position
                isMoving = false;
                return;
            }

            float cycleTime;

            if (isLoop)
            {
                cycleTime = timer % cycleLength;
            }
            else
            {
                cycleTime = Math.Min(timer, cycleLength);
            }

            float t;
            Vector3 from;
            Vector3 to;

            float timeCursor = 0f;

            if (cycleTime < timeCursor + moveDuration)
            {
                from = basePosition;
                to = basePosition + startOffset;
                t = (cycleTime - timeCursor) / moveDuration;
            }
            else
            {
                timeCursor += moveDuration;

                // -------- START PAUSE --------
                if (cycleTime < timeCursor + startPauseDuration)
                {
                    go.Position = basePosition + startOffset;
                    return;
                }
                timeCursor += startPauseDuration;

                // -------- START INIT --------
                if (cycleTime < timeCursor + moveDuration)
                {
                    from = basePosition + startOffset;
                    to = basePosition;
                    t = (cycleTime - timeCursor) / moveDuration;
                }
                else
                {
                    timeCursor += moveDuration;

                    // -------- MIDDLE PAUSE --------
                    if (cycleTime < timeCursor + middlePauseDuration)
                    {
                        go.Position = basePosition;
                        return;
                    }
                    timeCursor += middlePauseDuration;

                    // -------- INIT END --------
                    if (cycleTime < timeCursor + moveDuration)
                    {
                        from = basePosition;
                        to = basePosition + endOffset;
                        t = (cycleTime - timeCursor) / moveDuration;
                    }
                    else
                    {
                        timeCursor += moveDuration;

                        // -------- END PAUSE --------
                        if (cycleTime < timeCursor + endPauseDuration)
                        {
                            go.Position = basePosition + endOffset;
                            return;
                        }
                        timeCursor += endPauseDuration;

                        // -------- END INIT --------
                        if (cycleTime < timeCursor + moveDuration)
                        {
                            from = basePosition + endOffset;
                            to = basePosition;
                            t = (cycleTime - timeCursor) / moveDuration;
                        }
                        else
                        {
                            // -------- FINAL MIDDLE PAUSE --------
                            go.Position = basePosition;
                            return;
                        }
                    }
                }
            }

            t = Utilities.SmoothStep(0f, 1f, t);
            go.Position = Utilities.Lerp(from, to, t);
        }

        public override void OnCollideEnter(uint other)
        {
            if (isMoveOnContactWithPlayer)
            {
                if (other == Bootstrap.Player.gameObject.mID)
                {
                    isMoving = true;                    
                }
            }
        }

        public override void OnCollideExit(uint other)
        {
            if (isMoveOnContactWithPlayer)
            {
                if (other == Bootstrap.Player.gameObject.mID)
                {
                    isMoving = true;
                    // need to set the player parent to the platform
                }
            }
        }
    }
}
