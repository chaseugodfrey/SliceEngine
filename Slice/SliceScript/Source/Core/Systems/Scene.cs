using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class Scene
    {
        readonly public string name;
        readonly public string path;

        // Not implemented in engine rn
        readonly public int buildIndex;
        readonly public int handle;
        
        readonly public bool isLoaded;  

        public Scene() { }
        public Scene(Scene scene)
        {
            name = scene.name;
            path = scene.path;
            buildIndex = scene.buildIndex;
        }
        public Scene(string name)
        {
            this.name = name;
        }
        public Scene(string _name, int _index)
        {
            name = _name;
            buildIndex = _index;
        }

        public bool isValid()
        {
            return true;
        }

        public static bool operator ==(Scene a, Scene b)
        {
            // Handle null cases
            if (ReferenceEquals(a, b))
                return true;
            if (ReferenceEquals(a, null) || ReferenceEquals(b, null))
                return false;

            // Compare based on identifying data (e.g., name or path)
            return a.name == b.name && a.path == b.path;
        }

        public static bool operator !=(Scene a, Scene b)
        {
            return !(a == b);
        }

        public override bool Equals(object obj)
        {
            if (obj is Scene other)
                return this == other;
            return false;
        }

        public override int GetHashCode()
        {
            // A common way to combine hash codes on older frameworks
            unchecked // Overflow is fine, just wrap
            {
                int hash = 17;
                hash = hash * 23 + name.GetHashCode();
                hash = hash * 23 + path.GetHashCode();
                hash = hash * 23 + buildIndex.GetHashCode();
                hash = hash * 23 + isLoaded.GetHashCode();
                return hash;
            }
        }
    }
}
