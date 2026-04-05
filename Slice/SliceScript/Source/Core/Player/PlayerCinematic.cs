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
        public GameObject topBar;
        public GameObject bottomBar;
        public GameObject cinematicSword;

        // public GameObject cameraStartingPos;


        public int animState = 0;
        private Coroutine fadeInRoutine = null;
        private Coroutine fadeOutRoutine = null;
        private bool fadingIn = false;
        private const float TransitionDuration = 1.0f;
        private const float uiDuration = 2.0f;
        private const float fovDuration = 4.0f;
        private Vector3 swordPos; // cause it gets destroyed before i need the pos
        public override void OnCreate()
        {
            //cinematicSM = new StateMachine();
            //startingState = new StartingState(gameObject);
            swordPos = cinematicSword.GetComponent<Transform>().WorldPosition;
            // hide it on start
            ToggleRenderer(false);
            ToggleBagVisibility(false);
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

        public void StartCinematicAnimationNoFade()
        {
            cinematicCamera.SetActive(true);
            // move the camera to where it should be for the cinematic
            //  cinematicCamera.GetComponent<Transform>().Position = cameraStartingPos.GetComponent<Transform>().WorldPosition;
            Camera.SetMainCamera(cinematicCamera);
            GetComponent<Animator>().SetBool("Cinematic", true);
            StartCoroutine(UIAnimation());

            AudioSettings.PlaySFX("03_02_HQ_OurLastShot");
            Bootstrap.HUDManager.PlayDialogueForLevel(69, 2, true, true);
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
                        ToggleBagVisibility(true);
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
                   // GetComponent<Animator>().SetBool("Cinematic2", true);
                      StartCinematicAnimation();
                    // for moving the camera to the next position in the 2nd animation

                    break;
                case "StartFadeOut":
                    {
                        cinematicCamera.GetComponent<Animator>().SetBool("FadeOut", true);
                        StartCoroutine(FOVAnimation());
                        bagObject.Destroy();
                    }
                    break;
                case "End":
                    {
                        animState++;
                        StartCinematicAnimation();
                    }
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
            {
                GetComponent<Animator>().SetBool("Cinematic", true);
                StartCoroutine(UIAnimation());
            }

            if (animState == 1)
            {

            }

            if (animState == 2)
            {
                //Bootstrap.CameraController.LockCamera = false;
                //Bootstrap.Player.SetPlayerLock(false);
            }





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
                //cinematicCamera.GetComponent<Transform>().Position = new Vector3(-0.054f, 1.811f, -1.345f);
                //cinematicCamera.GetComponent<Transform>().Rotation = new Vector3(0.0f, -89.4f, 0.0f);
                cinematicSword.Destroy();

                cinematicCamera.GetComponent<Animator>().SetBool("Idle2", true);

                Bootstrap.HUDManager.PlayDialogueForLevel(69, 2, true, true);

            }

            //ToggleRenderer(true);

             if (animState == 1)
                GetComponent<Animator>().SetBool("Cinematic2", true);

            if (animState == 2)
            {
                //GameObject oldPlayer = Bootstrap.Player.gameObject;
                Bootstrap.Player.gameObject.As<PlayerController>().ChangeModel();

                Bootstrap.Player.gameObject.GetComponent<Transform>().Position = GetComponent<Transform>().WorldPosition;
                Bootstrap.Player.gameObject.GetComponent<Transform>().LookAt(swordPos, Bootstrap.Player.gameObject.GetComponent<Transform>().Up);

                //Bootstrap.ChangePlayer(newPlayer);
                //Bootstrap.Player.GetComponent<Transform>().WorldPosition

                Camera.SetMainCamera(Bootstrap.CameraController.cameraChild);
                Bootstrap.HUDManager.PlayDialogueForLevel(69, 2, true, true);
                Destroy();
            }

            fadeOutRoutine = null;
            if (fadeInRoutine == null)
            {
                Console.WriteLine("Starting Fade in coroutine");
                fadeInRoutine = StartCoroutine(FadeInRoutine());
            }
            //CoroutineManager.StopAllCoroutines(_transitionRunner);
        }

        public IEnumerator UIAnimation()
        {
            float elapsedTime = 0f;
            float startingTopBar = topBar.GetComponent<RectTransform>().Pos_Y;
            float startingBottomBar = bottomBar.GetComponent<RectTransform>().Pos_Y;
            float endingTopBar = startingTopBar - 75.0f;
            float endingBottomBar = startingBottomBar + 75.0f;

            while (elapsedTime < uiDuration)
            {
                elapsedTime += Time.deltaTime;
                float t = Utilities.InverseLerp(0, TransitionDuration, elapsedTime);

                float topPos = Utilities.Lerp(startingTopBar, endingTopBar, t);
                float bottomPos = Utilities.Lerp(startingBottomBar, endingBottomBar, t);

                topBar.GetComponent<RectTransform>().Pos_Y = (int)topPos;
                bottomBar.GetComponent<RectTransform>().Pos_Y = (int)bottomPos;           

                yield return null;
            }

            topBar.GetComponent<RectTransform>().Pos_Y = (int)endingTopBar;
            bottomBar.GetComponent<RectTransform>().Pos_Y = (int)endingBottomBar;

        }
    
        public IEnumerator FOVAnimation()
        {
                float elapsedTime = 0f;
                float startingFOV = cinematicCamera.GetComponent<Camera>().FOV;
                float endingFOV = startingFOV + 5.0f;


                while (elapsedTime < fovDuration)
                {
                    elapsedTime += Time.deltaTime;
                    float t = Utilities.InverseLerp(0, fovDuration, elapsedTime);
    
                    float currentFOV = Utilities.Lerp(startingFOV, endingFOV, t);
                    cinematicCamera.GetComponent<Camera>().FOV = currentFOV;
    
                    yield return null;
                }
    
                cinematicCamera.GetComponent<Camera>().FOV = endingFOV;
        }

        public void ToggleBagVisibility(bool visible)
        {
            GameObject[] children = bagObject.GetAllChildren();
            float alpha = visible ? 1.0f : 0.0f;

            foreach (GameObject child in children)
            {
                if (child.HasComponent<Renderer>())
                {
                    Vector4 col = child.GetComponent<Renderer>().GetColor();
                    col.w = alpha;
                    child.GetComponent<Renderer>().SetColor(col);
                }
            }

        }
    }
}
