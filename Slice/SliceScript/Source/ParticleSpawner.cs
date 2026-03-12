using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class ParticleSpawner : SliceBehaviour
    {
        public string projectilePrefabName; 
        public Vector3 spawnPosition = Vector3.Zero;

        public override void OnUpdate(float dt)
        {
            if (Input.IsKeyPressed(Keys.KEY_1))
            {
                SpawnParticle();
            }
        }
        public void SpawnParticle()
        {
            GameObject particle = CreateGameObject("Prefabs/" + projectilePrefabName + ".prefab");
            particle.GetComponent<Transform>().Position = 
                spawnPosition == Vector3.Zero? transform.Position : spawnPosition;
        }
    }
}