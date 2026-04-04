using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
namespace SliceEngine
{
    public class CameraAnimationTriggerBox : SliceBehaviour
    {
        /// <summary>
        /// Used only for the starting camera sequence when the player enters level 1
        /// </summary>
        public class IntroState : BaseState
        {
            private CameraAnimationTriggerBox animationTriggerBox;
            private CameraController camControl;
            private bool dialogueDone = false;
            private bool dialogueStarted = false;
            private bool keyPressed = false;
            
            public IntroState(GameObject owner) : base(owner)
            {
                animationTriggerBox = owner.As<CameraAnimationTriggerBox>();
            }

            public override void OnEnter()
            {

                // this triggers the camera moving to where its suppose to be when the player lands
                camControl = animationTriggerBox.camera.As<CameraController>();

                if (camControl != null && Bootstrap.Player != null)
                {
                    animationTriggerBox.StartCoroutine(LerpToPlayerPosition());
                }
            }

            public override void OnUpdate(float dt)
            {
               // Console.WriteLine($"On update {dialogueDone} and {dialogueStarted}");
                if (!dialogueDone && dialogueStarted)
                {
                    //Console.WriteLine("Dialogue started but not done");
                    if (Input.IsKeyDown(Keys.KEY_F) && !keyPressed)
                    {
                       
                       // Console.WriteLine("Key Pressed");
                        keyPressed = true;
                        if (!Bootstrap.HUDManager.PlayDialogueForLevel(animationTriggerBox.setOfThisTrigger, Bootstrap.HUDManager.currentScene))
                        {
                            animationTriggerBox.cameraSM.ChangeState(animationTriggerBox.exitState);
                            dialogueDone = true;
                        }
                    }

                    if (Input.IsKeyReleased(Keys.KEY_F) && keyPressed)
                    {
                        keyPressed = false;
                    }
                }
            }

            public override void OnExit()
            {
                // this is to turn start dialogue or turn off lock camera depending
            }

            public IEnumerator LerpToPlayerPosition()
            {
                float elapsed = 0f;
                // how long it takes to lerp
                float duration = 5.0f;
                Vector3 startingPos = camControl.transform.Position;
                Quaternion startingRot = camControl.transform.RotationQuat;
                    
                while (elapsed < duration)
                {
                    elapsed += Time.deltaTime;
                    float t = elapsed / duration;

                    float smoothT = t * t * (3f - 2f * t);

                    Vector3 targetPos = Bootstrap.Player.transform.Position;

                    camControl.transform.Position = Vector3.Lerp(startingPos, targetPos, smoothT);

                    Quaternion targetRot = Quaternion.LookRotation(Bootstrap.Player.transform.Forward);
                    camControl.transform.RotationQuat = Quaternion.Slerp(startingRot, targetRot, smoothT);

                    yield return null;
                }
                dialogueStarted = true;
                Bootstrap.HUDManager.PlayDialogueForLevel(animationTriggerBox.setOfThisTrigger, Bootstrap.HUDManager.currentScene);
                //owner.As<CameraAnimationTriggerBox>().cameraSM.ChangeState(owner.As<CameraAnimationTriggerBox>().exitState);
            }
        }

        public class SwordIntroState : BaseState
        {
            private CameraAnimationTriggerBox animationTriggerBox;
            public GameObject cinematicPlayer;
            public GameObject cinematicSword;

            // cause i dont want to modify the original animation trigger box
            // ill have to hard code it here
            public Vector3 startingPos = new Vector3(720.8f, 40.0f,  -651.2f);
            public Vector3 endingPos = new Vector3(723.0f, 30.5f, -490.0f);
            public Vector3 startingRot = new Vector3(180.0f, -3.07f, 180.0f);

            private Coroutine fadeInRoutine = null;
            private Coroutine fadeOutRoutine = null;
            private const float TransitionDuration = 1.0f; // for the black screen fading
            private CameraController camControl;

            public SwordIntroState(GameObject owner) : base(owner)
            {
                animationTriggerBox = owner.As<CameraAnimationTriggerBox>();
                cinematicPlayer = owner.FindGameObjectsWithTag("CinematicPlayer")[0];
                cinematicSword = owner.FindGameObjectsWithTag("CinematicSword")[0];
            }

            public override void OnEnter()
            {
                Bootstrap.CameraController.LockCamera = true;
                //  Bootstrap.Player.PlayerMovementState = PlayerController.MovementState.Falling;
                Bootstrap.Player.SetPlayerLock(true);

                camControl = animationTriggerBox.camera.As<CameraController>();
                if (fadeOutRoutine == null)
                {
                    fadeOutRoutine = animationTriggerBox.StartCoroutine(FadeOutRoutine());
                }
            }

            public override void OnUpdate(float dt)
            {
                
            }

            public override void OnExit()
            {
                
            }

            public IEnumerator LerpToSword()
            {
                float elapsed = 0f;

                Console.WriteLine("Starting Lerp To Sword");
                // how long it takes to lerp
                float duration = 11.0f;
                Vector3 startingPos = camControl.transform.Position;
                //   Quaternion startingRot = camControl.transform.RotationQuat;
                bool animStarted = false;
                while (elapsed < duration)
                {
                    elapsed += Time.deltaTime;
                    float t = elapsed / duration;

                    float smoothT = t * t * (3f - 2f * t);

                    Vector3 targetPos = endingPos;//cinematicSword.GetComponent<Transform>().Position;//Bootstrap.Player.transform.Position;

                    camControl.transform.Position = Vector3.Lerp(startingPos, targetPos, smoothT);
                    if (elapsed >= (duration - 0.2f))
                    {
                        cinematicPlayer.As<PlayerCinematic>().StartCinematicAnimationNoFade();
                        break;
                    }
                    //Quaternion targetRot = Quaternion.LookRotation(Bootstrap.Player.transform.Forward);
                    //camControl.transform.RotationQuat = Quaternion.Slerp(startingRot, targetRot, smoothT);

                    yield return null;
                }
                cinematicSword.Destroy();
            }

            public IEnumerator LookAtSword()
            {
                float elapsed = 0f;
                // how long it takes to lerp
                float duration = 1.0f;
                Quaternion startingQuat = camControl.transform.RotationQuat;

                while (elapsed < duration)
                {
                    elapsed += Time.deltaTime;
                    float t = elapsed / duration;

                    float smoothT = t * t * (3f - 2f * t);

                    Quaternion targetRot = Quaternion.LookRotation(cinematicSword.GetComponent<Transform>().Forward);
                    camControl.transform.RotationQuat = Quaternion.Slerp(startingQuat, targetRot, smoothT);

                    yield return null;
                }

                // when its done moving and looking at the sword
                // exit and start the cinematic cutscene
                animationTriggerBox.cameraSM.ChangeState(animationTriggerBox.exitState);
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

                Console.WriteLine("End of Fade in effect 2222" );
                animationTriggerBox.StartCoroutine(LerpToSword());

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
                    //Console.WriteLine("Stuck in fade out routine");
                    SetRectAlpha(alpha);
                    yield return null;
                }
                SetRectAlpha(1.0f); // Ensure fully black

                camControl.transform.Position = startingPos;
                camControl.transform.Rotation = startingRot;

                fadeOutRoutine = null;
                if (fadeInRoutine == null)
                {
                    Console.WriteLine("Starting Fade in coroutine");
                    fadeInRoutine = animationTriggerBox.StartCoroutine(FadeInRoutine());
                }
                //CoroutineManager.StopAllCoroutines(_transitionRunner);
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



        }

        // transition to this after each state so u can trigger onExit
        public class ExitState : BaseState
        {
            CameraAnimationTriggerBox animationTriggerBox;
            public ExitState(GameObject owner) : base(owner)
            {
                animationTriggerBox = owner.As<CameraAnimationTriggerBox>();
            }

            public override void OnEnter()
            {
                // idk if ppl want to do anything on enter after on exit
                // can do it here
                if (animationTriggerBox.cameraSM.prevState is IntroState)
                {
                    // do stuff here if needed
                }
            }
        }

       public GameObject camera;
        // idk if this should be a reusable thing
        // so this decides what state to run for the camera
        public int startingState = 0;
        public StateMachine cameraSM;
        IntroState introState;
        SwordIntroState swordIntroState;
        public ExitState exitState;
        public int setOfThisTrigger = 0;

        private bool _done = false;
        private bool _enabled = false;

        public override void OnCreate()
        {
            cameraSM = new StateMachine();
            introState = new IntroState(this.gameObject);
            swordIntroState = new SwordIntroState(this.gameObject);
            exitState = new ExitState(this.gameObject);
        }

        public override void OnUpdate(float dt)
        {
            //Console.WriteLine($"Mouse delta {Input.GetMouseDelta().x} and {Input.GetMouseDelta().y}");
            cameraSM.OnUpdate(dt);
            if (_enabled && !_done)
            {
                _done = true;

                switch (startingState)
                {
                    case 0:
                        cameraSM.ChangeState(introState);
                        break;
                    case 1:
                        cameraSM.ChangeState(swordIntroState);
                        break;
                }
            }

        }

        public override void OnFixedUpdate(float dt)
        {
            cameraSM.OnFixedUpdate(dt);
        }

        public override void OnTriggerEnter(uint other)
        {
            GameObject hit = gameObject.FindGameObjectWithID(other);

            if (hit.Has<PlayerController>() && Bootstrap.Player == hit.As<PlayerController>() && !_enabled)
            {
                _enabled = true;
            }
        }
    }
}
