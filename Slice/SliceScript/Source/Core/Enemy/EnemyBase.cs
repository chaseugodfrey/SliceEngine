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
        //protected NavAgent navAgent;

        float pathUpdateTimer = 0.0f;
        float pathUpdateInterval = 0.2f;

        GameObject targetObjRef;

        public bool active = false;
        private bool isDead = false;

        private EnemyState state;

        private bool chasingTarget = true;

        protected bool shield = false;

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
            //SliceLog.Log("Call after MovementSpeed");
            targetObjRef = Bootstrap.Player.gameObject;

            //SliceLog.Log("Call after Player");
            //



            active = true; }

        public virtual void Reset()
        { active = false; state = null; }
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