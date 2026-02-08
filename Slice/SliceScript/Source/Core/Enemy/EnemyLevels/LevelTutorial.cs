using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class LevelTutorial : BaseLevel
    {
        public int tutorialIndex = 0;
        public int KillsRequired = 0;
        public override bool CheckObjective()
        {
            if (Bootstrap.HUDManager.As<HUD_Manager>().dialogueDone && KillsRequired == 0)
            {
                return true;
            }

            return false;
        }

        public override void EnemyKilled(GameObject enemy)
        {
            base.EnemyKilled(enemy);

            KillsRequired--;

        }

    }
}
