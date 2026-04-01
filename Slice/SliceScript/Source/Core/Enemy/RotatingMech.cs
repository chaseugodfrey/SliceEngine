using SliceEngine;
using System;
using System.Collections.Generic;
using System.IO;

namespace SliceEngine
{
    public class RotatingMech : SliceBehaviour
    {
        public List<GameObject> activeLasers = new List<GameObject>();
        public string laserPrefabName = "RotatingMechLaser";
        public string firingFXPrefabName = "";
        public int numLasers = 1;

        public float rotateSpeed = 50.0f;

        public Vector3 axis = new Vector3(0, 1, 0);

        public void CreateLaser(Vector3 position, Vector3 rotation)
        {
            string prefabPath = "Prefabs/" + laserPrefabName + ".prefab";
            GameObject fx = CreateGameObject(prefabPath);            
            Transform t = fx.GetComponent<Transform>();
            t.Position = position;
            t.Rotation = rotation;

            fx.SetParent(gameObject);

            activeLasers.Add(fx);
        }

        public void CreateFiringFX(Vector3 position, Vector3 rotation)
        {
            if (firingFXPrefabName == "")
            {
                return;
            }
            string prefabPath = "Prefabs/" + firingFXPrefabName + ".prefab";
            GameObject fx = CreateGameObject(prefabPath);

            Transform t = fx.GetComponent<Transform>();
            t.Position = position;
            t.Rotation = rotation;
        }

        public override void OnCreate()
        {
            base.OnCreate();

            float angleStep = 360.0f / numLasers;

            for (int i = 0; i < numLasers; i++)
            {
                float angle = i * angleStep;

                CreateLaser(transform.WorldPosition, new Vector3(0, angle, 0));
            }
        }

        public override void OnFixedUpdate(float dt)
        {
            base.OnFixedUpdate(dt);
            transform.RotateAxisAngle(axis, rotateSpeed * dt);
        }
    }
}