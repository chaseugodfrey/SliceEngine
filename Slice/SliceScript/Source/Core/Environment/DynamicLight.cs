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
        public enum LightMode
        {
            None,
            PingPongColor,
            PulseIntensity,
            Flicker,
            RainbowCycle
        }

        public LightMode mode = LightMode.PingPongColor;

        // Color settings
        //public Color colorA = new Color(1, 0, 0); // Red
        //public Color colorB = new Color(0, 0, 1); // Blue
        public float colorDuration = 2.0f;

        // Intensity settings
        public float minIntensity = 0.5f;
        public float maxIntensity = 2.0f;

        // Flicker settings
        public float flickerSpeed = 10.0f;
        public float flickerAmount = 0.3f;

        private float timer = 0.0f;
        //private Light light;

        public override void OnCreate()
        {
            base.OnCreate();
            //light = GetComponent<Light>();
        }

        public override void OnFixedUpdate(float dt)
        {
            base.OnFixedUpdate(dt);

            //if (light == null) return;

            timer += dt;

            switch (mode)
            {
                case LightMode.PingPongColor:
                    UpdatePingPongColor();
                    break;

                case LightMode.PulseIntensity:
                    UpdatePulse();
                    break;

                case LightMode.Flicker:
                    UpdateFlicker();
                    break;

                case LightMode.RainbowCycle:
                    UpdateRainbow();
                    break;
            }
        }

        void UpdatePingPongColor()
        {
            float t = (float)(Math.Sin(timer / colorDuration * Math.PI) * 0.5 + 0.5);
            //light.color = Color.Lerp(colorA, colorB, t);
        }

        void UpdatePulse()
        {
            float t = (float)(Math.Sin(timer * 2.0f) * 0.5 + 0.5);
            //light.intensity = Lerp(minIntensity, maxIntensity, t);
        }

        void UpdateFlicker()
        {
            float noise = (float)(new Random().NextDouble());
            //light.intensity = Lerp(minIntensity, maxIntensity, noise * flickerAmount);
        }

        void UpdateRainbow()
        {
            float hue = (timer * 0.2f) % 1.0f;
            //light.color = Color.FromHSV(hue, 1.0f, 1.0f);
        }

        float Lerp(float a, float b, float t)
        {
            return a + (b - a) * t;
        }
    }
}