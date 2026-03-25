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
        public delegate void ShieldGeneratorDestroyedEvent();
        public event ShieldGeneratorDestroyedEvent Destroyedtrigger;

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
            // This override is just to insert a debug
            //Console.WriteLine("Enemy is taking damage");
            SliceLog.Console("SHIELD GENERATOR is taking damage");

            //source = source ?? gameObject;
            if (source == null)
            {
                source = gameObject;
            }
            Console.WriteLine($"taking {amount} damage and current health {currentHealth}");
            this.currentHealth -= amount;
            if (this.currentHealth > 0) { OnDamaged(source); }
            if (this.currentHealth <= 0)
            {
                currentHealth = 0; // Ensure health doesn't go below zero
                OnDeath();
            }

        }

        protected override void OnDamaged(GameObject source)
        {
            //rb.AddForce(new Vector3(0, vertKnockback, horKnockback), ForceMode.Impulse); 
            //CreateGameObject("Prefabs/Sparks.prefab").GetComponent<Transform>().Position = transform.Position;
            SliceLog.Console("SHIELD GENERATOR IS BEING HIT");

        }

        
        public override void OnDeath()
        {
            Destroyedtrigger?.Invoke();
            SliceLog.Console("SHIELD GENERATOR DESTROYED");
            this.gameObject.Destroy(); 

        }

    }
}
