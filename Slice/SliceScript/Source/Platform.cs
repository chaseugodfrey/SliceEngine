using SliceEngine;
using SliceScript.Source.Core;
using System;
using System.Collections.Generic;

namespace SliceEngine
{
    public class Platform : SliceBehaviour
    {
        public Transform go;

        public Vector3 startOffset;
        public Vector3 endOffset;

        public float moveDuration = 2f;

        public float startPauseDuration = 1f;
        public float middlePauseDuration = 1f;
        public float endPauseDuration = 1f;

        private Vector3 basePosition;
        private float timer = 0f;

        public override void OnCreate()
        {
            go = GetComponent<Transform>();
            basePosition = go.Position;
        }

        public override void OnUpdate(float dt)
        {
            timer += dt;

            float cycleLength =
                moveDuration + startPauseDuration +
                moveDuration + middlePauseDuration +
                moveDuration + endPauseDuration +
                moveDuration + middlePauseDuration;

            float cycleTime = timer % cycleLength;

            float t;
            Vector3 from;
            Vector3 to;

            float timeCursor = 0f;

            // -------- INIT → START --------
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

                // -------- START → INIT --------
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

                    // -------- INIT → END --------
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

                        // -------- END → INIT --------
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
    }
}
