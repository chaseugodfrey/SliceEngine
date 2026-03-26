using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class Level3ProjectileSpawner : Projectile_Spawner
    {
        #region States
        public class IntroState : BaseState
        {
            Level3ProjectileSpawner projectileSpawner;

            public IntroState(GameObject owner) : base(owner)
            {
                projectileSpawner = owner.As<Level3ProjectileSpawner>();
            }

            public override void OnEnter()
            {
                projectileSpawner.projectileSM.ChangeState(projectileSpawner.attackState);
            }

            public override void OnUpdate(float dt)
            {
                
            }

            public override void OnFixedUpdate(float dt)
            {
                
            }

            public override void OnExit()
            {
                
            }
        }

        public class AttackState : BaseState
        {
            Level3ProjectileSpawner projectileSpawner;

            public AttackState(GameObject owner) : base(owner)
            {
                projectileSpawner = owner.As<Level3ProjectileSpawner>();
            }

            public override void OnEnter()
            {

            }

            public override void OnUpdate(float dt)
            {

            }

            public override void OnFixedUpdate(float dt)
            {
                projectileSpawner.ShootUpdate(dt);
            }

            public override void OnExit()
            {

            }
        }
        #endregion

        public IntroState introState;
        public AttackState attackState;
        public StateMachine projectileSM;

        public override void OnCreate()
        {
            introState = new IntroState(this.gameObject);
            attackState = new AttackState(this.gameObject);
            projectileSM = new StateMachine();
            Console.WriteLine("LEvel 3 projectile On create");

            base.OnCreate();

            // for now just make it behave the same as the other projectile spawner
            projectileSM.ChangeState(introState);
        }

        public override void OnUpdate(float dt)
        {
            projectileSM.OnUpdate(dt);
        }

        public override void OnFixedUpdate(float dt)
        {
            projectileSM.OnFixedUpdate(dt);
        }

        public void ShootUpdate(float dt)
        {
            if (!active)
            {
                return;
            }

            count += dt;

            currentStyle = (SpawnStyle)spawnStyle;



            switch (currentStyle)
            {
                case SpawnStyle.Spiral:

                    this.transform.Rotate((360f / spiralRate) * dt, spiralAxis);

                    if (count >= 1f / projPerSecond)
                    {
                        count -= 1f / projPerSecond;


                        SpawnInBurstCheck();
                    }


                    break;
                case SpawnStyle.Aim:

                    if ((this.transform.WorldPosition - Bootstrap.Player.transform.WorldPosition).Magnitude() > rangeLimit)
                    {
                        if (preAimObject.Has<AlphaWiggleAnimation>())
                        {
                            //SliceLog.Log("passed the check on preaim");
                            AlphaWiggleAnimation a = preAimObject.As<AlphaWiggleAnimation>();

                            a.active = false;
                        }

                        if (HasComponent<AudioSource>())
                        {
                            GetComponent<AudioSource>().Play();
                        }

                        break;
                    }


                    this.transform.LookAt(Bootstrap.Player.transform.Position + new Vector3(0, aimVerticalOffset, 0), new Vector3(0, 1, 0));

                    float calc = 1f / projPerSecond;

                    // Find the charging up time
                    if (preaiming == false && count >= (calc * (1f - preAimPercentage)))
                    {
                        preaiming = true;
                        //start Preaiming
                        if (preAimObject.Has<AlphaWiggleAnimation>())
                        {
                            SliceLog.Log("passed the check on preaim");
                            AlphaWiggleAnimation a = preAimObject.As<AlphaWiggleAnimation>();

                            a.active = true;
                            a.rate = preAimFlickerRate;
                            a.MinWiggle = preaimMinAlpha;
                            a.MaxWiggle = preaimMaxAlpha;
                            a.random = preAimRandom;
                        }
                        //flicker
                    }


                    if (count >= 1f / projPerSecond)
                    {
                        count -= 1f / projPerSecond;

                        if (preAimObject.Has<AlphaWiggleAnimation>())
                        {
                            SliceLog.Log("passed the check on reset");
                            AlphaWiggleAnimation a = preAimObject.As<AlphaWiggleAnimation>();

                            preaiming = false;

                            a.Reset();
                        }

                        SpawnInBurstCheck();
                    }

                    break;
                case SpawnStyle.Straight:

                    if (count >= 1 / projPerSecond)
                    {
                        count -= 1 / projPerSecond;

                        SpawnInBurstCheck();
                    }

                    if (HasComponent<AudioSource>())
                    {
                        GetComponent<AudioSource>().Play();
                    }

                    break;
            }


        }
    }
}
