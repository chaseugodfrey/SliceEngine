using SliceEngine;
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Security.Permissions;


namespace SliceEngine
{

    // this class will handle the transition between the states
    public class Phase : SliceBehaviour
    {
        public List<State> states = new List<State>();

        private int currentAttack = 0;
        public bool IsFinished { get; private set; } = false;

        public void Enter()
        {
            currentAttack = 0;
            IsFinished = false;

            if (states.Count > 0)
            {
                states[currentAttack].Enter();
            }
        }
        public void Update(float dt)
        {
            if (states.Count == 0) 
                return;

            State active = states[currentAttack];
            active.Update(dt);

            if (active.IsFinished)
            {
                active.Exit();
                currentAttack++;

                if (currentAttack >= states.Count)
                {
                    currentAttack = 0;
                    IsFinished = true;
                }

                states[currentAttack].Enter();
            }
        }
        public void Exit()
        {
            if (states.Count > 0) states[currentAttack].Exit();
            IsFinished = false;
        }

        public Phase(List<int> states, uint bossID, uint playerID)
        {
            //idk how else it should be done due to limitations
            //for now do this each attack will be identified with an index
            foreach (int i in states)
            {
                switch (i)
                {
                    case 0:
                        this.states.Add(new ExampleAttack(bossID, playerID));
                        break;
                    //case 1:
                    //    this.states.Add(new Behaviour2());
                    //    break;
                    //case 2:
                    //    this.states.Add(new Behaviour3());
                    //    break;
                    default:
                        SliceLog.Log("Invalid attack pattern index: " + i);
                        break;
                }
            }
        }


    }
}