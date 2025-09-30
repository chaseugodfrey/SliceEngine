using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class SliceBehaviour
    {
        public uint mID;

        protected SliceBehaviour()
        {
            mID = 0;
        }
        
        ~SliceBehaviour()
        {
            mID = 0;
        }

        internal SliceBehaviour(uint id)
        {
            mID = id;
        }

        public virtual void OnCreate() { }
        public virtual void OnUpdate(float dt) { }
        //public virtual void OnDestroy() { }
       // public virtual void OnFixedUpdate(float dt) { }

        public Vector3 Position
        {
            get
            {
                Vector3 pos;
                FunctionCalls.Transform_GetPosition(mID, out pos);
                return pos;
            }
            set
            {
                FunctionCalls.Transform_SetPosition(mID, ref value);
            }
        }
    }
}
