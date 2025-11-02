using System;

namespace SliceEngine
{
    public struct Audio
    {
        public string soundName;

        public Audio(string name)
        {
            soundName = name;
        }

        public Audio(Audio name)
        {
            soundName = name.soundName;
        }

        public static Audio empty = new Audio("");

        public static bool operator ==(Audio a, Audio b)
        {
            return a.soundName == b.soundName;
        }

        public static bool operator !=(Audio a, Audio b)
        {
            return !(a==b);
        }

        public override bool Equals(object obj)
        {
            if (obj is Audio other)
            {
                return soundName == other.soundName;
            }
            return false;
        }

        public override int GetHashCode()
        {
            // A common way to combine hash codes on older frameworks
            unchecked // Overflow is fine, just wrap
            {
                int hash = 17;
                hash = hash * 23 + soundName.GetHashCode();
                return hash;
            }
        }

    }
}