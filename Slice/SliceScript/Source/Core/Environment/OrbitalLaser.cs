using SliceEngine;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.Remoting.Metadata.W3cXsd2001;

namespace SliceEngine
{
    public class OrbitalLaser : SliceBehaviour
    {
        public int damage = 20;
        //private bool hasHitPlayer = false;

        public float trackDuration = 4f;
        public float moveSpeed = 15f;
        //public string laserPrefabName = "FX_OrbitalLaser";

        private float tracktimer = 0f;
        private float damagetimer = 0f;
        private bool done = false;
        private Vector3 cachedPosition;

        ColliderShape cs;
        public float lingerTime = 2f;
        GameObject[] innerLaser;
        //public float damageDuration = 1.8f;

        public override void OnCreate()
        {
            base.OnCreate();
            cs = GetComponent<ColliderShape>();
            cs.ComponentEnabled = false;

            innerLaser = gameObject.GetAllChildren();
            foreach (GameObject child in innerLaser)
            {
                if (child.tag == "InnerLaser")
                {
                    child.GetComponent<Renderer>().ComponentEnabled = false;
                }
            }
        }

        public override void OnUpdate(float dt)
        {
            base.OnUpdate(dt);

            if (!done)
            {
                Tracking(dt);
            }
            else
            {
                StartCoroutine(Suicide());
            }

            //this.GetComponent<Transform>().Position = cachedPosition;


        }

        public void Tracking(float dt)
        {
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

            tracktimer += dt;
            if (tracktimer >= trackDuration)
            {
                done = true;
                cs.ComponentEnabled = true;

                foreach (GameObject child in innerLaser)
                {
                    if (child.tag == "InnerLaser")
                    {
                        child.GetComponent<Renderer>().ComponentEnabled = true;
                    }
                }
            }

            this.GetComponent<Transform>().Position = cachedPosition;
        }




        public void DamagePlayer(GameObject hit)
        {

            SliceLog.Log("Damage player called for Orbital lASER ALOY");

            if (hit.Has<PlayerController>() && hit.As<PlayerController>() == Bootstrap.Player)
            {
                SliceLog.Log("Player is hit");
                Bootstrap.Player.TakeDamage(damage, this.gameObject);


                AudioSettings.PlaySFX("PlayerHitLazer");

                // CreateGameObject("Prefabs/FX_Hit.prefab").GetComponent<Transform>().Position = transform.Position;
            }
        }

        public override void OnTriggerEnter(uint other)
        {
            SliceLog.Log("TRIGGER COLLIDE Orbital lASER ALOY");
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