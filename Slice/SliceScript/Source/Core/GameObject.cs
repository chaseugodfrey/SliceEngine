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
            Type componentType = typeof(T);
            if (!FunctionCalls.Entity_HasComponent(mID, componentType))
            {
                return null;
            }

            var ctor = componentType.GetConstructor(new[] { typeof(GameObject) });
            if (ctor == null)
                throw new InvalidOperationException(
                    $"Type {componentType.Name} must declare a public constructor {componentType.Name}({nameof(GameObject)})");

            T component = (T)ctor.Invoke(new object[] { this });
            component.gameObject = this;
            return component;
        }

        public T AddComponent<T>() where T : Component
        {
            var ctor = typeof(T).GetConstructor(new[] { typeof(GameObject) });
            if (ctor == null)
                throw new InvalidOperationException(
                    $"Type {typeof(T).Name} must declare a public constructor {typeof(T).Name}({nameof(GameObject)})");

            T component = (T)ctor.Invoke(new object[] { this });
            component.gameObject = this;
            
            return component;
        }

        public T As<T>() where T : Component, new()
        {
            string baseClassName = typeof(T).Name;
            //CMConsole.Log($"Trying to retrieve script instance of {baseClassName} for entity {mID}");
            Object scriptInstance = FunctionCalls.GetScriptInstance(mID, baseClassName);

            if (scriptInstance == null)
            {
                //  CMConsole.Log($"Script instance is null");
                return null;
            }

            return scriptInstance as T;
        }

        public bool Has<T>() where T : Component, new()
        {
            string baseClassName = typeof(T).Name;

            return FunctionCalls.HasScriptInstance(mID, baseClassName);
        }

        public GameObject[] GetAllChildren()
        {
            uint[] entityIDs = FunctionCalls.Entity_GetAllChildren(mID);

            GameObject[] gameObjects = new GameObject[entityIDs.Length];

            for (int i = 0; i < entityIDs.Length; i++)
            {
                gameObjects[i] = new GameObject(entityIDs[i]);
            }

            if (gameObjects.Length == 0)
            {
                return null;
            }

            return gameObjects;
        }

        public GameObject[] FindGameObjectsWithTag(string tag)
        {
            uint[] entityIDs = FunctionCalls.Entity_FindEntitiesWithTag(tag);

            GameObject[] gameObjects = new GameObject[entityIDs.Length];

            for (int i = 0; i < entityIDs.Length; i++)
            {
                gameObjects[i] = new GameObject(entityIDs[i]);
            }

            if (gameObjects.Length == 0)
            {
                return null;
            }

            return gameObjects;
        }

        public GameObject FindGameObjectWithTag(string tag)
        {
            uint entityIDs = FunctionCalls.Entity_FindEntityWithTag(tag);
            GameObject gameObject = new GameObject(entityIDs);

            return gameObject;
        }

        public GameObject CreateGameObject(string prefabName)
        {
            GameObject entity = new GameObject(FunctionCalls.CreateNewGameObject(prefabName));

            return entity;
        }

        public GameObject FindGameObjectWithName(string name)
        {
            uint entityID = FunctionCalls.Entity_FindEntityWithName(name);

            if (entityID == 0)
                return null;

            return new GameObject(entityID);
        }

        public void SetParent(GameObject parent)
        {
            FunctionCalls.Entity_SetParent(mID, parent.mID);
        }

        public GameObject FindGameObjectWithID(uint id)
        {
            uint entityID = FunctionCalls.Entity_FindEntityWithID(id);
            if (entityID == 0)
                return null;

            return new GameObject(entityID);
        }

        public void Destroy()
        {
            if (mID != 0)
            {
                SliceLog.Log("Enemy dying with id: " + mID);
                FunctionCalls.Destroy(mID);
                CoroutineManager.EntityDestroyed(mID);
                return;
            }
            SliceLog.Log("Killing an enemy with 0 id");
            
            //mID = 0;
        }

        public void SetActive(bool input)
        {
            FunctionCalls.Entity_SetActive(mID, input);
        }

        public bool IsActive()
        {
            return FunctionCalls.Entity_IsActive(mID);
        }

        public override bool Equals(object obj)
        {
            if (obj is GameObject other)
            {
                return this.mID == other.mID;
            }
            return false;
        }

        public override int GetHashCode()
        {
            return (int)mID;
        }
        
        public bool isValid()
        {
            if (!FunctionCalls.Entity_IsValid(mID))
            {
                mID = 0;
                return false;
            }

            return true;
        }
        public static bool operator ==(GameObject lhs, GameObject rhs)
        {
            if (ReferenceEquals(lhs, rhs)) return true;
            if (lhs is null || rhs is null) return false;
            return lhs.mID == rhs.mID;
        }

        public static bool operator !=(GameObject lhs, GameObject rhs) => !(lhs == rhs);
    }
}
