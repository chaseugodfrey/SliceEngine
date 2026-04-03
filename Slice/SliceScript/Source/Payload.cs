using SliceEngine;
using SliceScript.Source.Core;
using System;
using System.Collections.Generic;


namespace SliceEngine
{
    public class Payload : SliceBehaviour
    {
        //public WaypointSystem waypointSystem;
        public List<string> wayPoints = new List<string>();
        Animator animator;
        Transform t;

        public bool isLeftGrounded = true;
        public bool isRightGrounded = true;
        bool playSound = false;

        public Vector3 up = new Vector3(0.0f, 1.0f, 0.0f);

        AudioSource myAudio;
        public float moveSpeed = 0.7f;
        public int healthPoints = 10;

        public int[] test2 = new int[5];
        public List<float> testList = new List<float>();
        // TODO: Show arrays in inspector like unity does wiht List<> and stuff
        // TODO: GameObject variable should show as a string in inspector, but cna be converted to a gO in script

        public override void OnCreate()
        {
            //waypointSystem = new WaypointSystem();
            testList.Add(0.0f);

            wayPoints.Add("Waypoint1");
            wayPoints.Add("Waypoint2");
            wayPoints.Add("Waypoint3");
            wayPoints.Add("Waypoint4");
            wayPoints.Add("Waypoint5");
            wayPoints.Add("Waypoint6");
            wayPoints.Add("Waypoint7");
            wayPoints.Add("Waypoint8");
            wayPoints.Add("Waypoint9");
            wayPoints.Add("Waypoint10");

/*            foreach(string point in wayPoints)
            {
                waypointSystem.AddWaypoint(gameObject.FindGameObjectWithName(point).GetComponent<Transform>().Position);
            }*/

            t = GetComponent<Transform>();
            animator = GetComponent<Animator>();
            myAudio = GetComponent<AudioSource>();
        }

        public override void OnUpdate(float dt)
        {
            //waypointSystem.Update(t.Position);

            //Vector3 moveTo = new Vector3(waypointSystem.GetTargetPosition().x, t.Position.y, waypointSystem.GetTargetPosition().z);

            //Vector3 dir = (waypointSystem.GetTargetPosition() - t.Position).Normalize();

            //dir = Vector3.RotateTowards(dir, waypointSystem.GetTargetPosition().Normalize(), 30.0f*dt);
            //t.RotationQuat = Quaternion.LookRotation(dir, this.up);

            if (healthPoints > 0 && animator.GetCurrAnimTime() < 1.3f)
            {
                //t.Position += dir * moveSpeed * dt;
                playSound = false;
            }
            else
            {
                if (!playSound)
                {
                    AudioSettings.PlaySFX("Golem_LeftStomp", t.Position);
                    playSound = true;
                }
            }


            if (String.Compare(animator.GetCurrAnimName(), "Idle") == 0)
            {
                animator.SetBool("Step_Left", true);
            }

            if (String.Compare(animator.GetCurrAnimName(), "Step_Left") == 0)
            {
                animator.SetBool("Step_Right", true);
            }
            if (String.Compare(animator.GetCurrAnimName(), "Step_Right") == 0)
            {
                animator.SetBool("Step_Left", true);
            }

            if (String.Compare(animator.GetCurrAnimName(), "Step_Left") == 0 && healthPoints == 0)
            {
                animator.SetBool("StopFromLeft", true);
            }

        }
    }
}
