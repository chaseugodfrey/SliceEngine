using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class BaseState
    {
        public GameObject owner;
        public BaseState(GameObject owner)
        {
            this.owner = owner;
        }

        public virtual void OnEnter()
        {

        }

        public virtual void OnUpdate(float dt)
        {

        }

        public virtual void OnFixedUpdate(float dt)
        {

        }

        public virtual void OnExit()
        {

        }
    }
}
