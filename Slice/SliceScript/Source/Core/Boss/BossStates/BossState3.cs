using SliceEngine;
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Security.Permissions;
using System.Threading;


namespace SliceEngine
{   
    // Move in a square pattern
    public class BossState3 : State
    {
        float timer = 0f;
        float speed = 2f;
        int squareStep = 0;
        float stepTimer = 0f;
        Vector3 startPosition;

        public BossState3(uint bossId, uint playerId) : base(bossId, playerId) { }

        public override void Enter() 
        {
            startPosition = transform.Position;
            timer = 0f;
        }

        public override void Update(float dt)
        {
            stepTimer += dt;
            Vector3 move = Vector3.Zero;

            switch (squareStep)
            {
                case 0: move = new Vector3(speed * dt, 0, 0); break; // right
                case 1: move = new Vector3(0, speed * dt, 0); break; // up
                case 2: move = new Vector3(-speed * dt, 0, 0); break; // left
                case 3: move = new Vector3(0, -speed * dt, 0); break; // down
            }

            transform.Position += move;

            if (stepTimer > 1f) // move 1 second per side
            {
                stepTimer = 0f;
                squareStep++;
                if (squareStep > 3)
                    IsFinished = true; // finished full square
            }
        }

        public override void Exit() { IsFinished = false; } // reset for reuse

    }
}