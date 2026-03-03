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
        private int maxPhases;

        public AttackSetManager(List<AttackPatterns> atpattern, int numberOfPhases)
        {
            attackSets = atpattern;
            maxPhases = numberOfPhases;
        }
        public override void OnUpdate(float dt)
        {

        }

    }
}