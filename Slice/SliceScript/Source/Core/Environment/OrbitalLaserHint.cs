using SliceEngine;
using System;

namespace SliceEngine
{
    public class OrbitalLaserHint : SliceBehaviour
    {
        bool isRunning = false;
        float hintDuration;
        float laserDuration;
        float moveSpeed;
        float d_diameter;

        Vector3 cachedPosition;

        public override void OnUpdate(float dt)
        {
            isRunning = true;
        }

        public void SetupLaser(Vector3 spawnPos, float diameter, float hintTime, float laserTime, float trackSpeed = 15.0f)
        {
            var ps = GetComponent<ParticleSystem>();

            d_diameter = diameter;

            transform.Position = spawnPos;

            hintDuration = hintTime;
            moveSpeed = trackSpeed;

            ps.Scale = new Vector3(diameter, ps.Scale.y, diameter); ;
            ps.Duration = hintDuration;
            ps.Lifetime = hintDuration;

            GameObject[] rings = gameObject.GetAllChildren();
            GameObject ring1 = null;
            GameObject ring2 = null;
            GameObject ring3 = null;

            Console.WriteLine("a");

            foreach (var r in rings)
            {
                if (r.tag == "OrbitalHint1")
                    ring1 = r;
                if (r.tag == "OrbitalHint2")
                    ring2 = r;
                if (r.tag == "OrbitalHint3")
                    ring3 = r;
            }

            if (ring1 != null)
            {
                var ps1 = ring1.GetComponent<ParticleSystem>();
                ps1.Scale = new Vector3(0.01f, diameter / 10.0f, diameter / 10.0f);
            }

            if (ring2 != null)
            {
                var ps1 = ring2.GetComponent<ParticleSystem>();
                ps1.Scale = new Vector3(0.01f, diameter / 5.0f, diameter / 5.0f);
            }

            if (ring3 != null)
            {
                var ps1 = ring2.GetComponent<ParticleSystem>();
                ps1.Scale = new Vector3(0.01f, diameter / 5.0f, diameter / 5.0f);
            }
        }
    }
}