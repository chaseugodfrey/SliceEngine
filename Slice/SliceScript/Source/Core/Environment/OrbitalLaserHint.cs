using SliceEngine;
using System;

namespace SliceEngine
{
    public class OrbitalLaserHint : SliceBehaviour
    {
        public float trackDuration = 2f;
        public float moveSpeed = 15f;
        public string laserPrefabName = "FX_OrbitalLaser";

        private float timer = 0f;
        private bool done = false;
        private Vector3 cachedPosition;

        public override void OnUpdate(float dt)
        {
            base.OnUpdate(dt);

            if (done) return;

            Vector3 currentPos = this.GetComponent<Transform>().Position;
            Vector3 playerPos = Bootstrap.Player.transform.WorldPosition;

            // Target is player position but keep own Y
            Vector3 target = new Vector3(playerPos.x, currentPos.y, playerPos.z);
            Vector3 direction = (target - currentPos);

            // Only move if not already on top of the player
            if (direction.Magnitude() > 0.05f)
            {
                Vector3 move = direction.Normalize() * moveSpeed * dt;

                // Don't overshoot
                if (move.Magnitude() > direction.Magnitude())
                    cachedPosition = target;
                else
                    cachedPosition = currentPos + move;
            }
            else
            {
                cachedPosition = currentPos;
            }

            this.GetComponent<Transform>().Position = cachedPosition;

            timer += dt;
            if (timer >= trackDuration)
                done = true;
        }

        public override void OnEntityDestroy(uint id)
        {
            string laserPath = "Prefabs/" + laserPrefabName + ".prefab";
            GameObject laser = CreateGameObject(laserPath);
            laser.GetComponent<Transform>().Position = cachedPosition;
        }
    }
}