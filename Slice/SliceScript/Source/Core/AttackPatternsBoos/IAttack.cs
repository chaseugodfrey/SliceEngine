using SliceEngine;
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Security.Permissions;


namespace SliceEngine
{

    // this class will have the logic for the attack, and will be used as a base class for all attacks
    public class IAttack : SliceBehaviour
    {
        protected ulong bossId;
        protected ulong playerId;

        public IAttack(ulong bossId, ulong playerId)
        {
            this.bossId = bossId;
            this.playerId = playerId;
        }

        public virtual void Enter() { }
        public virtual void Exit() { }
        public virtual bool IsFinished { get; protected set; } = false;
        public virtual void Update(float dt) { } // not to be confused with onUpdate
    }
}