using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using SliceEngine;

namespace SliceScript.Source.Core
{
    public class FeetCollider : SliceBehaviour
    {
        public bool isLeft = false;
        GameObject floor;
        public Payload Golem;

        public override void OnCreate()
        {
            floor = gameObject.FindGameObjectWithName("Floor");
            Golem = gameObject.FindGameObjectWithName("Golem")?.As<Payload>();
        }

        public override void OnCollideEnter(uint other)
        {
            if (other == floor.mID)
            {
                if (isLeft)
                {
                    Golem.isLeftGrounded = true;
                }
                else
                {
                    Golem.isRightGrounded = true;
                }
                
            }
        }
        public override void OnCollideExit(uint other)
        {
            if(other == floor.mID)
            {
                if (isLeft)
                {
                    Golem.isLeftGrounded = false;
                }
                else
                {
                    Golem.isRightGrounded = false;
                }
            }
        }
    }
}
