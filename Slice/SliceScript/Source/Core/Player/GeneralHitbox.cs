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
            Console.WriteLine("Trigger Enter called");
            base.OnTriggerEnter(other);
            HitBoxListeners(gameObject.FindGameObjectWithID(other));
        }

        //REMOVE THIS ONCE ENABLE IS WORKING
        public override void OnTriggerExit(uint other)
        {
            base.OnTriggerEnter(other);
            ExitListeners(gameObject.FindGameObjectWithID(other));
        }
    }
}
