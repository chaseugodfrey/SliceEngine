using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class PlayFinalAudio : SliceBehaviour
    {

        public override void OnCreate()
        {
            base.OnCreate();

            AudioSettings.PlaySFX("05_04_Ozone_Final");
        }

    }
}
