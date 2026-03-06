using SliceEngine;
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Security.Permissions;


namespace SliceEngine
{

    // this class will handle the transition between the attacks
    public class AttackPatterns : SliceBehaviour
    {
        private List<IAttack> attacks = new List<IAttack>();

        private int currentAttack = 0;
        public bool IsFinished { get; private set; } = false;

        public void Enter()
        {
            currentAttack = 0;
            IsFinished = false;

            if (attacks.Count > 0)
            {
                attacks[currentAttack].Enter();
            }
        }
        public void Update(float dt)
        {
            if (attacks.Count == 0) 
                return;

            IAttack active = attacks[currentAttack];
            active.Update(dt);

            if (active.IsFinished)
            {
                active.Exit();
                currentAttack++;

                if (currentAttack >= attacks.Count)
                {
                    currentAttack = 0;
                    IsFinished = true;
                }

                attacks[currentAttack].Enter();
            }
        }
        public void Exit()
        {
            if (attacks.Count > 0) attacks[currentAttack].Exit();
            IsFinished = false;
        }

        public AttackPatterns(List<int> attacks, uint bossID, uint playerID)
        {
            //idk how else it should be done due to limitations
            //for now do this each attack will be identified with an index
            foreach (int i in attacks)
            {
                switch (i)
                {
                    case 0:
                        this.attacks.Add(new ExampleAttack(bossID, playerID));
                        break;
                    //case 1:
                    //    this.attacks.Add(new Behaviour2());
                    //    break;
                    //case 2:
                    //    this.attacks.Add(new Behaviour3());
                    //    break;
                    default:
                        SliceLog.Log("Invalid attack pattern index: " + i);
                        break;
                }
            }
        }


    }
}