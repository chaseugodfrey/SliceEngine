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
        public GameObject levelToTurnOn;

        public GameObject levelToTurnOff;

        public bool startOn = false;


        public override void OnAwake()
        {
            base.OnAwake();

            if (startOn)    { TurnOn(); }
            else            { TurnOff(); }

            if (levelToTurnOn != null && levelToTurnOn.Has<BaseLevel>()) 
            { levelToTurnOn.As<BaseLevel>().LevelCompleteEvent += TurnOn; }

            if (levelToTurnOff != null && levelToTurnOff.Has<BaseLevel>())
            { levelToTurnOff.As<BaseLevel>().LevelCompleteEvent += TurnOff; }

        }


        public void TurnOn()
        {this.gameObject.SetActive(true);}

        public void TurnOff() 
        {this.gameObject.SetActive(false);}
    }
}
