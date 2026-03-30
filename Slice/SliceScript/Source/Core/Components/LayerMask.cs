using System;

namespace SliceEngine
{
    public struct LayerMask
    {
        public uint value; // the actual mask value

        public static implicit operator LayerMask(uint value)
        {
            LayerMask mask = new LayerMask();
            mask.value = value;
            return mask;
        }

        public static implicit operator uint(LayerMask mask)
        {
            return mask.value;
        }

        //get the full bit mask of this layer(hit anything this layer can hit)
        public static uint GetCollisionMask(string layerName)
        {
            //return the mask 0 if invalid
            return FunctionCalls.LayerMask_GetCollisionMask(layerName);
        }

        //just get the bitmask of this individual layer(for example if you put enviroment you can only hit objects with enviroment layer)
        public static uint ToMask(string layerName)
        {
            //return the mask 0 if invalid
            return FunctionCalls.LayerMask_ToMask(layerName);
        }

        public static string LayerToName(uint layer)
        {
            //return the layer index check returns -1 if invalid
            return FunctionCalls.LayerMask_LayerToName(layer);
        }
        
        public static uint NameTolayer(string layerName)
        {
            //return the layer index check returns -1 if invalid
            return FunctionCalls.LayerMask_NameToLayer(layerName);
        }



    }
}