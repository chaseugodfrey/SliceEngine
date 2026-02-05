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
        private ColliderShape _collider;
        private bool _enabled = false;

        public override void OnUpdate(float dt)
        {

        }

        public override void OnCreate()
        {
            Console.WriteLine("General Hitbox On Create " + gameObject.mID);
            _collider = GetComponent<ColliderShape>();
            if (_collider == null)
            {
                Console.WriteLine("collider is null");
            }
        }

        static bool debugFlag1 = false;
        static bool debugFlag2 = false;

        public void TurnOn()
        {
            Console.WriteLine("Turning on General Hit box for " + gameObject.mID);

            if (_collider == null && !debugFlag1)
            {

                Console.WriteLine("collider is null");
                debugFlag1 = true;
            }

            if (!this.HasComponent<ColliderShape>() && !debugFlag2)
            {
                Console.WriteLine("collider has no collider shape");

            }

            if (_collider == null && this.HasComponent<ColliderShape>())
            {
                _collider = GetComponent<ColliderShape>();

                //Console.WriteLine("Getting collider shape in turn on for some reason");
                Console.Write("++ Box Turned On version 2++");
                _enabled = true;
                _collider.ComponentEnabled = true;

            }
            else if (_collider != null)
            {
                Console.Write("++ Box Turned On ++");
                _enabled = true;
                _collider.ComponentEnabled = true;
            }
        }

        public void TurnOff()
        {
            //Redundancy
            if (_collider == null && this.HasComponent<ColliderShape>())
            {
                _collider = GetComponent<ColliderShape>();

                _enabled = false;
                _collider.ComponentEnabled = false;
            }
            else if (_collider != null)
            {
                _enabled = false;
                _collider.ComponentEnabled = false;
            }
            Console.WriteLine("Turning off General Hit box");
        }


        public override void OnTriggerEnter(uint other)
        {
            SliceLog.Log("GENERAL HIT BOX Trigger Enter called");
            //base.OnTriggerEnter(other);

            if (_enabled)
            {
                Console.WriteLine("Enabled");
                HitBoxListeners(gameObject.FindGameObjectWithID(other));
            }
            else
            {
                Console.WriteLine("Enabled not enabled");
            }
        }

        public override void OnTriggerStay(uint other)
        {
            GameObject temp = new GameObject(other);
            if (temp.tag == "Player")
                Console.WriteLine("Ur mudder");
        }

    }
}
