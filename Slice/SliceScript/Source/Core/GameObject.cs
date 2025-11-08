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

        public GameObject[] FindGameObjectsWithTag(string tag)
        {
            uint[] entityIDs = FunctionCalls.Entity_FindEntitiesWithTag(tag);

            GameObject[] gameObjects = new GameObject[entityIDs.Length];

            for (int i = 0; i < entityIDs.Length; i++)
            {
                gameObjects[i] = new GameObject(entityIDs[i]);
            }

            return gameObjects;
        }

        public GameObject CreateGameObject(string prefabName)
        {
            GameObject entity = new GameObject(FunctionCalls.CreateNewGameObject(prefabName));

            return entity;
        }

        public GameObject FindGameObjectWithName(string name)
        {
            uint entityID = FunctionCalls.Entity_FindEntityWithName(name);

            return new GameObject(entityID);
        }
    }
}
