using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class Level3Boss : EnemyBase
    {
        #region States
        public class IntroState : BaseState
        {
            /// <summary>
            /// Idk if you want to do any intro animation cutscene
            /// can probably handle it here
            /// </summary>
            Level3Boss bossController;
            public IntroState(GameObject owner) : base(owner)
            {
                bossController = owner.As<Level3Boss>();
            }

            public override void OnEnter()
            {
                
            }

            public override void OnUpdate(float dt)
            {
                
            }

            public override void OnExit()
            {
                
            }
        }

        public class RechargingState : BaseState
        {
            Level3Boss bossController;
            public RechargingState(GameObject owner) : base(owner)
            {
                bossController = owner.As<Level3Boss>();
            }

            public override void OnEnter()
            {
                
            }

            public override void OnUpdate(float dt)
            {
                
            }

            public override void OnExit()
            {
                
            }
        }

        public class ReferenceState : BaseState
        {
            Level3Boss bossController;
            public ReferenceState(GameObject owner) : base(owner)
            {
                bossController = owner.As<Level3Boss>();
            }

            public override void OnEnter()
            {

            }

            public override void OnUpdate(float dt)
            {

            }

            public override void OnFixedUpdate(float dt)
            {

            }

            public override void OnExit()
            {
                
            }
        }

        #endregion

        // make it public so u can access it in the states as well

        // declare all states here 
        public IntroState introState;
        public RechargingState rechargingState;
        public ReferenceState referenceState;

        // state machine for the boss
        public StateMachine bossSM;

        public override void OnCreate()
        {
            // you have to set the state using
            //bossSM.ChangeState(introState);
            
        }

        public override void OnUpdate(float dt)
        {
            // you have to call on update if u want the onUpdate to run
            bossSM.OnUpdate(dt);
        }

        public override void OnFixedUpdate(float dt)
        { 
            // if u want fixed update calls in state machine
            bossSM.OnFixedUpdate(dt);
        }

        
    }
}
