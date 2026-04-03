using SliceEngine;
using System;
using System.Collections.Generic;
using System.Diagnostics.Contracts;
using System.IO.Pipes;
using System.Security.Permissions;


namespace SliceEngine
{
    public class DynamicLight : SliceBehaviour
    {
        const int MODE_NONE = 0;
        const int MODE_PINGPONG = 1;
        const int MODE_PULSE = 2;
        const int MODE_FLICKER = 3;
        const int MODE_RAINBOW = 4;

        public int mode = MODE_PINGPONG;

        // Color settings
        public Vector3 colorA = new Vector3(1, 0, 0); // Red
        public Vector3 colorB = new Vector3(0, 0, 1); // Blue
        public float colorDuration = 2.0f;

        // Intensity settings
        public float minIntensity = 0.5f;
        public float maxIntensity = 2.0f;

        // Flicker settings
        public float flickerSpeed = 10.0f;
        public float flickerAmount = 0.3f;

        public float speed = 1.0f;
        private float timer = 0.0f;
        private Light light;


        public override void OnCreate()
        {
            base.OnCreate();
            light = GetComponent<Light>();
        }

        public override void OnFixedUpdate(float dt)
        {
            base.OnFixedUpdate(dt);

            timer += dt * speed;

            switch (mode)
            {
                case MODE_PINGPONG:
                    UpdatePingPongColor();
                    break;

                case MODE_PULSE:
                    UpdatePulse();
                    break;

                case MODE_FLICKER:
                    UpdateFlicker();
                    break;

                case MODE_RAINBOW:
                    UpdateRainbow();
                    break;
            }
        }

        void UpdatePingPongColor()
        {
            float t = (float)(Math.Sin(timer / colorDuration * Math.PI) * 0.5 + 0.5);
            light.Color = Utilities.Lerp(colorA, colorB, t);
        }

        void UpdatePulse()
        {
            float t = (float)(Math.Sin(timer * 2.0f) * 0.5 + 0.5);
            light.Intensity = Lerp(minIntensity, maxIntensity, t);
        }

        void UpdateFlicker()
        {
            float noise = (float)(new Random().NextDouble());
            light.Intensity = Lerp(minIntensity, maxIntensity, noise * flickerAmount);
        }

        void UpdateRainbow()
        {
            float hue = (timer * 0.2f) % 1.0f;
            float pulse = (float)(Math.Sin(timer * 2.0f) * 0.5 + 0.5);
            light.Color = Utilities.FromHSV(hue, 1.0f, pulse);
        }

        float Lerp(float a, float b, float t)
        {
            return a + (b - a) * t;
        }
    }
}