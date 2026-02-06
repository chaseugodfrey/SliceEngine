using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class TestRandomness : SliceBehaviour
    {
        public override void OnCreate()
        {
            // Random doubles
            //Console.WriteLine("Static ValueDouble():");
            //for (int i = 0; i < 5; i++)
            //    Console.WriteLine(SliceRandom.ValueDouble());

            //// Random floats
            //Console.WriteLine("Static ValueFloat():");
            //for (int i = 0; i < 5; i++)
            //    Console.WriteLine(SliceRandom.ValueFloat());

            //// Random ints
            //Console.WriteLine("Static RangeInt(0, 10):");
            //for (int i = 0; i < 5; i++)
            //    Console.WriteLine(SliceRandom.RangeInt(0, 10));

            //// Random float range
            //Console.WriteLine("Static RangeFloat(1.0f, 5.0f):");
            //for (int i = 0; i < 5; i++)
            //    Console.WriteLine(SliceRandom.RangeFloat(1.0f, 5.0f));

            //// Random double range
            //Console.WriteLine("Static RangeDouble(10.0, 20.0):");
            //for (int i = 0; i < 5; i++)
            //    Console.WriteLine(SliceRandom.RangeDouble(10.0, 20.0));

            //Console.WriteLine("=== Testing SliceRandomInstance ===");

            // Create an instance RNG with a seed
            SliceRandomInstance instance = new SliceRandomInstance(123);

            // Random doubles
            //Console.WriteLine("Instance ValueDouble():");
            //for (int i = 0; i < 5; i++)
            //    Console.WriteLine(SliceRandom.ValueDouble(instance));

            //// Random floats
            //Console.WriteLine("Instance ValueFloat():");
            //for (int i = 0; i < 5; i++)
            //    Console.WriteLine(SliceRandom.ValueFloat(instance));

            //// Random ints
            //Console.WriteLine("Instance RangeInt(0, 10):");
            //for (int i = 0; i < 5; i++)
            //    Console.WriteLine(SliceRandom.RangeInt(instance, 0, 10));

            //// Random float range
            //Console.WriteLine("Instance RangeFloat(1.0f, 5.0f):");
            //for (int i = 0; i < 5; i++)
            //    Console.WriteLine(SliceRandom.RangeFloat(instance, 1.0f, 5.0f));

            //// Random double range
            //Console.WriteLine("Instance RangeDouble(10.0, 20.0):");
            //for (int i = 0; i < 5; i++)
            //    Console.WriteLine(SliceRandom.RangeDouble(instance, 10.0, 20.0));
        }
    }
}
