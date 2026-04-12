using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class ProjectileSpawnParent : SliceBehaviour
    {
        public GameObject playerObj;
        public GameObject bossObj;
        Transform playerTransform;
        Transform bossTransform;

        public List<GameObject> projectileShooters;

        // variables
        public float revolveSpeed;

        public override void OnAwake()
        {
            bossTransform = bossObj.GetComponent<Transform>();
            var list = gameObject.GetAllChildren();
            SliceLog.Console(list.Length);
            foreach (var child in list)
            {
                projectileShooters.Add(child);
            }
        }

        public override void OnUpdate(float dt)
        {
            Follow();
            Orbit(dt);
        }

        void Follow()
        {
            transform.Position = bossTransform.Position;
        }

        void Orbit(float dt)
        {
            transform.Rotate(Vector3.Up * revolveSpeed * dt);
            foreach (var shooter in projectileShooters)
            {
                var shooterTr = shooter.GetComponent<Transform>();
                shooterTr.LookAt(playerTransform.Position, Vector3.Up);
                SliceLog.Console("updating tr");

            }
        }
    }
}
