using SliceEngine;
using System;


namespace SliceEngine
{
    public abstract class Entity : SliceBehaviour
    {
        #region --- Fields --
        public int maxHealth        = 100;
        public int currentHealth    = 100;

        public int damage           = 100;

        public float movementSpeed  = 5f;
        #endregion

        public override void OnUpdate(float dt)
        {

        }

        public virtual void Heal(int amount)
        {
            currentHealth = (int)Utilities.Clamp(currentHealth + amount, 0, maxHealth);
            OnHeal();
        }

        public virtual void TakeDamage(int amount , GameObject source = null)
        {
            //source = source ?? gameObject;
            if (source == null)
            {
                source = gameObject;
            }
            //Console.WriteLine("Enitity taking damage");
            //Debug.Log($"{name} taking {amount} damage");
            this.currentHealth -= amount;
            if (this.currentHealth > 0) { OnDamaged(source); }
            if (this.currentHealth <= 0)
            {
                currentHealth = 0; // Ensure health doesn't go below zero
                OnDamaged(source);
                OnDeath();
            }
        }

        #region --- On Abstract Methods ---
        protected abstract void OnHeal();
        protected abstract void OnDamaged(GameObject source);
        public abstract void OnDeath();
        #endregion
    }
}