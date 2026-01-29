using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using static SliceEngine.Cursor;

namespace SliceEngine
{
    /// <summary>
    /// Intended action. Will chase the target .
    /// </summary>

    //--- Base Grunt State ---
    public class EnemyGruntState : EnemyState
    {
        protected EnemyGrunt enemyOwner;
        public EnemyGruntState(EnemyGrunt owner) { this.enemyOwner = owner; /*SliceLog.Log("Grunt chase state created");*/ Console.WriteLine("Grunt chase state created"); }
        public override void DoEnemyAction(float dt) {}
        public override void DoEnemyActionFixed() {}
        public override void OnCollide() {}
        public override void OnDamaged() {}
        public override void OnLanding() {}
        public override void ReachTargetAction() {}
    }

    //--- Chase State (Default)--- 
    public class EnemyGruntChaseState: EnemyGruntState
    {
        public EnemyGruntChaseState(EnemyGrunt owner) : base(owner)
        {
            
        }

        public override void DoEnemyAction(float dt)
        {
            base.DoEnemyAction(dt);

            Vector3 direction_diff = enemyOwner.playerT.GetComponent<Transform>().Position - enemyOwner.enemyT.Position;

            enemyOwner.enemyT.Position += direction_diff.Normalize() * enemyOwner.movementSpeed * dt;

            enemyOwner.enemyT.Rotation = direction_diff;

            if (direction_diff.Magnitude() <= enemyOwner.strafeDistance)
            {
                enemyOwner.ChangeState(new EnemyGruntStrafeState(enemyOwner));
                //strafe state
            }
        }
    }

    //--- Strafe State ---
    public class EnemyGruntStrafeState : EnemyGruntState
    {
        public EnemyGruntStrafeState(EnemyGrunt owner) : base(owner) { }


        public override void DoEnemyAction(float dt)
        {
            base.DoEnemyAction(dt);

            Vector3 direction_diff = enemyOwner.playerT.GetComponent<Transform>().Position - enemyOwner.enemyT.Position;
            enemyOwner.enemyT.Rotation = direction_diff;

            //float distFromPlayer = direction_diff.Magnitude(); 

            //if (direction_diff.Magnitude() <= (enemyOwner.strafeDistance + enemyOwner.strafeTolerance) && direction_diff.Magnitude() >= (enemyOwner.strafeDistance - enemyOwner.strafeTolerance))
            SliceLog.Log("" + direction_diff.Magnitude());
            if (direction_diff.Magnitude() > (enemyOwner.strafeDistance + enemyOwner.strafeTolerance))
            {
                enemyOwner.ChangeState(new EnemyGruntChaseState(enemyOwner));
            }
            else if (direction_diff.Magnitude() < enemyOwner.strafeDistance)
            {
                enemyOwner.enemyT.Position -= direction_diff.Normalize() * enemyOwner.strafeSpeed * dt;
            }
            else
            {
                enemyOwner.enemyT.Position += direction_diff.Normalize() * enemyOwner.strafeSpeed * dt;
            }



        }
    }


    //--- Attack State ---
    public class EnemyGruntAttackState : EnemyGruntState
    {
        public EnemyGruntAttackState(EnemyGrunt owner) : base(owner) {}

        public override void DoEnemyAction(float dt)
        {
            base.DoEnemyAction(dt);
            Vector3 direction_diff = enemyOwner.playerT.GetComponent<Transform>().Position - enemyOwner.enemyT.Position;


            if (!enemyOwner.As<EnemyGrunt>().attacking)
            {
                if (direction_diff.Magnitude() < enemyOwner.attackTriggerRange)
                {

                    enemyOwner.As<EnemyGrunt>().StartAttackCoroutine();
                }
                else
                {
                    enemyOwner.ChangeState(new EnemyGruntChaseState(enemyOwner));
                }
            }
            
        }
    }


    //--- Stunned State ---
    public class EnemyGruntStunState : EnemyGruntState
    {
        public EnemyGruntStunState(EnemyGrunt owner) : base(owner) { }

    }

}
