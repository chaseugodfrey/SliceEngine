using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    /// <summary>
    /// Intended action. Will chase the target .
    /// </summary>

    //--- Base Slime State ---
    public class EnemySlimeState : EnemyState
    {
        protected EnemySlime enemyOwner;
        public EnemySlimeState(EnemySlime owner) { this.enemyOwner = owner; }/*SliceLog.Log("Slime chase state created");*/ //SliceLog.Log("Slime chase state created"); }
        public override void DoEnemyAction(float dt) {}
        public override void DoEnemyActionFixed() {}
        public override void OnCollide() {}
        public override void OnDamaged() {}
        public override void OnLanding() {}
        public override void ReachTargetAction() {}
    }

    //--- Chase State (Default)--- 
    public class EnemySlimeChaseState: EnemySlimeState
    {
        public EnemySlimeChaseState(EnemySlime owner) : base(owner)
        {
            owner.StartNav();
        }

        public override void DoEnemyAction(float dt)
        {
            base.DoEnemyAction(dt);

            
            Vector3 direction_diff = enemyOwner.playerT.GetComponent<Transform>().Position - enemyOwner.enemyT.Position;

            //enemyOwner.enemyT.Position += direction_diff.Normalize() * enemyOwner.movementSpeed * dt;

            if (direction_diff.Magnitude() <= enemyOwner.attackTriggerRange)
            {

               
                enemyOwner.ChangeState(new EnemySlimeExplodeState(enemyOwner));
                //attack state
            }
            
        }
    }

    //--- Explode State ---
    public class EnemySlimeExplodeState : EnemySlimeState
    {
        public EnemySlimeExplodeState(EnemySlime owner) : base(owner) { owner.StopNav(); }

        public override void DoEnemyAction(float dt)
        {
            base.DoEnemyAction(dt);
            Vector3 direction_diff = enemyOwner.playerT.GetComponent<Transform>().Position - enemyOwner.enemyT.Position;


            if (!enemyOwner.As<EnemySlime>().exploding)
            {
                if (direction_diff.Magnitude() < enemyOwner.attackTriggerRange)
                {

                    enemyOwner.As<EnemySlime>().StartExplodeCoroutine();
                }
                /*
                else
                {
                    enemyOwner.ChangeState(new EnemySlimeChaseState(enemyOwner));
                }
                */
            }
            
        }
    }


    //--- Stunned State ---
    public class EnemySlimeStunState : EnemySlimeState
    {
        public EnemySlimeStunState(EnemySlime owner) : base(owner) { }

    }

}
