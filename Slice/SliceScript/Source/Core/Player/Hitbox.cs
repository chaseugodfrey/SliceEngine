using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class Hitbox : SliceBehaviour
    {
        public delegate void Attack();
        public Attack OnAttack;

        public override void OnTriggerEnter(uint other)
        {
            //if (Bootstrap.LevelDirector.EnemyList.TryGetValue(other, out EnemySlime slime))
            //{
            //    //Console.WriteLine();
            //    enemiesInRange.Add(slime);
            //}
        }
        public override void OnTriggerExit(uint other)
        {
            //if (Bootstrap.LevelDirector.EnemyList.TryGetValue(other, out EnemySlime slime))
            //{
            //    enemiesInRange.Remove(slime);
            //}
        }
    }
}
