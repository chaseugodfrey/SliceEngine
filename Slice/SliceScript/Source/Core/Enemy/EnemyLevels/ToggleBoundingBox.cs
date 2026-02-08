using SliceEngine;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class ToggleBoundingBox : SliceBehaviour
    {
        public GameObject actualBoundingObject;

        public GameObject levelToTurnOn;

        public GameObject levelToTurnOff;

        public bool startOn = false;


        public override void OnCreate()
        {
            base.OnCreate();

            if (levelToTurnOn != null && (levelToTurnOn.Has<LevelKills>() || levelToTurnOn.Has<BaseLevel>()))
            { levelToTurnOn.As<BaseLevel>().MovingToNextLevelEvent += TurnOn; }//SliceLog.Log("Bounding Box Added Turn On Behaviour"); }
            //else if (levelToTurnOn == null)
            //{ SliceLog.Log("Bounding Box Failed To Add Turn On Behaviour, Reference is empty"); }
            //else if (!levelToTurnOn.Has<BaseLevel>())
            //{ SliceLog.Log("Bounding Box Failed To Add Turn On Behaviour, Cannot grab script"); }

            if (levelToTurnOff != null && (levelToTurnOn.Has<LevelKills>() || levelToTurnOn.Has<BaseLevel>()))
            { levelToTurnOff.As<BaseLevel>().MovingToNextLevelEvent += TurnOff; /*SliceLog.Log("Bounding Box Added Turn Off Behaviour");*/ }
            //else
            //{ SliceLog.Log("Bounding Box Failed To Add Turn Off Behaviour"); }

            if (startOn) { TurnOn(); }
            else { TurnOff(); }
        }


        public void TurnOn()
        { actualBoundingObject.SetActive(true);}

        public void TurnOff() 
        { actualBoundingObject.SetActive(false);}
    }
}
