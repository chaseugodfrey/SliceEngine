using SliceEngine;
using System;

namespace SliceEngine
{
    public class OrbitalLaserLight : SliceBehaviour
    {
        public override void OnCreate()
        {
            base.OnCreate();
        }
        
        public void SetupLaser(Vector3 spawnPos, float diameter, float laserTime)
        {
            GameObject[] objs = gameObject.GetAllChildren();

            GameObject ringObj = null;

            foreach (GameObject obj in objs)
            {
                if (obj.tag == "OrbitalLaserRing")
                    ringObj = obj;
            }

            var ps = GetComponent<ParticleSystem>();

            transform.Position = spawnPos;

            ps.Scale = new Vector3(diameter, ps.Scale.y, diameter); ;
            ps.Duration = laserTime;
            ps.Lifetime = laserTime;

            if (ringObj != null)
            {
                var ringPs = ringObj.GetComponent<ParticleSystem>();
                ringPs.Scale = new Vector3(diameter, diameter, 1f);
                ringPs.Duration = laserTime * 0.8f;
                ringPs.Lifetime = laserTime * 0.8f;
            }
        }
    }
}