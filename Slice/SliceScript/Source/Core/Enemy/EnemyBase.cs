using SliceEngine;
using System;
using System.Security.Permissions;


namespace SliceEngine
{
    public class EnemyBase : Entity
    {
        #region Enemy Fields
        public Transform enemyT { get { return this.GetComponent<Transform>(); } protected set{; } }
        protected RigidBody rb;
        protected NavAgent navAgent;

        float pathUpdateTimer = 0.0f;
        float pathUpdateInterval = 0.2f;

        GameObject targetObjRef;

        public bool active = false;
        private bool isDead = false;

        private EnemyState state;

        private bool chasingTarget = true;

        public void ChangeState(EnemyState newState)
        {
            state = newState;
        }

        #endregion


        public PlayerController playerT { get { return Bootstrap.Player; } protected set { ; } }

        #region Slice Behavior Overrides
        public override void OnCreate()
        {
            //Console.WriteLine("On create enemy base");
            
            base.OnCreate();
            enemyT = GetComponent<Transform>();
            rb = GetComponent<RigidBody>();
            navAgent = GetComponent<NavAgent>();
            navAgent.Speed = this.movementSpeed;
            StartNav();
        }
        public override void OnUpdate(float dt)
        {
            base.OnUpdate(dt);
            if (active)
            {

                if (state != null)
                {
                    state.DoEnemyAction(dt);
                }


                if (navAgent != null && chasingTarget)
                {
                    pathUpdateTimer += dt;

                    if (pathUpdateTimer > pathUpdateInterval)
                    {
                        pathUpdateTimer = 0.0f;
                        UpdateNavAgentTarget();
                    }
                }
            }
        }

        public override void OnFixedUpdate(float dt)
        {
            base.OnFixedUpdate(dt);
            if (active && state != null)
            {
                state.DoEnemyActionFixed();
            }
        }

        #endregion

        #region Creation and Set Up
        public virtual void SetUp()
        {
            //SliceLog.Log("Base Setup Called");
            //REMEMBER TO REMOVE THIS 
            enemyT = GetComponent<Transform>();
            rb = GetComponent<RigidBody>();

            //SliceLog.Log("Call after Rigibody");

            navAgent = GetComponent<NavAgent>();

            //SliceLog.Log("Call after NavAgent");

            if (navAgent == null)
            {
                //SliceLog.Log("Navgent is empty actually");
            }

            navAgent.Speed = this.movementSpeed;

            //SliceLog.Log("Call after MovementSpeed");
            targetObjRef = Bootstrap.Player.gameObject;

            //SliceLog.Log("Call after Player");
            //



            active = true; }

        public virtual void Reset()
        { active = false; state = null; }
        #endregion

        #region Navmesh

        public void StartNav()
        {
            //SliceLog.Log("Navmesh is starting");
            navAgent.ComponentState(true);
            //navAgent.enabled = true;
            if (navAgent == null)
            {
                //SliceLog.Log("NavAgentEmpty");
            }
        }

        public void StopNav()
        {
            navAgent.ComponentState(false);
            //navAgent.enabled = false;
        }

        public void UpdateNavAgentTarget()
        {
            GameObject activeTarget = targetObjRef != null ? targetObjRef : Bootstrap.Player.gameObject;

            navAgent.SetDestination(activeTarget.GetComponent<Transform>().Position);
        }

        public void ChangeActiveTarget(GameObject newTarget)
        {
            targetObjRef = newTarget;
        }

        public void SetDestinationToVector(Vector3 input)
        {
            chasingTarget = false;
            navAgent.SetDestination(input);
        }

        public void ResetDestinationToActiveTarget()
        {
            chasingTarget = true;
        }

        public void UpdateNavAgentSpeed(float input)
        {
            navAgent.Speed = input;
        }
        #endregion  

        #region Entity Overrides
        public override void OnDeath()
        {
            if (!isDead)
            {
                isDead = true;
                this.gameObject.Destroy();
            }
        }

        protected override void OnDamaged(GameObject source)
        {

        }

        protected override void OnHeal()
        {

        }
        #endregion
    }
}