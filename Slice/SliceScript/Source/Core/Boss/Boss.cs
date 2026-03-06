using SliceEngine;
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Security.Permissions;


namespace SliceEngine
{
    public class Boss : SliceBehaviour
    {
        //Classes handling different responsibility
        private BossHealth health;
        private PhaseController phaseController;
        private AttackSetManager attackSetManager;

        //Just some variables to set in the editor for the boss
        public float bossHealth = 10f;
        public List<float> healthPhasesList = new List<float>();

        //Attack Patterns
        public List<int> attackPatterns1 = new List<int>(); // thsi will the first attack pattern before the next threshold

        public List<int> attackPatterns2 = new List<int>();

        public List<int> attackPatterns3 = new List<int>();

        public List<int> attackPatterns4 = new List<int>();

        // unsure if i have to set it in oncreate on just through editor is fine, leaving comment here in case
        public GameObject player;

        public override void OnCreate()
        {
            health = new BossHealth(bossHealth);
            phaseController = new PhaseController(healthPhasesList);
            phaseController.PhaseTrigger += HandlePhaseChange;

            List<AttackPatterns> temp = initializeAttackPatterns();

            attackSetManager = new AttackSetManager(temp, healthPhasesList.Count);

            phaseController.StartFirstPhase();
        }
        public override void OnUpdate(float dt)
        {
            phaseController.UpdatePhases(health.PercentageHealth());
            attackSetManager.Update(dt);
            //if health below zero play death animation then delete entity?


        }

        public void HandlePhaseChange(int phase)
        {
            //SliceLog.Log("Phase changed to " + phase);
            //uh ill think of how to do the attack manager might not need it
            attackSetManager.SetPhase(phase);
        }

        public List<AttackPatterns> initializeAttackPatterns()
        {
            List<AttackPatterns> temp = new List<AttackPatterns>();

            if((attackPatterns1.Count == 0)) 
            {
                SliceLog.Log("Boss needs at least the first attack pattern");
                return temp;
            }

            temp.Add(new AttackPatterns(attackPatterns1, gameObject.mID, player.mID));

            if (!(attackPatterns2.Count == 0))// not equal zero, just in case yall never see
            {
                temp.Add(new AttackPatterns(attackPatterns2, gameObject.mID, player.mID));
            }
            if (!(attackPatterns3.Count == 0))
            {
                temp.Add(new AttackPatterns(attackPatterns3, gameObject.mID, player.mID));
            }
            if (!(attackPatterns4.Count == 0))
            {
                temp.Add(new AttackPatterns(attackPatterns4, gameObject.mID, player.mID));
            }

            return temp;
        }
    }
}