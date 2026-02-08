using SliceEngine;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class ToggleObjectiveDone : SliceBehaviour
    {
        public FontRenderer textToDisplay;

        //public GameObject levelToTurnOn;

        //public GameObject levelToTurnOff;

        public bool startOn = false;


        public override void OnCreate()
        {
            base.OnCreate();

            textToDisplay = GetComponent<FontRenderer>();

            foreach(GameObject level in Bootstrap.LevelDirector.levels)
            {
                level.As<BaseLevel>().LevelCompleteEvent += TurnOn;
                level.As<BaseLevel>().MovingToNextLevelEvent += TurnOff;
            }
            

            //if (levelToTurnOn != null && (levelToTurnOn.Has<LevelKills>() || levelToTurnOn.Has<BaseLevel>()))
            //{ levelToTurnOn.As<BaseLevel>().MovingToNextLevelEvent += TurnOn; }

            //if (levelToTurnOff != null && (levelToTurnOn.Has<LevelKills>() || levelToTurnOn.Has<BaseLevel>()))
            //{ levelToTurnOff.As<BaseLevel>().MovingToNextLevelEvent += TurnOff; }

            if (startOn) { TurnOn(); }
            else { TurnOff(); }
        }


        public void TurnOn()
        { SliceLog.Log("Turn on objective called"); textToDisplay.SetEnabled(true); }

        public void TurnOff() 
        { textToDisplay.SetEnabled(false); }
    }
}
