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
            if (_enabled)
            {
                Console.Write(" __ On update is turning enabled off __ ");
                _enabled = false;
            }
        }
        public override void OnCreate()
        {
            base.OnCreate();
            _collider = GetComponent<ColliderShape>();
        }

        public void TurnOn()
        {
            if (_collider == null && this.HasComponent<ColliderShape>())
            {
                _collider = GetComponent<ColliderShape>();
            }
            else if (_collider != null)
            {
                Console.Write("++ Box Turned On ++");
                _collider.ComponentEnabled = true;
                _enabled = true;
            }
        }

        public void TurnOff()
        {
            //Redundancy
            if (_collider == null && this.HasComponent<ColliderShape>())
            {
                _collider = GetComponent<ColliderShape>();
            }
            else if (_collider != null)
            {
                _collider.ComponentEnabled = false;
                _enabled = false;
            }
        }


        public override void OnTriggerEnter(uint other)
        {
            Console.WriteLine("GENERAL HIT BOX Trigger Enter called");
            //base.OnTriggerEnter(other);


            //if (HitBoxListeners != null)
            //{
            //    Console.WriteLine("Enter Hitbox has subs");
            //    SliceLog.Log("Enter Hitbox has subs");

            if (_enabled)
            { 
                HitBoxListeners(gameObject.FindGameObjectWithID(other));
            }
            //}
            //else
            //{
            //    Console.WriteLine("Enter Hitbox no subs");
            //    SliceLog.Log("Enter Hitbox no subs");
            //}

            
        }

        public override void OnTriggerStay(uint other)
        {
            Console.WriteLine("!! STAY detected!!");
            base.OnTriggerStay(other);
            if (_enabled)
            {
                Console.Write("!! STAY IS THROWING THINGS !!");
                HitBoxListeners(gameObject.FindGameObjectWithID(other));
            }
        }
    }
}
