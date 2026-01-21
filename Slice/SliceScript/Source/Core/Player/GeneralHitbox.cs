using SliceEngine;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class GeneralHitbox : SliceBehaviour
    {
        public delegate void HitBoxTriggerEvent(GameObject hit);
        public event HitBoxTriggerEvent HitBoxListeners;
        public event HitBoxTriggerEvent ExitListeners;

        public override void OnTriggerEnter(uint other)
        {
            Console.WriteLine("GENERAL HIT BOX Trigger Enter called");
            //base.OnTriggerEnter(other);


            //if (HitBoxListeners != null)
            //{
            //    Console.WriteLine("Enter Hitbox has subs");
            //    SliceLog.Log("Enter Hitbox has subs");
                HitBoxListeners(gameObject.FindGameObjectWithID(other));
            //}
            //else
            //{
            //    Console.WriteLine("Enter Hitbox no subs");
            //    SliceLog.Log("Enter Hitbox no subs");
            //}

            
        }

        //REMOVE THIS ONCE ENABLE IS WORKING
        public override void OnTriggerExit(uint other)
        {
            /*
            //base.OnTriggerEnter(other);
            if (ExitListeners != null)
            {
                Console.WriteLine("Exit Hitbox has subs");
                SliceLog.Log("Exit Hitbox has subs");
                ExitListeners(gameObject.FindGameObjectWithID(other));
            }
            else
            {
                Console.WriteLine("Exit Hitbox no subs");
                SliceLog.Log("Exit Hitbox no subs");
            }
            */
        }
    }
}
