using SliceEngine;
using System;
using static System.Runtime.CompilerServices.RuntimeHelpers;

namespace SliceEngine
{
    public class OrbitalLaserDebug : SliceBehaviour
    {
        public string prefabName;
        public float diameter;
        public float hintDuration;
        public float laserDuration;
        public Vector3 spawnPos;

        public override void OnUpdate(float dt)
        {
            base.OnUpdate(dt);

            if (Input.IsKeyPressed(Keys.KEY_M))
            {
                SpawnHint();
            }
        }

        private void SpawnHint()
        {
            string path = "Prefabs/" + prefabName + ".prefab";
            GameObject prefab = CreateGameObject(path);
            prefab.As<OrbitalLaser>().SetupLaser(spawnPos, diameter, hintDuration, laserDuration, 0);
        }
    }
}