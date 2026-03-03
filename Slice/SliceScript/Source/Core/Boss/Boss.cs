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

            //attackSetManager = new AttackSetManager();
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

        public void initializeAttackPatterns()
        {


        }
    }
}