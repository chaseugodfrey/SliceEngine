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
    // Example of making something jump continuously
    public class TestParabola : SliceBehaviour
    {
        public override void OnCreate()
        {          
            StartCoroutine(JumpLoop());
        }

        private IEnumerator JumpLoop()
        {
            Vector3 offset = new Vector3(5.0f, 0.0f, 5.0f);
            float howLongJumpShouldTake = 2.0f;

            StartCoroutine(Utilities.ParabolaCoroutine(transform, transform.Position, transform.Position + offset, 5.0f, howLongJumpShouldTake));
            yield return new WaitForSeconds(howLongJumpShouldTake);
            StartCoroutine(JumpLoop());
        }

    }
}
