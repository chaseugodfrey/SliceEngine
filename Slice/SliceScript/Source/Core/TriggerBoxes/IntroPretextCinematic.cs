using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
namespace SliceEngine
{
    public class IntroPretextCinematic : SliceBehaviour
    {

        public class PretextState : BaseState
        {
            private IntroPretextCinematic owner;

            private Coroutine fadeInRoutine = null;
            private Coroutine fadeOutRoutine = null;
            private const float TransitionDuration = 1.0f; // for the black screen fading
            private CameraController camControl;

            public PretextState(GameObject owner) : base(owner)
            {
                this.owner = owner.As<IntroPretextCinematic>();
            }

            public override void OnEnter()
            {
                //cinematicPlayer = base.owner.FindGameObjectsWithTag("CinematicPlayer")[0];

                Bootstrap.CameraController.LockCamera = true;
                //  Bootstrap.Player.PlayerMovementState = PlayerController.MovementState.Falling;
                Bootstrap.Player.SetPlayerLock(true);

                camControl = owner.camera.As<CameraController>();
                if (fadeInRoutine == null)
                {
                    fadeInRoutine = owner.StartCoroutine(FadeInRoutine());
                }

                Bootstrap.HUDManager.PlayDialogueForLevel(0,0, true, true);
            }

            public override void OnUpdate(float dt)
            {
                
            }

            public override void OnExit()
            {
                //Bootstrap.HUDManager.PlayDialogueForLevel(69, 2, true, true);
            }

            public IEnumerator LerpThrough()
            {
                float elapsed = 0f;

                Console.WriteLine("Starting Lerp To Sword");
                // how long it takes to lerp
                float duration = 28.0f;
                Vector3 startingPos = camControl.transform.Position;
                Quaternion startingRot = camControl.transform.RotationQuat;
                bool animStarted = false;
                while (elapsed < duration)
                {
                    elapsed += Time.deltaTime;
                    float t = elapsed / duration;

                    float smoothT = t * t * (3f - 2f * t);

                    Vector3 targetPos = owner.EndPos.GetComponent<Transform>().Position;        
                    camControl.transform.Position = Vector3.Lerp(startingPos, targetPos, smoothT);

                    Quaternion targetRot = Quaternion.LookRotation(Bootstrap.Player.transform.Forward);
                    camControl.transform.RotationQuat = Quaternion.Slerp(startingRot, targetRot, smoothT);


                    if (elapsed >= duration)
                    {

                        if (fadeOutRoutine == null)
                        {
                            //fadeOutRoutine = owner.StartCoroutine(FadeOutRoutine());
                        }

                        Bootstrap.LevelDirector.LoadNextLevel();

                        break;
                    }
                    if (elapsed >= owner.phrase1End && owner.phrase1Ended == false)
                    {
                        owner.phrase1Ended = true;
                        Bootstrap.HUDManager.PlayDialogueForLevel(0, 0, true, true);
                    }
                    if (elapsed >= owner.phrase2End && owner.phrase2Ended == false)
                    {
                        owner.phrase2Ended = true;
                        Bootstrap.HUDManager.PlayDialogueForLevel(0, 0, true, true);
                    }
                    if (elapsed >= owner.phrase3End && owner.phrase3Ended == false)
                    {
                        owner.phrase3Ended = true;
                        Bootstrap.HUDManager.PlayDialogueForLevel(0, 0, true, true);
                    }
                    if (elapsed >= owner.phrase4End && owner.phrase4Ended == false)
                    {
                        owner.phrase4Ended = true;
                        Bootstrap.HUDManager.PlayDialogueForLevel(0, 0, true, true);
                    }
                    yield return null;
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

                camControl.transform.Position = owner.StartPos.GetComponent<Transform>().Position;
                camControl.transform.Rotation = owner.StartPos.GetComponent<Transform>().Rotation;

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
                owner.StartCoroutine(LerpThrough());

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



                fadeOutRoutine = null;
                if (fadeInRoutine == null)
                {
                    Console.WriteLine("Starting Fade in coroutine");
                    //fadeInRoutine = owner.StartCoroutine(FadeInRoutine());
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
                //if (animationTriggerBox.cameraSM.prevState is IntroState)
                //{
                //    // do stuff here if needed
                //}
            }
        }

       public GameObject camera;
        // idk if this should be a reusable thing
        // so this decides what state to run for the camera
        //public int startingState = 0;
        public StateMachine cameraSM;
        PretextState preTextState;
        public ExitState exitState;


        public GameObject StartPos;
        public GameObject EndPos;
        public float phrase1End = 4f;
        private bool phrase1Ended = false;
        public float phrase2End = 11f;
        private bool phrase2Ended = false;
        public float phrase3End = 13f;
        private bool phrase3Ended = false;
        public float phrase4End = 20f;
        private bool phrase4Ended = false;

        private bool _done = false;
        private bool _enabled = true;

        public override void OnCreate()
        {
            cameraSM = new StateMachine();
            exitState = new ExitState(this.gameObject);
            preTextState = new PretextState(this.gameObject);
        }

        public override void OnUpdate(float dt)
        {
            //Console.WriteLine($"Mouse delta {Input.GetMouseDelta().x} and {Input.GetMouseDelta().y}");
            cameraSM.OnUpdate(dt);
            if (!_done)
            {
                _done = true;
                cameraSM.ChangeState(preTextState);
                
            }

        }

        public override void OnFixedUpdate(float dt)
        {
            cameraSM.OnFixedUpdate(dt);
        }
    }
}
