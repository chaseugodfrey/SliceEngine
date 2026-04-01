using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class LevelKills : BaseLevel
    {
        public int KillsRequired = 0;

        /// <summary>
        /// If kills required is 0, then is done
        /// </summary>
        /// <returns></returns>
        public override bool CheckObjective()
        {
            //SliceLog.Log("kills objective beign checked. Kills left:" + KillsRequired);

            if (KillsRequired <= 0)
            {
                // Clear enemies
                // Display objective done
                Bootstrap.LevelDirector.levels[Bootstrap.LevelDirector.currLevel].As<BaseLevel>().TriggerLevelCompleteEvent();
                return true;
            }
            return false;
        }

        public override void EnemyKilled(GameObject enemy)
        {
            base.EnemyKilled(enemy);
        }
    }
}
