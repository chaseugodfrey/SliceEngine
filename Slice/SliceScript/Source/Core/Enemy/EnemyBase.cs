using SliceEngine;
using System;
using System.Security.Permissions;


namespace SliceEngine
{
    public class EnemyBase : Entity
    {
        #region Enemy Fields
        public Transform enemyT { get; protected set; }
        protected RigidBody rb;
        protected NavAgent navAgent;

        float pathUpdateTimer = 0.0f;
        float pathUpdateInterval = 0.2f;

        GameObject targetObjRef;

        public bool active = false;
        private bool isDead = false;

        private EnemyState state;

        public void ChangeState(EnemyState newState)
        {
            state = newState;
        }

        #endregion


        public PlayerController playerT { get { return Bootstrap.Player; } protected set { ; } }

        #region Slice Behavior Overrides
        public override void OnCreate()
        {
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


                if (navAgent != null)
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
            SliceLog.Log("Base Setup Called");
            //REMEMBER TO REMOVE THIS 
            enemyT = GetComponent<Transform>();
            rb = GetComponent<RigidBody>();

            navAgent = GetComponent<NavAgent>();
            navAgent.Speed = this.movementSpeed;
            targetObjRef = Bootstrap.Player.gameObject;
            //



            active = true; }

        public virtual void Reset()
        { active = false; state = null; }
        #endregion

        #region Navmesh

        public void StartNav()
        {
            SliceLog.Log("Navmesh is starting");
            navAgent.enabled = true;
            if (navAgent == null)
            {
                SliceLog.Log("NavAgentEmpty");
            }
        }

        public void StopNav()
        {
            navAgent.enabled = false;
        }

        public void UpdateNavAgentTarget()
        {
            GameObject activeTarget = targetObjRef != null ? targetObjRef : Bootstrap.Player.gameObject;

            navAgent.SetDestination(activeTarget.GetComponent<Transform>().Position);
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