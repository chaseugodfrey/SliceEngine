using SliceEngine;
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Security.Permissions;


namespace SliceEngine
{
    public class Boss : SliceBehaviour
    {
        private BossHealth health;
        private PhaseController phaseController;
        private AttackSetManager attackSetManager;

        public float bossHealth = 10f;
        public List<float> healthPhasesList = new List<float>();

        //Attack Patterns
        public List<int> attackPatterns1 = new List<int>();

        public List<int> attackPatterns2 = new List<int>();

        public List<int> attackPatterns3 = new List<int>();

        public override void OnCreate()
        {
            health = new BossHealth(bossHealth);
            phaseController = new PhaseController(healthPhasesList);
            phaseController.PhaseTrigger += HandlePhaseChange;

            List<AttackPatterns> temp = initializeAttackPatterns();

            attackSetManager = new AttackSetManager(temp, healthPhasesList.Count);
        }
        public override void OnUpdate(float dt)
        {
            phaseController.UpdatePhases(health.PercentageHealth());
            //if health below zero play death animation then delete entity?


        }

        public void HandlePhaseChange(int phase)
        {
            //SliceLog.Log("Phase changed to " + phase);
            //uh ill think of how to do the attack manager might not need it
        }

        public List<AttackPatterns> initializeAttackPatterns()
        {
            List<AttackPatterns> temp = new List<AttackPatterns>();

            if(!(attackPatterns1.Count == 0)) // not equal zero, just in case yall never see
            {
                temp.Add(new AttackPatterns(attackPatterns1));
            }
            if (!(attackPatterns2.Count == 0))
            {
                temp.Add(new AttackPatterns(attackPatterns2));
            }
            if (!(attackPatterns3.Count == 0))
            {
                temp.Add(new AttackPatterns(attackPatterns3));
            }

            if (temp.Count == 0)
            {
               SliceLog.Log("No attack patterns assigned to boss");
            }

            return temp;
        }
    }
}