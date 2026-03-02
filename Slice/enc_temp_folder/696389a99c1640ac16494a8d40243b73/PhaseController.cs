using SliceEngine;
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Security.Permissions;


namespace SliceEngine
{
    public class PhaseController : SliceBehaviour
    {
        private List<float> healthPhases = new List<float>(); //dk if it should be a list or arr
        private int currentPhase = 0;
        private int MaxPhase;



        public PhaseController(List<float> healthPhases)
        {
            if (healthPhases.Count == 0)
            {
                this.healthPhases.Add(0.5f); // just in case, but should be set in boss script
                MaxPhase = 1;
            }
            else
            {
                this.healthPhases = healthPhases;
                MaxPhase = healthPhases.Count;
            }
        }

        public override void OnUpdate(float dt)
        {
        }

    }
}