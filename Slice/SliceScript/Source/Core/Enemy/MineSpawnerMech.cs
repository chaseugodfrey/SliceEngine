using SliceEngine;
using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;

namespace SliceEngine
{
    public class MineSpawnerMech : EnemyBase
    {
        // Hit shake settings
        public float shakeDuration = 0.2f;
        public float shakeMagnitude = 0.12f;
        public float dipMagnitude = 0.1f;

        bool isShaking = false;
        Vector3 shakeOrigin;
        bool spawnerDestroyed = false;

        public void CreateDamagedFX(Vector3 position, Vector3 rotation)
        {
            GameObject fx = CreateGameObject("Prefabs/FX_TurretMechDamaged.prefab");
            Transform t = fx.GetComponent<Transform>();
            t.Position = position;
            t.Rotation = rotation;
        }

        public void CreateDeathFX(Vector3 position, Vector3 rotation)
        {
            GameObject fx = CreateGameObject("Prefabs/FX_TurretMechDestroyed.prefab");
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
            StartCoroutine(HitShake());
            base.OnDamaged(source);
        }

        public override void OnDeath()
        {
            spawnerDestroyed = true;
            CreateDeathFX(transform.WorldPosition, Vector3.Zero);
            base.OnDeath();
        }

        IEnumerator HitShake()
        {
            if (spawnerDestroyed) yield break;

            if (!isShaking)
            {
                shakeOrigin = transform.Position;
                isShaking = true;
            }

            float elapsed = 0f;
            while (elapsed < shakeDuration && !spawnerDestroyed)
            {
                float x = SliceRandom.RangeFloat(-1f, 1f) * shakeMagnitude;
                float z = SliceRandom.RangeFloat(-1f, 1f) * shakeMagnitude;

                float t = elapsed / shakeDuration;
                float y = t < 0.5f
                    ? -dipMagnitude * (t / 0.5f)
                    : -dipMagnitude * (1f - (t - 0.5f) / 0.5f);

                transform.Position = shakeOrigin + new Vector3(x, y, z);
                elapsed += Time.deltaTime;
                yield return null;
            }

            if (!spawnerDestroyed)
                transform.Position = shakeOrigin;
            isShaking = false;
        }
    }
}