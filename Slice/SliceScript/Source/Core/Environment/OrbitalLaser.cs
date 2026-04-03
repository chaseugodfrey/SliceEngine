using SliceEngine;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.Remoting.Metadata.W3cXsd2001;

namespace SliceEngine
{
    public class OrbitalLaser : SliceBehaviour
    {
        static int count = 0;
        public int damage = 20;
        //private bool hasHitPlayer = false;

        public float trackDuration = 4f;
        public float moveSpeed = 15f;
        //public string laserPrefabName = "FX_OrbitalLaser";

        private float tracktimer = 0f;
        private float tickTimer = 0f;
        private float tickRate = 0.25f;
        private bool done = false;
        private Vector3 cachedPosition;

        ColliderShape cs;
        public float lingerTime = 2f;
        GameObject[] innerLaser;
        GameObject signallingLaser;
        //public float damageDuration = 1.8f;
        Camera camera;

        bool isPlayerIn = false;
        float impactY;

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

                if (child.tag == "SignallingLaser")
                {
                    signallingLaser = child;
                }
            }
        }

        public override void OnAwake()
        {
            GameObject[] cameras = gameObject.FindGameObjectsWithTag("mainCam");
            if (cameras[0] != null)
                SliceLog.Console("Camera found for orbital laser");
            camera = cameras[0].GetComponent<Camera>();
            impactY = transform.Position.y * 3.0f;
        }

        public override void OnUpdate(float dt)
        {
            base.OnUpdate(dt);

            if (isPlayerIn)
            {
                if (tickTimer <= 0.0f)
                {
                    DamagePlayer(Bootstrap.Player.gameObject);
                    tickTimer = tickRate;
                }
                else
                {
                    tickTimer -= dt;
                }

                var pos = transform.Position;
                pos.y = impactY;
                camera.SetImpactFramePosition(pos);
                camera.SetImpactFrame(true);
            }

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
            if (hit.Has<PlayerController>() && hit.As<PlayerController>() == Bootstrap.Player)
            {
                Bootstrap.Player.TakeDamage(damage, this.gameObject);

                AudioSettings.PlaySFX("PlayerHitLazer");

                // CreateGameObject("Prefabs/FX_Hit.prefab").GetComponent<Transform>().Position = transform.Position;
            }
        }

        public override void OnTriggerEnter(uint other)
        {
            GameObject collidedGO = FindGameObjectWithID(other);
            if (collidedGO != null)
            {
                if (collidedGO.tag == "Player")
                    isPlayerIn = true;

                //if (collidedGO.tag == "OrbitalGround")
                //{
                //    Vector3 pos = collidedGO.GetComponent<Transform>().WorldPosition;
                //    signallingLaser.GetComponent<Transform>().Position = new Vector3(transform.Position.x, pos.y + 0.1f, transform.Position.z);
                //}
                    
            }
        }

        public override void OnTriggerExit(uint other)
        {
            ResetTickTimer();
            isPlayerIn = false;
        }

        IEnumerator Suicide()
        {
            yield return new WaitForSeconds(lingerTime);
            //camera.SetImpactFrame(false);
            gameObject.Destroy();
        }

        void ResetTickTimer()
        {
            tickTimer = 0;
        }

        public void SetupLaser(float diameter, float height, float tracktime, float lifetime, float trackSpeed = 15.0f)
        {
            var newScale = new Vector3(diameter, height, diameter);
            var ps = signallingLaser.GetComponent<ParticleSystem>();

            // parent
            transform.Scale = newScale;

            // script
            trackDuration = tracktime;
            lingerTime = lifetime;
            moveSpeed = trackSpeed;

            // particle system
            ps.Scale = newScale;
            ps.Lifetime = tracktime * 0.5f;
        }

        public override void OnEntityDestroy(uint id)
        {
            CreateParticleEnd();
            camera.SetImpactFrame(false);
        }

        void CreateParticleEnd()
        {
            SliceLog.Console(++count);
            GameObject go = gameObject.CreateGameObject("Prefabs/FX_OrbitalLaserEnd.prefab");
            Transform tr = go.GetComponent<Transform>();
            tr.transform.Position = transform.Position;

            GameObject[] children = go.GetAllChildren();
            var ps_inner = children[0].GetComponent<ParticleSystem>();
            ps_inner.Scale = transform.Scale;

            var ps_outer = children[1].GetComponent<ParticleSystem>();
            ps_outer.Scale = new Vector3(transform.Scale.x * 0.1f, transform.Scale.y * 0.05f, transform.Scale.z * 0.1f);
        }
    }
}