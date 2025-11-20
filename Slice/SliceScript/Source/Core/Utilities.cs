using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace SliceEngine
{
    public struct Utilities
    {
        public const float Epsilon = 1.401298E-45f;

        public static float Clamp(float value, float min, float max)
        {
            if (value < min) return min;
            if (value > max) return max;
            return value;
        }

        public static float Lerp(float a, float b, float t)
        {
            return a + (b - a) * t;
        }

        public static float InverseLerp(float a, float b, float value)
        {
            if (a == b) return 0f;
            return Clamp((value - a) / (b - a), 0f, 1f);
        }

        public static float EaseIn(float t) => t * t;
        public static float EaseOut(float t) => 1 - (1 - t) * (1 - t);
        public static float EaseInOut(float t)
        {
            return (t < 0.5f)
                ? 2f * t * t
                : 1f - (float)Math.Pow(-2f * t + 2f, 2f) / 2f;
        }


        public static float LerpAngle(float a, float b, float t)
        {
            float delta = Repeat((b - a), 360f);
            if (delta > 180f) delta -= 360f;
            return a + delta * Clamp(t, 0f, 1f);
        }

        public static float Repeat(float value, float length)
        {
            return value - (float)Math.Floor(value / length) * length;
        }

        public static float DeltaAngle(float a, float b)
        {
            float delta = Repeat((b - a), 360f);
            if (delta > 180f) delta -= 360f;
            return delta;
        }

        public static float Abs(float value)
        {
            return (value < 0f) ? -value : value;
        }

        public static float Deg2Rad(float degrees)
        {
            return degrees * (float)Math.PI / 180f;
        }

        public static float Rad2Deg(float radians)
        {
            return radians * 180f / (float)Math.PI;
        }

        public static Vector3 Parabola(Vector3 start, Vector3 end, float height, float t)
        {
            // Linear interpolation
            Vector3 pos = Vector3.Lerp(start, end, t);

            // Add parabolic height using quadratic formula
            float parabola = 4f * height * t * (1f - t);

            pos.y += parabola;
            return pos;
        }

        // Parabolas
        public static IEnumerator ParabolaCoroutine(Transform transform, Vector3 start, Vector3 end, float height, float duration)
        {
            float time = 0f;

            while (time < duration)
            {
                float t = time / duration;
                transform.Position = Parabola(start, end, height, t);
                time += Time.deltaTime;
                yield return null;
            }

            // Ensure final position exactly matches end
            transform.Position = end;
        }
    }
}
