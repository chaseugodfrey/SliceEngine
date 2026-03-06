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

        private List<AttackPatterns> attackSets = new List<AttackPatterns>();
        private int currentSet = 0;
        private int maxNumberAttackPattern;

        public AttackSetManager(List<AttackPatterns> atpattern, int numberOfPhases)
        {
            attackSets = atpattern;
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
                attackSets[currentSet].Enter();
            }
            else
            {
                attackSets[currentSet].Exit();
                currentSet = phase;
                attackSets[currentSet].Enter();
            }

        }
        public void Update(float dt)
        {
            if (attackSets.Count == 0)
            {
               SliceLog.Log("No attack patterns set in AttackSetManager");
               return;
            }
            attackSets[currentSet].Update(dt);
        }

    }
}