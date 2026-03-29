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

        public Transform target;
        public Vector3 destination;
        public Vector3 offset;
        public bool followTarget;
        public float followTightness = 0.0f;
        public float followRange = 200.0f;
        public float followOscillator = 0.0f;
        public float followOscillatorRate = 0.5f;
        public float followOscillatorTiming = 2.5f;
        public float followExternalModifier = 1.0f;

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

        public override void OnAwake()
        {
            base.OnAwake();
        }

        public override void OnUpdate(float dt)
        {

            if (followTarget)
            {
                followOscillator += followOscillatorRate * dt;
                if (followOscillator > followOscillatorTiming || followOscillator <= 0.0f)
                {
                    followOscillatorRate *= -1.0f;
                }

                float distance = Utilities.Distance3D(transform.Position, destination);
                float tightness = Utilities.Clamp(distance / followRange, 0.0f, 1.0f) - followTightness;
                followTightness += tightness * 0.5f;
                destination = target.WorldPosition + offset;

                Vector3 direction = (destination - transform.Position).Normalize();
                transform.Position = transform.Position + direction * followTightness * dt * 100f * followOscillator * followExternalModifier;
            }

            projectileSM.OnUpdate(dt);
        }

        public override void OnFixedUpdate(float dt)
        {
            projectileSM.OnFixedUpdate(dt);
        }

        public void SetTarget(Transform targetTransform = null, float speedModifier = 1.0f)
        {
            active = followTarget = targetTransform != null;
            target = targetTransform;
            followExternalModifier = speedModifier;
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

                    if (count >= 1f / projPerSecond)
                    {
                        count -= 1f / projPerSecond;

                        this.transform.LookAt(
                            Bootstrap.Player.transform.Position + new Vector3(0, aimVerticalOffset, 0),
                            new Vector3(0, 1, 0)
                        );

                        // hide the pre-aim line on fire
                        if (preAimObject != null && preAimObject.Has<AlphaWiggleAnimation>())
                        {
                            AlphaWiggleAnimation a = preAimObject.As<AlphaWiggleAnimation>();
                            preaiming = false;
                            a.Reset();
                        }

                        SpawnInBurstCheck();

                        if (HasComponent<AudioSource>())
                        {
                            GetComponent<AudioSource>().Play();
                        }
                    }
                    else if (!preaiming && count >= (1f / projPerSecond) * (1f - preAimPercentage))
                    {
                        // start showing the pre-aim line during charge-up
                        preaiming = true;
                        this.transform.LookAt(
                            Bootstrap.Player.transform.Position + new Vector3(0, aimVerticalOffset, 0),
                            new Vector3(0, 1, 0)
                        );

                        if (preAimObject != null && preAimObject.Has<AlphaWiggleAnimation>())
                        {
                            AlphaWiggleAnimation a = preAimObject.As<AlphaWiggleAnimation>();
                            a.active = true;
                            a.rate = preAimFlickerRate;
                            a.MinWiggle = preaimMinAlpha;
                            a.MaxWiggle = preaimMaxAlpha;
                            a.random = preAimRandom;
                        }
                    }

                    break;
            }
        }
    }
}
