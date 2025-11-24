using SliceEngine;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics.Contracts;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class TestParabola : SliceBehaviour
    {
        public override void OnCreate()
        {
            Vector3 offset = new Vector3(5.0f, 0.0f, 5.0f);

            StartCoroutine(Utilities.ParabolaCoroutine(transform, transform.Position, transform.Position + offset, 5.0f, 2.0f));
        }

        public override void OnUpdate(float dt)
        {
            
        }

    }
}
