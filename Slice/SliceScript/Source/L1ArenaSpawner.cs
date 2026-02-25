using SliceEngine;
using SliceScript.Source.Core;
using System;
using System.Collections.Generic;

namespace SliceEngine
{
    public class L1ArenaSpawner : SliceBehaviour
    {
        public Transform go;

        public Vector3 startOffset;
        public Vector3 endOffset;

        public float moveDuration = 2f;

        public int enemiesRemaining;

        public override void OnCreate()
        {
            go = GetComponent<Transform>();
        }

        public override void OnUpdate(float dt)
        {
            
        }
    }
}
