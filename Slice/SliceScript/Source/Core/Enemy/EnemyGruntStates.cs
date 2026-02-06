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
        public EnemyGruntState(EnemyGrunt owner) { this.enemyOwner = owner; }/*SliceLog.Log("Grunt chase state created");*/ //Console.WriteLine("Grunt chase state created"); }
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
            owner.UpdateNavAgentSpeed(owner.movementSpeed);
            owner.ResetDestinationToActiveTarget();
            owner.StartNav();
        }

        public override void DoEnemyAction(float dt)
        {
            base.DoEnemyAction(dt);

            Vector3 direction_diff = enemyOwner.playerT.GetComponent<Transform>().Position - enemyOwner.enemyT.Position;

            enemyOwner.transform.LookAt(enemyOwner.playerT.GetComponent<Transform>().Position, new Vector3(0,1,0));

            //enemyOwner.enemyT.Position += direction_diff.Normalize() * enemyOwner.movementSpeed * dt;

            //enemyOwner.enemyT.Rotation = direction_diff;

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

        private float atkCheckCounter = 0f;

        public EnemyGruntStrafeState(EnemyGrunt owner) : base(owner) 
        {
            owner.UpdateNavAgentSpeed(owner.strafeSpeed);
        }


        public override void DoEnemyAction(float dt)
        {
            base.DoEnemyAction(dt);

            //rotation
            enemyOwner.transform.LookAt(enemyOwner.playerT.GetComponent<Transform>().Position, new Vector3(0, 1, 0));

            
            Vector3 direction_diff = enemyOwner.playerT.GetComponent<Transform>().Position - enemyOwner.enemyT.Position;
            if (direction_diff.Magnitude() >= (enemyOwner.strafeDistance + enemyOwner.strafeTolerance))
            {
                enemyOwner.ChangeState(new EnemyGruntChaseState(enemyOwner));
            }
            else
            {

                Vector3 displacementVector = direction_diff.Normalize() * enemyOwner.strafeDistance; 

                enemyOwner.SetDestinationToVector(enemyOwner.playerT.GetComponent<Transform>().Position - displacementVector);
            }

            //old code beefore navmesh
            /*else if (direction_diff.Magnitude() < enemyOwner.strafeDistance)
            {
                enemyOwner.enemyT.Position -= direction_diff.Normalize() * enemyOwner.strafeSpeed * dt;
            }
            else
            {
                enemyOwner.enemyT.Position += direction_diff.Normalize() * enemyOwner.strafeSpeed * dt;
            }*/


            //Attack checking
            this.atkCheckCounter += dt;

            if (this.atkCheckCounter >= enemyOwner.attackOddsCheckFrequency)
            {
                
                this.atkCheckCounter = 0f;

                if (SliceRandom.ValueFloat() <= enemyOwner.attackOdds)
                {
                    enemyOwner.ChangeState(new EnemyGruntWindUpState(enemyOwner));
                }
            }





        }
    }


    //--- Attack State ---
    public class EnemyGruntAttackState : EnemyGruntState
    {
        private Vector3 positionToLook;

        public EnemyGruntAttackState(EnemyGrunt owner, Vector3 positionInput) : base(owner) 
        {
            owner.StartAttackCoroutine();
            positionToLook = positionInput;
        }

        public override void DoEnemyAction(float dt)
        {
            base.DoEnemyAction(dt);

            enemyOwner.transform.LookAt(positionToLook, new Vector3(0, 1, 0));
        }
    }

    //--- WindUp State ---
    public class EnemyGruntWindUpState: EnemyGruntState
    {
        public EnemyGruntWindUpState(EnemyGrunt owner) : base(owner)
        {
            owner.StopNav();
            owner.StartWindUp();
        }

        public override void DoEnemyAction(float dt)
        {
            base.DoEnemyAction(dt);

            enemyOwner.transform.LookAt(enemyOwner.playerT.GetComponent<Transform>().Position, new Vector3(0, 1, 0));

            if (enemyOwner.triggerAttack)
            {
                
                enemyOwner.ChangeState(new EnemyGruntAttackState(enemyOwner,enemyOwner.playerT.GetComponent<Transform>().Position));
            }
        }
    }

    //--- Stunned State ---
    public class EnemyGruntStunnedState: EnemyGruntState
    {
        public EnemyGruntStunnedState(EnemyGrunt owner) : base(owner)
        {

        }
    }
}
