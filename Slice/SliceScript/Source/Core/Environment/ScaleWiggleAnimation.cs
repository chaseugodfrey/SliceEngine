using System;
using System.Collections.Generic;
using System.Diagnostics.Eventing.Reader;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace SliceEngine
{

    public class ScaleWiggleAnimation : SliceBehaviour
    {
        public bool active = true;
        public float rate = 1f;
        private Vector3 storedTransform;
        public Vector3 MaxWiggle = new Vector3(2f, 2f, 2f);
        public Vector3 MinWiggle = new Vector3(1f, 1f, 1f);
        //public float floatingAmp = 10f;
        //public float floatingFreq = 10f;
        private float count = 0f;

        public override void OnCreate()
        {
            base.OnCreate();

            storedTransform = this.transform.Position;
        }

        public override void OnUpdate(float dt)
        {
            base.OnUpdate(dt);
            if (!active)
            {
                return;
            }

            count += dt;

            //float calc = (float)(floatingAmp * Math.Sin( floatingFreq * cycle));

            //transform.Scale =  storedTransform + new Vector3( 0, 0, 0 );

            if (count >= 1/rate)
            {
                count -= 1 / rate;

                transform.Scale = new Vector3(SliceRandom.RangeFloat(MinWiggle.x, MaxWiggle.x), SliceRandom.RangeFloat(MinWiggle.y, MaxWiggle.y), SliceRandom.RangeFloat(MinWiggle.z, MaxWiggle.z));
            }
        }
    }
}
