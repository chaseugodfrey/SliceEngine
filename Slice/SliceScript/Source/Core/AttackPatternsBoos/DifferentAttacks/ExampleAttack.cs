using SliceEngine;
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Security.Permissions;


namespace SliceEngine
{
    // example attack behaviour
    public class  ExampleAttack : IAttack
    {
        public ExampleAttack(uint bossId, uint playerId) : base(bossId, playerId) { }

        public override void Enter() { }

        public override void Update(float dt)
        {
            //attack logic here set is finish to true to go to next attack
        }

        public override void Exit() { IsFinished = false; } // reset for reuse

    }
}