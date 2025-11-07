using System;

namespace SliceEngine
{
    public class GameObject
    {
        public uint mID;

        public GameObject(uint id)
        {
            mID = id;
        }

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

            T component = (T)ctor.Invoke(new object[] { this });
            component.gameObject = this;
            return component;
        }
    }
}
