using SliceEngine;
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Security.Permissions;
using System.Threading;


namespace SliceEngine
{
    // Move up and down like a sine wave forever
    public class BossState2 : State
    {
        float timer = 0f;
        float amplitude = 2f; // up/down range
        float frequency = 2f; // speed of oscillation
        Vector3 startPosition;

        public BossState2(uint bossId, uint playerId) : base(bossId, playerId) { }

        public override void Enter()
        {
            startPosition = transform.Position;
            timer = 0f;
        }

        public override void Update(float dt)
        {
            timer += dt;
            float yOffset = (float)Math.Sin(timer * frequency) * amplitude;
            transform.Position = startPosition + new Vector3(0, yOffset, 0);

        }

        public override void Exit()
        {
        }
    }
}