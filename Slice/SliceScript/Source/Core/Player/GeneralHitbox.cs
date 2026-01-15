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

        private void OnTriggerEnter(uint other)
        {
            HitBoxListeners(gameObject.FindGameObjectWithID(other));
        }
    }
}
