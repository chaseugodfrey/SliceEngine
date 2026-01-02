using SliceEngine;
using System;


namespace SliceEngine
{
    public class EnemyBase : Entity
    {
        #region Enemy Fields
        public Transform enemyT { get; protected set; }
        protected RigidBody rb;
        public bool active = false;
        private bool isDead = false;

        private EnemyState state;

        public void ChangeState(EnemyState newState)
        {
            newState.SetUp(this);
            state = newState;
        }

        #endregion


        public Transform playerT { get; protected set; } = null;

        #region Slice Behavior Overrides
        public override void OnCreate()
        {
            base.OnCreate();
            enemyT = GetComponent<Transform>();
            rb = GetComponent<RigidBody>();
        }
        public override void OnUpdate(float dt)
        {
            base.OnUpdate(dt);
            if (active && state != null)
            {
                state.DoEnemyAction(dt);
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
        { active = true; playerT = Bootstrap.Player.transform; }

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