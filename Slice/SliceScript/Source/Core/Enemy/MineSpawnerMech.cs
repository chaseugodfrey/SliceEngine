using SliceEngine;
using System;
using System.Collections.Generic;
using System.IO;

namespace SliceEngine
{
    public class MineSpawnerMech : EnemyBase
    {
        public void CreateDamagedFX(Vector3 position, Vector3 rotation)
        {
            GameObject fx = CreateGameObject("Prefabs/FX_TurretMechDamaged.prefab");
            fx.GetComponent<Transform>().Position = position;

            Transform t = fx.GetComponent<Transform>();
            t.Position = position;
            t.Rotation = rotation;
        }

        public void CreateDeathFX(Vector3 position, Vector3 rotation)
        {
            GameObject fx = CreateGameObject("Prefabs/FX_TurretMechDestroyed.prefab");
            fx.GetComponent<Transform>().Position = position;

            Transform t = fx.GetComponent<Transform>();
            t.Position = position;
            t.Rotation = rotation;
        }

        public override void TakeDamage(int amount, GameObject source = null)
        {
            base.TakeDamage(amount, source);
        }

        protected override void OnDamaged(GameObject source)
        {
            CreateDamagedFX(transform.WorldPosition, Vector3.Zero);
            base.OnDamaged(source);
        }

        public override void OnDeath()
        {
            CreateDeathFX(transform.WorldPosition, Vector3.Zero);
            base.OnDeath();
        }
    }
}