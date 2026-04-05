using SliceEngine;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.Remoting.Metadata.W3cXsd2001;

namespace SliceEngine
{
    public class LandmineMechExplosion : SliceBehaviour
    {
        public string fx_explosion = "FX_LandmineExplosion";

        bool triggered = false;
        bool hasExploded = false;

        float timer = 0.0f;

        public Vector3 jumpVelocity = new Vector3(0.0f, 5.0f, 0.0f);
        public float jumpTime = 0.15f;
        public float lingerTime = 0.1f;

        public int damage = 5;

        ColliderShape cs;

        Renderer r;
        Vector4 startColour = new Vector4(0.25f, 0.0f, 0.0f, 1.0f);
        Vector4 endColour = new Vector4(1.0f, 0.0f, 0.0f, 1.0f);
        float colTimer = 0.0f;
        public float colLerpTime = 0.5f;
        public void Triggered()
        {
            triggered = true;
        }

        public override void OnCreate()
        {
            base.OnCreate();
            cs = GetComponent<ColliderShape>();
            cs.ComponentEnabled = false;
            r = GetComponent<Renderer>();
        }

        public override void OnFixedUpdate(float dt)
        {
            base.OnFixedUpdate(dt);
            if (triggered)
            {
                if (timer < jumpTime)
                {
                    timer += dt;
                    transform.Translate(jumpVelocity * dt);
                }
                else if (!hasExploded)
                {
                    hasExploded = true;

                    CreateExplosionFX(transform.WorldPosition, Vector3.Zero);
                    cs.ComponentEnabled = true;
                    StartCoroutine(Suicide());
                }
            }
            else
            {
                //colTimer += dt;

                //// Smooth oscillation between 0 and 1
                //float colorT = (float)Math.Sin(colTimer * (Math.PI / colLerpTime)) * 0.5f + 0.5f;

                //Vector4 currentColor = Vector4.Lerp(startColour, endColour, colorT);
                //r.SetColor(currentColor);
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