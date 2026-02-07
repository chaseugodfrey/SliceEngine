using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class EnemySlime : EnemyBase
    {
        public bool stunned = false;

        public float horKnockback = 1f;
        public float vertKnockback = 1f;

        public float attackTriggerRange = 1f;
        public float attackDamageRange = 1f;
        public float buildUpTiming = 1f;
        public float flickerTiming = 1f;
        public bool exploding { get; private set; } = false;
        //private float _explodingCounter = 0f;

        //public GameObject explodeParentObject;
        public GameObject explodeMeshObject;
        public GameObject explosionHitBoxObject;
        private GeneralHitbox explosionHitBox;
        public GameObject meshRenderer;

        //Function called when you want the enemy to be active
        public override void SetUp()
        {
            //Safety net return;
            if (active) return;


            base.SetUp(); 
            //Console.WriteLine("Slime setup called");
            if(explosionHitBoxObject.Has<GeneralHitbox>())
            {
                explosionHitBox = explosionHitBoxObject.As<GeneralHitbox>();
                explosionHitBox.HitBoxListeners += BasicExplode;
                //_basicHitBox.SetActive(false);
                //basicHitBox.GetComponent<ColliderShape>().ComponentEnabled = false;

                explosionHitBox.TurnOff();
                explodeMeshObject.SetActive(false);

                if(explosionHitBoxObject.GetComponent<ColliderShape>().ComponentEnabled == false)
                {
                    //Console.WriteLine("Hit Box successfully turned off");
                    //SliceLog.Log("Hit Box successfully turned off");
                }
                else
                {
                    //Console.WriteLine("Hit Box still on");
                    //SliceLog.Log("Hit Box still on");
                }
            }
            else
            {
                //Console.WriteLine("Slime has no hitbox");
            }

            this.ChangeState(new EnemySlimeChaseState(this));
        }
        public override void OnCreate()
        {
            base.OnCreate();
        }

        public void Reset()
        {   active = false; }

        public override void OnUpdate(float dt)
        {
            /*
            if (Input.IsKeyDown(Keys.KEY_P) && active == false)
            {
                Console.WriteLine("PPPPressed"); SetUp();
            }
            */
            base.OnUpdate(dt);
        }

        public void BasicExplode(GameObject hit)
        {
            //Console.Write("| Basic Attack called |");

            if ( hit.Has<PlayerController>()  && hit.As<PlayerController>() == Bootstrap.Player)
            {
                //isPlayerInBasic = true;
                //RE INSERT ONCE ENABLE IS WORKING
                Bootstrap.Player.TakeDamage(damage, this.gameObject);
                this.TakeDamage(1000, this.gameObject );
            }
            else
            {
                //Console.Write("| Failed player check on damage, no damage done |");
            }
        }

        public void StartExplodeCoroutine()
        {
            StartCoroutine(ExplodeCoroutine());
        }

        IEnumerator ExplodeCoroutine()
        {
            //Console.Write("exploding is On -> ");
            exploding = true;

            //Console.Write("Building Up-> ");
            float buildupCount = 0f;


            Transform meshRenderT = meshRenderer.GetComponent<Transform>();


            while (buildupCount <= buildUpTiming)
            {
                buildupCount += Time.deltaTime;

                float newScale = 1f + ( (buildupCount / buildUpTiming) * 1f);

                meshRenderT.Scale = new Vector3(newScale, newScale, newScale);

                yield return new WaitForSeconds(Time.deltaTime);
            }
            //Console.Write("Build Up Done -> ");

            // COMMENTING THIS OUT UNTIL ENABLE/DISABLE IS WORKING
            //_basicHitBox.SetActive(true);
            //basicHitBox.GetComponent<ColliderShape>().ComponentEnabled = true;
            explosionHitBox.TurnOn();
            explodeMeshObject.SetActive(true);
            //Console.Write("Box On | ");


            if (explosionHitBoxObject.GetComponent<ColliderShape>().ComponentEnabled == true)
            {
                //Console.Write("Hit Box successfully turned on -> ");
                //SliceLog.Log("Hit Box successfully turned off");
            }
            else
            {
                ////Console.WriteLine("Hit Box still off -> ");
                //SliceLog.Log("Hit Box still on");
            }


            //Console.Write("Flicker waiting -> ");
            yield return new WaitForSeconds(flickerTiming);
            //Console.Write("Flicker returned -> ");


            //_basicHitBox.As<GeneralHitbox>().SetActive(false);
            //basicHitBox.GetComponent<ColliderShape>().ComponentEnabled = false;
            explosionHitBox.TurnOff();
            explodeMeshObject.SetActive(false);
            //Console.Write("Box Off | ");


            if (explosionHitBoxObject.GetComponent<ColliderShape>().ComponentEnabled == false)
            {
                //Console.WriteLine("Hit Box successfully turned off -> ");
                //SliceLog.Log("Hit Box successfully turned off");
            }
            else
            {
                //Console.WriteLine("Hit Box still on -> ");
                //SliceLog.Log("Hit Box still on");
            }

            exploding = false;
            //Console.WriteLine("exploding is Off");

            //ChangeState(new EnemySlimeChaseState(movementSpeed, attackTriggerRange));

            yield break;
        }


        public override void OnCollideEnter(uint other)
        {
            base.OnCollideEnter(other);
        }

        public override void OnCollideStay(uint other)
        {
            base.OnCollideStay(other);
        }

        public override void TakeDamage(int amount, GameObject source = null)
        {
            // This override is just to insert a debug
            //Console.WriteLine("Enemy is taking damage");
            SliceLog.Console("Enemy is taking damage");
            base.TakeDamage(amount, source);

        }



        protected override void OnHeal() { }
        protected override void OnDamaged(GameObject source)
        {
            rb.AddForce(new Vector3(0, vertKnockback, horKnockback), ForceMode.Impulse);
            CreateGameObject("Prefabs/Bloodsplatter.prefab").GetComponent<Transform>().Position = transform.Position;
            SliceLog.Console("ENEMY IS BEING HIT");
        }

        
        public override void OnDeath()
        {
            this.gameObject.Destroy();   
        }

    }
}
