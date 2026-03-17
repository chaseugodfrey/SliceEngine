using SliceEngine;
using SliceScript.Source.Core;
using System;
using System.Collections.Generic;

namespace SliceEngine
{
    public class L1FallingPillar : SliceBehaviour
    {
        public Transform go;

        public Vector3 startPos;
        public Vector3 endPos;

        public Vector3 startRot;
        public Vector3 endRot;

        public override void OnCreate()
        {
            go = GetComponent<Transform>();
        }

        public override void OnUpdate(float dt)
        {
        }

        public override void OnCollideEnter(uint other)
        {

        }

        public override void OnCollideExit(uint other)
        {

        }
    }
}
