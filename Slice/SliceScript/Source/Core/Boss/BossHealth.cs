using SliceEngine;
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Security.Permissions;


namespace SliceEngine
{
    public class BossHealth : SliceBehaviour
    {
        private float max;
        private float current;

        public BossHealth(float maxHealth)
        {
            if(maxHealth <= 0)
            {
                max = 10f;
                current = 10f;
            }
            else 
            {
                max = maxHealth;
                current = maxHealth;
            }

        }

        public float GetCurrentHealth()
        {
            return current;
        }

        public float GetMaxHealth()
        {
            return max;
        }

        public void TakeDamage(float amount)
        {
            current -= amount;
            if (current <= 0)
            {
                current = 0;
            }
        }

        public float PercentageHealth()
        {
            return current / max;
        }

    }
}