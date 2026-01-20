using System;

namespace SliceEngine
{
    public struct Prefab
    {
        public string prefabName;

        public Prefab(string name)
        {
            prefabName = name;
        }

        public Prefab(Prefab name)
        {
            prefabName = name.prefabName;
        }

        public GameObject Instantiate()
        {
            return new GameObject(FunctionCalls.CreateNewGameObject(prefabName));
        }

        public static Prefab empty = new Prefab("");

        public static bool operator ==(Prefab a, Prefab b)
        {
            return a.prefabName == b.prefabName;
        }

        public static bool operator !=(Prefab a, Prefab b)
        {
            return !(a==b);
        }

        public override bool Equals(object obj)
        {
            if (obj is Prefab other)
            {
                return prefabName == other.prefabName;
            }
            return false;
        }

        public override int GetHashCode()
        {
            // A common way to combine hash codes on older frameworks
            unchecked // Overflow is fine, just wrap
            {
                int hash = 17;
                hash = hash * 23 + prefabName.GetHashCode();
                return hash;
            }
        }

    }
}