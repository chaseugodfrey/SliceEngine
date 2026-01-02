using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public abstract class EnemyState
    {
        protected EnemyBase enemyOwner;
        public void SetUp(EnemyBase ownerInput)
        {
            Console.WriteLine("Base Set Up");
            enemyOwner = ownerInput;
        }
        public abstract void DoEnemyAction(float dt); // Called on Update from Base enemy class
        public abstract void DoEnemyActionFixed(); // Called on Fixed Update from Base enemy class
        public abstract void ReachTargetAction(); 
        public abstract void OnLanding();
        public abstract void OnCollide();
        public abstract void OnDamaged();
    }
}
