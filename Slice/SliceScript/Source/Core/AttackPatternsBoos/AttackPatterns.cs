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

        public AttackPatterns(List<int> attacks)
        {
            // idk how else it should be done due to limitations
            //for now do this each attack will be identified with an index
            foreach (int i in attacks)
            {
                switch (i)
                {
                    case 0:
                        this.attacks.Add(new ExampleAttack());
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
        public override void OnUpdate(float dt)
        {
        }

    }
}