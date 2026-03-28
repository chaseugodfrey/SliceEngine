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

        //Function called when you want the enemy to be active
        public override void OnCreate()
        {
            enemyT = GetComponent<Transform>();
            active = true;
            currentHealth = 1; // for testing purposes, set to 1 so it dies in one hit
        }

        public override void OnUpdate(float dt)
        {
            //base.OnUpdate(dt);
        }

        public override void TakeDamage(int amount, GameObject source = null)
        {
            if (!active) return;
            if (!generating) return;

            // This override is just to insert a debug
            //Console.WriteLine("Enemy is taking damage");
            SliceLog.Console("SHIELD GENERATOR is taking damage");

            base.TakeDamage(amount, source);

        }

        protected override void OnDamaged(GameObject source)
        {
            //rb.AddForce(new Vector3(0, vertKnockback, horKnockback), ForceMode.Impulse); 
            //CreateGameObject("Prefabs/Sparks.prefab").GetComponent<Transform>().Position = transform.Position;
            SliceLog.Console("SHIELD GENERATOR IS BEING HIT");
        }

        public override void OnDeath()
        {
            DestroyTrigger?.Invoke(this.gameObject);
            active = false;
            SliceLog.Console("SHIELD GENERATOR DESTROYED");
        }

        public void GenerateShields()
        {
            generating = true;
            SliceLog.Console("SHIELD GENERATOR STARTED GENERATING SHIELDS");
        }

        public void StopGenerating()
        {
            generating = false;
            SliceLog.Console("SHIELD GENERATOR STOPPED GENERATING SHIELDS");
        }
    }
}
