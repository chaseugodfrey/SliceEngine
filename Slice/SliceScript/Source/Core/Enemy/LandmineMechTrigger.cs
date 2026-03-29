using SliceEngine;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.Remoting.Metadata.W3cXsd2001;

namespace SliceEngine
{
    public class LandmineMechTrigger : SliceBehaviour
    {
        public bool triggered = false;

        public override void OnTriggerEnter(uint other)
        {
            if (triggered) { return; }            
            base.OnTriggerEnter(other);
            GameObject collidedGO = FindGameObjectWithID(other);
            if (collidedGO != null && collidedGO.tag == "Player")
            {
                triggered = true;
            }
        }
    }
}