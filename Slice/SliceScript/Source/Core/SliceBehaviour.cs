using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class SliceBehaviour : Component
    {
        public uint mID;
        //public Transform transform;

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

        public virtual void OnAwake() { }
        //public virtual void OnEnable() { }
        public virtual void OnCreate() { }
        public virtual void OnUpdate(float dt) { }
        //public virtual void OnDestroy() { }
        public virtual void OnFixedUpdate(float dt) { }

        public bool HasComponent<T>() where T : Component, new()
        {
            Type componentType = typeof(T);
            return FunctionCalls.Entity_HasComponent(mID, componentType);
        }
        public T GetComponent<T>() where T : Component
        {
            var ctor = typeof(T).GetConstructor(new[] { typeof(SliceBehaviour) });
            if (ctor == null)
                throw new InvalidOperationException(
                    $"Type {typeof(T).Name} must declare a public constructor {typeof(T).Name}({nameof(SliceBehaviour)})");

            return (T)ctor.Invoke(new object[] { this });
        }
    }
}
