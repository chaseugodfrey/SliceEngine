using System;
using System.Collections;
using System.Collections.Generic;
using System.Text;
using System.Threading.Tasks;
namespace SliceEngine
{
    public class CameraAnimationTriggerBoxShowEnemy : SliceBehaviour
    {
        /// <summary>
        /// I, Ezekiel, copied gideon's code. Overall im not sure how reusable it is. 
        /// I am keeping some structure of it but it seems awfully convoluted in some ways.
        /// </summary>
        public class IntroEnemyState : BaseState
        {
            private CameraAnimationTriggerBoxShowEnemy animationTriggerBox;
            private CameraController camControl;
            private bool dialogueDone = false;
            private bool dialogueStarted = false;
            private bool keyPressed = false;
            
            public IntroEnemyState(GameObject owner) : base(owner)
            {
                animationTriggerBox = owner.As<CameraAnimationTriggerBoxShowEnemy>();
            }

            public override void OnEnter()
            {
                // this triggers the camera moving to where its suppose to be when the player lands
                camControl = animationTriggerBox.camera.As<CameraController>();

                if (camControl != null && Bootstrap.Player != null)
                {
                    animationTriggerBox.StartCoroutine(LerpToEnemyPosition());
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
                            animationTriggerBox.cameraSM.ChangeState(animationTriggerBox.backToPlayerState);
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

            public IEnumerator LerpToEnemyPosition()
            {

                Bootstrap.CameraController.LockCamera = true;
                Bootstrap.Player.SetPlayerLock(true);

                float elapsed = 0f;
                // how long it takes to lerp
                float duration = 2.0f;
                Vector3 startingPos = camControl.transform.Position;
                Quaternion startingRot = camControl.transform.RotationQuat;
                    
                while (elapsed < duration)
                {
                    elapsed += Time.deltaTime;
                    float t = elapsed / duration;

                    float smoothT = t * t * (3f - 2f * t);

                    Vector3 targetPos = animationTriggerBox.enemyToTarget.GetComponent<Transform>().Position;
                    camControl.transform.Position = Vector3.Lerp(startingPos, targetPos, smoothT);

                    Quaternion targetRot = Quaternion.LookRotation(animationTriggerBox.enemyToTarget.GetComponent<Transform>().Backward);
                    camControl.transform.RotationQuat = Quaternion.Slerp(startingRot, targetRot, smoothT);

                    yield return null;
                }
                dialogueStarted = true;
                Bootstrap.HUDManager.PlayDialogueForLevel(animationTriggerBox.setOfThisTrigger, Bootstrap.HUDManager.currentScene);
                //owner.As<CameraAnimationTriggerBox>().cameraSM.ChangeState(owner.As<CameraAnimationTriggerBox>().exitState);
            }
        }

        public class BackToPlayerState : BaseState
        {
            private CameraAnimationTriggerBoxShowEnemy animationTriggerBox;
            private CameraController camControl;
            private bool dialogueDone = false;
            private bool dialogueStarted = false;
            private bool keyPressed = false;

            public BackToPlayerState(GameObject owner) : base(owner)
            {
                animationTriggerBox = owner.As<CameraAnimationTriggerBoxShowEnemy>();
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
                float duration = 2.0f;
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
                //dialogueStarted = true;
                //Bootstrap.HUDManager.PlayDialogueForLevel(animationTriggerBox.setOfThisTrigger, Bootstrap.HUDManager.currentScene);

                Bootstrap.CameraController.LockCamera = false;
                Bootstrap.Player.SetPlayerLock(false);
            }
        }


        // transition to this after each state so u can trigger onExit
        public class ExitState : BaseState
        {
            CameraAnimationTriggerBoxShowEnemy animationTriggerBox;
            public ExitState(GameObject owner) : base(owner)
            {
                animationTriggerBox = owner.As<CameraAnimationTriggerBoxShowEnemy>();
            }

            public override void OnEnter()
            {
                // idk if ppl want to do anything on enter after on exit
                // can do it here
                if (animationTriggerBox.cameraSM.prevState is IntroEnemyState)
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
        IntroEnemyState introState;
        BackToPlayerState backToPlayerState;
        public ExitState exitState;
        public int setOfThisTrigger = 0;

        public GameObject enemyToTarget;

        private bool _done = false;
        private bool _enabled = false;

        public override void OnCreate()
        {
            cameraSM = new StateMachine();
            introState = new IntroEnemyState(this.gameObject);
            backToPlayerState = new BackToPlayerState(this.gameObject);
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
