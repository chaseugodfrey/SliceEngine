using SliceEngine;
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Security.Permissions;


namespace SliceEngine
{
    //sets which attack pattern to use
    //not in use anymore here just for reference, the phase controller will handle the transition between the attack patterns now
    public class AttackSetManager : SliceBehaviour
    {

        private List<Phase> phases = new List<Phase>();
        private int currentPhase = 0;
        private int maxNumberAttackPattern;

        public AttackSetManager(List<Phase> atpattern, int numberOfPhases)
        {
            phases = atpattern;
            //currentPhase = phases[0];
            maxNumberAttackPattern = numberOfPhases + 1;
        }

        public void SetPhase(int phase)
        {
            if (phase < 0 || phase >= maxNumberAttackPattern)
            {
                SliceLog.Log("Invalid phase index: " + phase);
                return;
            }

            if(phase == 0)
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

    }
}