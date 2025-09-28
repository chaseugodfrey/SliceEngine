using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceScript
{
    public class Entity
    {
        public uint mID;

        protected Entity()
        {
            mID = 0;
        }
        
        ~Entity()
        {
            mID = 0;
        }

        public virtual void OnCreate() { }
        public virtual void OnUdate(float dt) { }
        public virtual void OnDestroy() { }
        public virtual void OnFixedUpdate(float dt) { }
    }
}
