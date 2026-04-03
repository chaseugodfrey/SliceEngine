using SliceEngine;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Security.Permissions;
using System.Threading;


namespace SliceEngine
{
    public class CollapsingPipe : SliceBehaviour
    {
        public float collapseTime = 3f;

        Vector3 collapsedState = new Vector3(0.01f, 1.0f, 1.0f);

        public override void OnAwake()
        {

        }

        public override void OnUpdate(float dt)
        {
            if (Input.IsKeyPressed(Keys.KEY_C))
            {
                StartCoroutine(Collapse());
            }
        }

        public IEnumerator Collapse()
        {
            float timer = 0.0f;
            while (timer < collapseTime)
            {
                timer += Time.deltaTime;
                transform.Scale = Vector3.Lerp(Vector3.One, collapsedState, timer / collapseTime);
                yield return null;

            }
        }

        public IEnumerator ResetPipe()
        {
            float timer = 0.0f;
            while (timer <= collapseTime)
            {
                timer += Time.deltaTime;
                transform.Scale = Vector3.Lerp(collapsedState, Vector3.One, timer / collapseTime);
                yield return null;
            }
        }
    }
}