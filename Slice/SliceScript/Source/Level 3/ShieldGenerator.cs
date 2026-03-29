using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class ShieldGenerator : EnemyBase
    {
        public delegate void ShieldGeneratorDestroyedEvent(GameObject gen);
        public event ShieldGeneratorDestroyedEvent DestroyTrigger;

        public bool generating = false;
        private ColliderShape hitbox;

        //Function called when you want the enemy to be active
        public override void OnCreate()
        {
            enemyT = GetComponent<Transform>();
            active = true;
        }

        public override void OnAwake()
        {
            SliceLog.Console("g" + gameObject.mID);
            hitbox = GetComponent<ColliderShape>();
            SliceLog.Console("c" + hitbox.gameObject.mID);
            hitbox.ComponentEnabled = false;
        }

        public override void OnUpdate(float dt)
        {
            //base.OnUpdate(dt);
        }

        public override void TakeDamage(int amount, GameObject source = null)
        {
            if (!active) return;
            if (!generating) return;

            SliceLog.Console("SHIELD GENERATOR is taking damage");

            base.TakeDamage(1, source);
        }

        protected override void OnDamaged(GameObject source)
        {
            // put some vfx here
        }

        public override void OnDeath()
        {
            DestroyTrigger?.Invoke(this.gameObject);
            active = false;
            generating = false;
            hitbox.ComponentEnabled = false;
            SliceLog.Console("SHIELD GENERATOR DESTROYED");
        }

        public void StartGenerating()
        {
            generating = true;
            hitbox.ComponentEnabled = true;
        }

        public void GenerateShield()
        {
            // animation here
        }

        public void StopGenerating()
        {
            generating = false;
            hitbox.ComponentEnabled = false;
            SliceLog.Console("SHIELD GENERATOR STOPPED GENERATING SHIELDS");
        }
    }
}
