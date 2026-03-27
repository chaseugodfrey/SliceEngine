using SliceEngine;

namespace SliceEngine
{
    public class OrbitalLaserHint : SliceBehaviour
    {
        public float trackDuration = 5f;
        public string laserPrefabName = "FX_OrbitalLaser";

        private float timer = 0f;
        private bool done = false;
        private Vector3 cachedPosition; // cache in case Transform is gone on destroy

        public override void OnUpdate(float dt)
        {
            base.OnUpdate(dt);

            if (done) return;

            Vector3 playerPos = Bootstrap.Player.transform.WorldPosition;
            Vector3 currentPos = this.GetComponent<Transform>().Position;

            // Always cache latest position while tracking
            cachedPosition = currentPos;

            this.GetComponent<Transform>().Position = new Vector3(playerPos.x, currentPos.y, playerPos.z);

            timer += dt;
            if (timer >= trackDuration)
            {
                done = true;
                // Prefab drives its own destruction from here
            }
        }

        public override void OnEntityDestroy(uint id)
        {
            string laserPath = "Prefabs/" + laserPrefabName + ".prefab";
            GameObject laser = CreateGameObject(laserPath);
            laser.GetComponent<Transform>().Position = cachedPosition;
        }
    }
}