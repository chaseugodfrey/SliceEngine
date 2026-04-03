using SliceEngine;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.Remoting.Metadata.W3cXsd2001;

namespace SliceEngine
{
    public class AirmineMechExplosion : SliceBehaviour
    {
        public string fx_explosion = "FX_LandmineExplosion";

        bool triggered = false;
        bool hasExploded = false;

        float timer = 0.0f;

        public float lingerTime = 0.1f;

        public int damage = 5;

        ColliderShape cs;

        public void Triggered()
        {
            triggered = true;
        }

        public override void OnCreate()
        {
            base.OnCreate();
            cs = GetComponent<ColliderShape>();
            cs.ComponentEnabled = false;
        }

        public override void OnFixedUpdate(float dt)
        {
            base.OnFixedUpdate(dt);

            if (triggered)
            {
                if (!hasExploded)
                {
                    hasExploded = true;

                    CreateExplosionFX(transform.WorldPosition, Vector3.Zero);
                    cs.ComponentEnabled = true;
                    StartCoroutine(Suicide());
                }
            }
        }

        public void CreateExplosionFX(Vector3 position, Vector3 rotation)
        {
            string prefabPath = "Prefabs/" + fx_explosion + ".prefab";
            GameObject fx = CreateGameObject(prefabPath);

            Transform t = fx.GetComponent<Transform>();
            t.Position = position;
            t.Rotation = rotation;
        }

        public void DamagePlayer(GameObject hit)
        {

            if (hit.Has<PlayerController>() && hit.As<PlayerController>() == Bootstrap.Player)
            {
                Bootstrap.Player.TakeDamage(damage, this.gameObject);
                AudioSettings.PlaySFX("PlayerHitLazer");
            }
        }

        public override void OnTriggerEnter(uint other)
        {
            GameObject collidedGO = FindGameObjectWithID(other);
            if (collidedGO != null && collidedGO.tag == "Player")
            {
                DamagePlayer(collidedGO);
            }
        }

        IEnumerator Suicide()
        {
            yield return new WaitForSeconds(lingerTime);
            gameObject.Destroy();
        }
    }
}