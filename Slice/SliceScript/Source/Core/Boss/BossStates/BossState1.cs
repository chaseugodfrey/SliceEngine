using SliceEngine;
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Security.Permissions;


namespace SliceEngine
{   
    // Move in a straight line to the right
    public class BossState1 : State
    {
        float timer = 0f;
        float speed = 2f;
        Vector3 startPosition;

        public BossState1(uint bossId, uint playerId) : base(bossId, playerId) { }

        public override void Enter() 
        {
            startPosition = transform.Position;
            timer = 0f;
        }

        public override void Update(float dt)
        {
            transform.Position += new Vector3(speed * dt, 0, 0);
            timer += dt;

            if (timer > 3f) // after 3 seconds, finish state
                IsFinished = true;
        }

        public override void Exit() { IsFinished = false; } // reset for reuse

    }
}