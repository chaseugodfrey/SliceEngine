using System;
using System.Collections;
using System.Collections.Generic;

namespace SliceEngine
{
    public class Attack_1 : SliceBehaviour
    {
        public override void OnCollideEnter(uint other)
        {
            Console.WriteLine("Attack Hit ENTER! entity <" + other + ">LOLOLOLOLOLOL");
        }

        public override void OnCollideStay(uint other)
        {
            //Console.WriteLine("Attack Hit STAY! entity <" + other + ">LOLOLOLOLOLOL");
        }
    }
}
