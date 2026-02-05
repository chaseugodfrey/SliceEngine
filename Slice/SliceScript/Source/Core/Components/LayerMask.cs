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
        public uint GetMask(string layerName)
        {
            //return the mask 0 if invalid
            return FunctionCalls.LayerMask_GetMask(layerName);
        }

        public string LayerToName(uint layer)
        {
            //return the layer index check returns -1 if invalid
            return FunctionCalls.LayerMask_LayerToName(layer);
        }
        
        public uint NameTolayer(string layerName)
        {
            //return the layer index check returns -1 if invalid
            return FunctionCalls.LayerMask_NameToLayer(layerName);
        }



    }
}