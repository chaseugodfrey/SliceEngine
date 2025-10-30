using SliceEngine;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{


    public class AudioComponent : Component
    {

        public Audio audio
        {
            get
            {
                //Console.WriteLine($"Entity ID:{Entity.mID}");
                string audioName = FunctionCalls.Audio_GetSoundName(Entity.mID);
                return new Audio(audioName);
            }
            //set
            //{
            //    //Console.WriteLine($"Entity ID:{Entity.mID}");

            //    FunctionCalls.Audio_SetSoundName(Entity.mID, ref value);
            //}
        }

    }


}

