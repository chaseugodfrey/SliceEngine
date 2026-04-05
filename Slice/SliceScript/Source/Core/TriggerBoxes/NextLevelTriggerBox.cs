using SliceEngine;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Diagnostics.Contracts;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class NextLevelTriggerBox : SliceBehaviour
    {

        //public delegate void HitBoxTriggerEvent(GameObject hit);
        //public event HitBoxTriggerEvent HitBoxListeners;
        //private ColliderShape _collider;

        //private bool _done = false;
        private bool _enabled = false;
        public float LoadDelay = 1f;
        public GameObject DoorModel;
        Animator animator;
        public bool check = true;

        private bool counting = false;
        float count = 0f;

        public override void OnUpdate(float dt)
        {
            if (!check)
                return;

            base.OnUpdate(dt);


            if (counting)
            {
                count += Time.deltaTime;
                if (count >= LoadDelay)
                {
                    check = false;
                    Bootstrap.LevelDirector.LoadNextLevel();
                }
            }

        }

        //public override void OnCreate()
        //{
        //    //Console.WriteLine("General Hitbox On Create " + gameObject.mID);
        //    _collider = GetComponent<ColliderShape>();
        //    if (_collider == null)
        //    {
        //        //console.writeline("collider is null");
        //    }
        //}

        static bool debugFlag1 = false;
        static bool debugFlag2 = false;

        //public void TurnOn()
        //{
        //    //console.writeline("Turning on General Hit box for " + gameObject.mID);

        //    if (_collider == null && !debugFlag1)
        //    {

        //        //console.writeline("collider is null");
        //        debugFlag1 = true;
        //    }

        //    if (!this.HasComponent<ColliderShape>() && !debugFlag2)
        //    {
        //        //console.writeline("collider has no collider shape");

        //    }

        //    if (_collider == null && this.HasComponent<ColliderShape>())
        //    {
        //        _collider = GetComponent<ColliderShape>();

        //        //Console.WriteLine("Getting collider shape in turn on for some reason");
        //        //Console.Write("++ Box Turned On version 2++");
        //        _enabled = true;
        //        _collider.ComponentEnabled = true;

        //    }
        //    else if (_collider != null)
        //    {
        //        //Console.Write("++ Box Turned On ++");
        //        _enabled = true;
        //        _collider.ComponentEnabled = true;
        //    }
        //}

        //public void TurnOff()
        //{
        //    //Redundancy
        //    if (_collider == null && this.HasComponent<ColliderShape>())
        //    {
        //        _collider = GetComponent<ColliderShape>();

        //        _enabled = false;
        //        _collider.ComponentEnabled = false;
        //    }
        //    else if (_collider != null)
        //    {
        //        _enabled = false;
        //        _collider.ComponentEnabled = false;
        //    }
        //    //console.writeline("Turning off General Hit box");
        //}


        public override void OnTriggerEnter(uint other)
        {
            base.OnTriggerEnter(other);
            SliceLog.Log("Dialogue Trigger Enter called");
            //base.OnTriggerEnter(other);

            GameObject hit = gameObject.FindGameObjectWithID(other);
            //DoorModel = gameObject.FindGameObjectWithName("Door_1"); if (DoorModel == null) SliceLog.Warn("DoorController cannot find RootNode");
            //animator = DoorModel?.GetComponent<Animator>(); if (DoorModel == null) SliceLog.Warn("DoorController cannot find Animator");

            if (hit.Has<PlayerController>() && Bootstrap.Player == hit.As<PlayerController>() && !_enabled)
            {
                //console.writeline("Enabled");
                _enabled = true;
                animator?.SetBool("Open", true);
                counting = true;
                //Bootstrap.LevelDirector.LoadNextLevel();
            }
        }

    }
}
