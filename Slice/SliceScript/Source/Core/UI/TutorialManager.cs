using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class TutorialManager : SliceBehaviour
    {
        //public GameObject HUDManager;
        public List<GameObject> triggerBox = new List<GameObject>();
        public int currIndex = 0;
        public int scene = 0;

        public override void OnCreate()
        {
            base.OnCreate();
            foreach (GameObject trigger in triggerBox)
            {
                if (trigger != null && (trigger.Has<LevelTutorial>() || trigger.Has<BaseLevel>()))
                {
                    trigger.As<BaseLevel>().LevelCompleteEvent += IncrementTutorial;
                }
            }
           
        }

        public void IncrementTutorial()
        {
            if (currIndex >= triggerBox.Count) return;

            //Bootstrap.HUDManager.As<HUD_Manager>().PlayDialogueForLevel(triggerBox[currIndex].As<LevelTutorial>().tutorialIndex, scene);
            currIndex++;
        }
    }


}
