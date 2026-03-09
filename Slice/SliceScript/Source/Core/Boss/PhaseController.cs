using SliceEngine;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Security.Permissions;


namespace SliceEngine
{
    public class PhaseController : SliceBehaviour
    {
        //dk if it should be a list or arr
        //phase are base on health threshold percentage 0-1
        private List<float> healthPhases = new List<float>();

        //phases will hold the list of states
        private List<Phase> phases = new List<Phase>();

        
        private int currentPhase = 0;
        private int maxPhase;

        // there will be one more state than the phases since the boss will start with a base state before the first threshold is reached, so if there are 3 phases there will be 4 states
        private int maxNumberStates;

        public delegate void PhaseTriggerEvent(int phase);
        public event PhaseTriggerEvent PhaseTrigger;

        public PhaseController(List<float> healthPhases, List<Phase> phaseList)
        {
            if (healthPhases.Count == 0)
            {
                this.healthPhases.Add(0.5f); // just in case, but should be set in boss script
                maxPhase = 1;

                phases = phaseList;
                maxNumberStates = phaseList.Count;
            }
            else
            {
                this.healthPhases = healthPhases;
                maxPhase = healthPhases.Count;

                phases = phaseList;
                maxNumberStates = phaseList.Count;
            }
        }

        public void SetPhase(int phase)
        {
            if (phase < 0 || phase >= maxNumberStates)
            {
                SliceLog.Log("Invalid phase index: " + phase);
                return;
            }

            if (phase == 0)
            {
                currentPhase = phase;
                phases[currentPhase].Enter();
            }
            else
            {
                phases[currentPhase].Exit();
                currentPhase = phase;
                phases[currentPhase].Enter();
            }

        }
        public void Update(float dt)
        {
            if (phases.Count == 0)
            {
                SliceLog.Log("No attack patterns set in AttackSetManager");
                return;
            }
            phases[currentPhase].Update(dt);
        }

        public void StartFirstPhase()
        {
            SetPhase(0);
        }

        public void UpdateActivePhase(float percentageHealth)
        {
            if (currentPhase >= maxPhase)
                return;

            if  (percentageHealth <= healthPhases[currentPhase])
            {
                currentPhase++;
                SetPhase(currentPhase);
                //PhaseTrigger?.Invoke(currentPhase); // there will one mroe attack patetrn compared to the phases since the boss will start with a base attack pattern
            }

        }

        public void EndPhase()
        {
            //dk if i need this
        }

    }
}