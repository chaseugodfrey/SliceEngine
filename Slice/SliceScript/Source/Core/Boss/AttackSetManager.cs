using SliceEngine;
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Security.Permissions;


namespace SliceEngine
{
    //sets which attack pattern to use
    public class AttackSetManager : SliceBehaviour
    {

        private List<Phase> phases = new List<Phase>();
        private int currentSet = 0;
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
                currentSet = phase;
                phases[currentSet].Enter();
            }
            else
            {
                phases[currentSet].Exit();
                currentSet = phase;
                phases[currentSet].Enter();
            }

        }
        public void Update(float dt)
        {
            if (phases.Count == 0)
            {
               SliceLog.Log("No attack patterns set in AttackSetManager");
               return;
            }
            phases[currentSet].Update(dt);
        }

    }
}