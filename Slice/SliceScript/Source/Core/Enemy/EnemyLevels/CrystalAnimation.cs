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

    public class CrystalAnimation : SliceBehaviour
    {
        private float cycle = 0f;
        private Vector3 storedTransform;
        public float floatingAmp = 10f;
        public float floatingFreq = 10f;

        public override void OnCreate()
        {
            base.OnCreate();

            storedTransform = this.transform.Position;
        }

        public override void OnUpdate(float dt)
        {
            base.OnUpdate(dt);

            cycle += dt;

            float calc = (float)(floatingAmp * Math.Sin( floatingFreq * cycle));

            this.transform.Position =  storedTransform + new Vector3( 0, storedTransform.y + calc,0 );
        }
    }
}
