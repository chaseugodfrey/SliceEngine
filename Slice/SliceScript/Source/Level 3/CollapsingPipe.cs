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
        public bool isCollapsed = false;
        Vector3 collapsedScale = new Vector3(1.0f, 0.01f, 1.0f);
        public Vector3 initialScale;
        public GameObject PipeStartAudio;



        public override void OnAwake()
        {
            initialScale = transform.Scale;
            transform.Scale = collapsedScale;
        }

        public override void OnFixedUpdate(float dt)
        {
            if (Input.IsKeyPressed(Keys.KEY_C))
            {
                StartCoroutine(Collapse());
            }
        }

        public IEnumerator Collapse()
        {
            var audio = GetComponent<AudioSource>();
            PipeStartAudio.GetComponent<AudioSource>().Play();
            audio.Play();
            float timer = 0.0f;

            while (timer < collapseTime)
            {
                timer += Time.deltaTime;
                transform.Scale = Vector3.Lerp(initialScale, collapsedScale, timer / collapseTime);
                yield return null;
            }

            audio.Stop();
        }

        public IEnumerator ResetPipe()
        {
            var audio = GetComponent<AudioSource>();
            PipeStartAudio.GetComponent<AudioSource>().Play();
            audio.Play();
            float timer = 0.0f;
            while (timer <= collapseTime)
            {
                timer += Time.deltaTime;
                transform.Scale = Vector3.Lerp(collapsedScale, initialScale, timer / collapseTime);
                Bootstrap.CameraController.Shake(0.1f, 0.5f);
                yield return null;
            }
            audio.Stop();
        }
    }
}