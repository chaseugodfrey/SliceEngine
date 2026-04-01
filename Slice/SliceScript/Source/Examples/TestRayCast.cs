using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Security.Principal;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class TestRayCast : SliceBehaviour
    {
        public float magnitude = 20.0f;
        public Vector3 direction = new Vector3(-1.0f, 0.0f, 0.0f);
        private String prefabPath = "Prefabs/FX_TheBallDamaged.prefab";

        private bool isSpawning = false;

        Transform t;
        //  SliceBehaviour go;

        public override void OnCreate()
        {
            t = GetComponent<Transform>();
        }

        public override void OnUpdate(float dt)
        {
            t = GetComponent<Transform>();
            Physics.DebugDrawRay(t.Position, direction, magnitude);
            Vector3 totalDirection = direction.Normalize() * magnitude;
            if (Physics.RayCast(t.Position, totalDirection) && !isSpawning)
            {
                StartCoroutine(SpawnParticle());
            }
        }

        private IEnumerator SpawnParticle()
        {
            isSpawning = true;

            GameObject hint = CreateGameObject(prefabPath);
            hint.GetComponent<Transform>().Position = t.WorldPosition + new Vector3(0, 3, 0);

            yield return new WaitForSeconds(1.0f); 

            isSpawning = false;
        }

    }
}
