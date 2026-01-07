using System;

namespace SliceEngine
{
    public struct LayerMask
    {
        public int value;

        public static implicit operator LayerMask(int value)
        {
            LayerMask mask = new LayerMask();
            mask.value = value;
            return mask;
        }

        public static implicit operator int(LayerMask mask)
        {
            return mask.value;
        }
        public int GetMask(string layerName)
        {
            //return the maskvalue check returns -1 if invalid
            return -1;
        }

        public string LayerToName(int layer)
        {
            //return the layer name check returns null if invalid
            return null;
        }
        
        public int NameTolayer(string layerName)
        {
            //return the layer index check returns -1 if invalid
            return -1;
        }



    }
}