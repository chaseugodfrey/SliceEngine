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


        public override void OnAwake()
        {
            base.OnAwake();

            if (startOn)    { TurnOn(); }
            else            { TurnOff(); }

            if (levelToTurnOn != null && levelToTurnOn.Has<BaseLevel>()) 
            { levelToTurnOn.As<BaseLevel>().LevelCompleteEvent += TurnOn; SliceLog.Log("Bounding Box Added Turn On Behaviour"); }

            if (levelToTurnOff != null && levelToTurnOff.Has<BaseLevel>())
            { levelToTurnOff.As<BaseLevel>().LevelCompleteEvent += TurnOff; SliceLog.Log("Bounding Box Added Turn Off Behaviour"); }

        }


        public void TurnOn()
        { actualBoundingObject.SetActive(true);}

        public void TurnOff() 
        { actualBoundingObject.SetActive(false);}
    }
}
