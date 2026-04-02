using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class PlayerCinematic : SliceBehaviour
    {
        public GameObject bagObject;
        public GameObject cinematicCamera;
       // public GameObject cameraStartingPos;


        public int animState = 0;
        private Coroutine fadeInRoutine = null;
        private Coroutine fadeOutRoutine = null;
        private bool fadingIn = false;
        private const float TransitionDuration = 1.0f;

        public override void OnCreate()
        {
            //cinematicSM = new StateMachine();
            //startingState = new StartingState(gameObject);

            // hide it on start
            ToggleRenderer(false);
        }

        public void StartCinematicAnimation()
        {
            if (fadeOutRoutine == null)
            {
                Console.WriteLine("Starting fade out coroutine");
                fadeOutRoutine = StartCoroutine(FadeOutRoutine());
               // fadingIn = true;
            }
        }

        public override void OnUpdate(float dt)
        {
            // if fading in is done, start the cinematic
            //if (fadingIn && fadeInRoutine.isActive == false)
            //{

            //    // start coroutine of the black bars ontop and bottom

            //    fadingIn = false;
            //}
        }

        public void ToggleRenderer(bool toRender)
        {
            
            GameObject[] children = gameObject.GetAllChildren();

            foreach(GameObject child in children)
            {
                if (child.HasComponent<Renderer>())
                {
                    if (toRender)
                    {
                        child.SetActive(true);
                        child.GetComponent<Renderer>().enabled = true;
                    }
                    else
                    {
                        child.SetActive(false);
                        child.GetComponent<Renderer>().enabled = false;
                    }
                }
            }


        }

        public void AnimationState(string state)
        {
            switch (state)
            {
                case "ToRender":
                    {
                        ToggleRenderer(true);
                    }
                    break;
                case "First":
                    if (bagObject.HasComponent<RigidBody>())
                    {
                        bagObject.GetComponent<RigidBody>().gravityFactor = 1.0f;
                    }
                    // for the bag dropping
                break;
                case "Second":
                    animState++;
                    StartCinematicAnimation();
                    // for moving the camera to the next position in the 2nd animation

                break;
            }
        }

        private void SetRectAlpha(float alpha)
        {
            if (SceneManager._transitionRenderer != null)
            {

                Vector4 color = SceneManager._transitionRenderer.Colour;
                color.w = alpha;
                SceneManager._transitionRenderer.Colour = color;
            }
        }

        public IEnumerator FadeInRoutine()
        {
            float elapsedTime = 0f;
            if (SceneManager._transitionRenderer != null)
            {
                SceneManager._transitionRenderer.SetEnabled(true);
            }
            SetRectAlpha(1.0f); // Start black

            while (elapsedTime < TransitionDuration)
            {
                elapsedTime += Time.deltaTime;
                float t = Utilities.InverseLerp(0, TransitionDuration, elapsedTime);
                float alpha = Utilities.Lerp(1.0f, 0.0f, t); // 1 -> 0

                SetRectAlpha(alpha);
                yield return null;
            }

            Console.Write("End of fade in routine");
            SetRectAlpha(0.0f); // Ensure fully transparent

            if (SceneManager._transitionRenderer != null)
            {
                SceneManager._transitionRenderer.SetEnabled(false);
            }

            if (animState == 0)
                GetComponent<Animator>().SetBool("Cinematic", true);


            fadeInRoutine = null;
            
        }

        public IEnumerator FadeOutRoutine()
        {
            float elapsedTime = 0f;
            if (SceneManager._transitionRenderer != null) SceneManager._transitionRenderer.SetEnabled(true);
            SetRectAlpha(0.0f); // Start transparent

            while (elapsedTime < TransitionDuration)
            {
                elapsedTime += Time.deltaTime;
                float t = Utilities.InverseLerp(0, TransitionDuration, elapsedTime);
                float alpha = Utilities.Lerp(0.0f, 1.0f, t); // 0 -> 1
                Console.WriteLine("Stuck in fade out routine");
                SetRectAlpha(alpha);
                yield return null;
            }
            SetRectAlpha(1.0f); // Ensure fully black

            cinematicCamera.SetActive(true);
            // move the camera to where it should be for the cinematic
            //  cinematicCamera.GetComponent<Transform>().Position = cameraStartingPos.GetComponent<Transform>().WorldPosition;
            Camera.SetMainCamera(cinematicCamera);
            Console.WriteLine("End of Fade out coroutine");

            if (animState == 1)
            {
                // move the camera to where its behind the player for 2nd animation
                cinematicCamera.GetComponent<Transform>().Position = new Vector3(-0.054f, 1.811f, -1.345f);
                cinematicCamera.GetComponent<Transform>().Rotation = new Vector3(0.0f, -89.4f, 0.0f);
            }

            //ToggleRenderer(true);

             if (animState == 1)
                GetComponent<Animator>().SetBool("Cinematic2", true);

            fadeOutRoutine = null;
            if (fadeInRoutine == null)
            {
                Console.WriteLine("Starting Fade in coroutine");
                fadeInRoutine = StartCoroutine(FadeInRoutine());
            }
            //CoroutineManager.StopAllCoroutines(_transitionRunner);
        }


    }
}
