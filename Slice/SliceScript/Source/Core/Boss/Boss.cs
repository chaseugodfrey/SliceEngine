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
        //private AttackSetManager attackSetManager;

        //Just some variables to set in the editor for the boss
        public float bossHealth = 10f;
        public List<float> healthPhasesList = new List<float>();

        //Attack Patterns
        public List<int> stateList1 = new List<int>(); // thsi will the first attack pattern before the next threshold

        public List<int> stateList2 = new List<int>();

        public List<int> stateList3 = new List<int>();

        public List<int> stateList4 = new List<int>();

        // unsure if i have to set it in oncreate on just through editor is fine, leaving comment here in case
        public GameObject player;

        public override void OnCreate()
        {
            health = new BossHealth(bossHealth);


            List<Phase> temp = InitalizeStateList();

            phaseController = new PhaseController(healthPhasesList, temp);
            //phaseController.PhaseTrigger += HandlePhaseChange;

            //attackSetManager = new AttackSetManager(temp, healthPhasesList.Count);

            phaseController.StartFirstPhase();
        }
        public override void OnUpdate(float dt)
        {
            phaseController.UpdateActivePhase(health.PercentageHealth());
            phaseController.Update(dt);
            //if health below zero play death animation then delete entity?


        }

        public void HandlePhaseChange(int phase)
        {
            //SliceLog.Log("Phase changed to " + phase);
            //uh ill think of how to do the attack manager might not need it
            phaseController.SetPhase(phase);
        }

        public List<Phase> InitalizeStateList()
        {
            List<Phase> temp = new List<Phase>();

            if((stateList1.Count == 0)) 
            {
                SliceLog.Log("Boss needs at least a base state list for behaviour");
                return temp;
            }

            temp.Add(new Phase(stateList1, gameObject.mID, player.mID));

            if (!(stateList2.Count == 0))// not equal zero, just in case yall never see
            {
                temp.Add(new Phase(stateList2, gameObject.mID, player.mID));
            }
            if (!(stateList3.Count == 0))
            {
                temp.Add(new Phase(stateList3, gameObject.mID, player.mID));
            }
            if (!(stateList4.Count == 0))
            {
                temp.Add(new Phase(stateList4, gameObject.mID, player.mID));
            }

            return temp;
        }
    }
}