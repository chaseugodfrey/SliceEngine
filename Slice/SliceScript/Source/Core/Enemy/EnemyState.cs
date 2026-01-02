using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public abstract class EnemyState
    {
        public abstract void DoEnemyAction();
        public abstract void DoEnemyActionFixed();
        public abstract void ReachTargetAction();
        public abstract void OnLanding();
        public abstract void OnCollide();
        public abstract void OnDamaged();
    }
}
