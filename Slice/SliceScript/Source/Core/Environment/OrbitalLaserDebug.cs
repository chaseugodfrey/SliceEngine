using SliceEngine;
using System;
using static System.Runtime.CompilerServices.RuntimeHelpers;

namespace SliceEngine
{
    public class OrbitalLaserDebug : SliceBehaviour
    {
        public string hintPrefabName = "OrbitalLaser";
        public float spawnRadius = 200f;
        public float yOffset = 0.1f;
        public float trackDuration = 2f;

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
            Vector3 playerPos = Bootstrap.Player.transform.WorldPosition;

            float randomAngleRad = SliceRandom.RangeFloat(0f, 360f) * ((float)Math.PI / 180f);
            float randomRadius = SliceRandom.RangeFloat(spawnRadius * 0.5f, spawnRadius);

            Vector3 spawnPos = new Vector3(
                playerPos.x + (float)Math.Cos(randomAngleRad) * randomRadius,
                playerPos.y + yOffset,
                playerPos.z + (float)Math.Sin(randomAngleRad) * randomRadius
            );

            string hintPath = "Prefabs/" + hintPrefabName + ".prefab";
            GameObject hint = CreateGameObject(hintPath);
            hint.GetComponent<Transform>().Position = spawnPos;

            if (hint.Has<OrbitalLaserHint>())
                hint.As<OrbitalLaserHint>().trackDuration = trackDuration;
        }
    }
}