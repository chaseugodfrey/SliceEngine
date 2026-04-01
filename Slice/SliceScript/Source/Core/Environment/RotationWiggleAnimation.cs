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

    public class RotationWiggleAnimation : SliceBehaviour
    {
        public bool active = true;
        public float rate = 1f;
        private Vector3 storedTransform;
        public Vector3 RotationAxis = new Vector3(0f, 1f, 0f);
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

                transform.Rotate(SliceRandom.RangeFloat(0f, 360f), RotationAxis);

                //transform.Scale = new Vector3(SliceRandom.RangeFloat(0f, MaxWiggle.x), SliceRandom.RangeFloat(MinWiggle.y, MaxWiggle.y), SliceRandom.RangeFloat(MinWiggle.z, MaxWiggle.z));
            }
        }
    }
}
