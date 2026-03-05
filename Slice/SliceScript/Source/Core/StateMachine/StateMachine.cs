using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{

    public class StateMachine
    {
        BaseState currentState;

        // if needed for any "if prev state is this do this instead" logic
        BaseState prevState;

        public StateMachine() { }
        
        public void ChangeState(BaseState newState)
        {
            if (currentState != null)
            {
                currentState.OnExit();
            }
            prevState = currentState;
            currentState = newState;
            currentState.OnEnter();
        }

        public void OnUpdate(float dt)
        {
            currentState?.OnUpdate(dt);
        }

        public void OnFixedUpdate(float dt)
        {
            currentState?.OnFixedUpdate(dt);
        }
    }
}
