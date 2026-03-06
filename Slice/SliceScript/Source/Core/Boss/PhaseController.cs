using SliceEngine;
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Security.Permissions;


namespace SliceEngine
{
    public class PhaseController : SliceBehaviour
    {
        //dk if it should be a list or arr
        //phase are base on health threshold percentage 0-1
        private List<float> healthPhases = new List<float>();
        private int currentPhase = 0;
        private int maxPhase;

        public delegate void PhaseTriggerEvent(int phase);
        public event PhaseTriggerEvent PhaseTrigger;

        public PhaseController(List<float> healthPhases)
        {
            if (healthPhases.Count == 0)
            {
                this.healthPhases.Add(0.5f); // just in case, but should be set in boss script
                maxPhase = 1;
            }
            else
            {
                this.healthPhases = healthPhases;
                maxPhase = healthPhases.Count;
            }
        }

        public void StartFirstPhase()
        {
            PhaseTrigger?.Invoke(0);
        }

        public void UpdatePhases(float percentageHealth)
        {
            if (currentPhase >= maxPhase)
                return;

            if  (percentageHealth <= healthPhases[currentPhase])
            {
                ++currentPhase;
                PhaseTrigger?.Invoke(currentPhase); // there will one mroe attack patetrn compared to the phases since the boss will start with a base attack pattern
            }

        }

        public void EndPhase()
        {
            //dk if i need this
        }

    }
}