using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using static SliceEngine.Cursor;

namespace SliceEngine
{
    //--- Chase State (Default)--- 
    public class EnemySlimeChaseState: EnemyState
    {
        private float _movementSpeed;
        private float _attackTriggerRange;

        public EnemySlimeChaseState(float movementSpeedInput, float attackTriggerInput)
        {
            _movementSpeed = movementSpeedInput;
            _attackTriggerRange = attackTriggerInput;
        }

        public override void DoEnemyAction(float dt)
        {
            Vector3 direction_diff = enemyOwner.playerT.Position - enemyOwner.enemyT.Position;



            enemyOwner.enemyT.Position += direction_diff.Normalize() * _movementSpeed * dt;

            if (direction_diff.Magnitude() <= _attackTriggerRange)
            {
                enemyOwner.ChangeState(new EnemySlimeAttackState());
                //attack state
            }
        }

        public override void DoEnemyActionFixed()
        {   }

        public override void ReachTargetAction()
        {   }

        public override void OnLanding()
        {   }

        public override void OnCollide()
        {   }

        public override void OnDamaged()
        {   }
    }

    //--- Attack State ---
    public class EnemySlimeAttackState : EnemyState
    {

        public EnemySlimeAttackState()
        {
        }

        public override void DoEnemyAction(float dt)
        {
            if (!enemyOwner.As<EnemySlime>().attacking)
            {
                enemyOwner.As<EnemySlime>().Attack();
            }
        }
        

        public override void DoEnemyActionFixed()
        { }

        public override void ReachTargetAction()
        { }

        public override void OnLanding()
        { }

        public override void OnCollide()
        { }

        public override void OnDamaged()
        { }
    }

}
