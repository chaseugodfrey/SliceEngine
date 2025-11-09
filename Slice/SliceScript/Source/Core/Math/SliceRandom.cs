using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    /// <summary>
    /// Provides an instance of randomness with a separate seed from its static equivelant
    /// </summary>
    public class SliceRandomInstance
    {
        private Random _rng;

        public SliceRandomInstance()
        {
            _rng = new Random();
        }

        public SliceRandomInstance(int seed)
        {
            _rng = new Random(seed);           
        }

        /// <summary>
        /// Sets the seed for the instanced rng system.
        /// </summary>
        public void SetSeed(int seed)
        {
            _rng = new Random(seed);
        }

        /// <summary>
        /// Gets the underlying Random instance.
        /// Read-only; cannot be modified from outside.
        /// </summary>
        public Random RNG
        {
            get { return _rng; } // getter only
        }
    }


    /// <summary>
    /// Provides static methods for generating random numbers of various types.
    /// </summary>
    public static class SliceRandom
    {
        private static Random rng = new Random();

        /// <summary>
        /// Sets the seed for the static rng system.
        /// </summary>
        public static void SetSeed(int seed)
        {
            rng = new Random(seed);
        }

        /// <summary>
        /// Returns a random double value between 0.0 (inclusive) and 1.0 (exclusive).
        /// </summary>
        /// <returns>A double in the range [0.0, 1.0).</returns>
        public static double ValueDouble()
        {
            return rng.NextDouble(); // [0.0, 1.0)
        }
        /// <summary>
        /// Returns a random double value between 0.0 (inclusive) and 1.0 (exclusive).
        /// </summary>
        /// <returns>A double in the range [0.0, 1.0).</returns>
        public static double ValueDouble(SliceRandomInstance instance)
        {
            return instance.RNG.NextDouble(); // [0.0, 1.0)
        }


        /// <summary>
        /// Returns a random float value between 0.0 (inclusive) and 1.0 (exclusive).
        /// </summary>
        /// <returns>A float in the range [0.0, 1.0).</returns>
        public static float ValueFloat()
        {
            return (float)rng.NextDouble(); // [0.0, 1.0)
        }
        /// <summary>
        /// Returns a random float value between 0.0 (inclusive) and 1.0 (exclusive).
        /// </summary>
        /// <returns>A float in the range [0.0, 1.0).</returns>
        public static float ValueFloat(SliceRandomInstance instance)
        {
            return (float)instance.RNG.NextDouble(); // [0.0, 1.0)
        }

        /// <summary>
        /// Returns a random integer between the specified minimum (inclusive) and maximum (exclusive) values.
        /// </summary>
        /// <param name="min">The inclusive lower bound of the random number.</param>
        /// <param name="max">The exclusive upper bound of the random number.</param>
        /// <returns>An integer in the range [min, max).</returns>
        public static int RangeInt(int min, int max)
        {
            return rng.Next(min, max); // [min, max)
        }
        /// <summary>
        /// Returns a random integer between the specified minimum (inclusive) and maximum (exclusive) values.
        /// </summary>
        /// <param name="min">The inclusive lower bound of the random number.</param>
        /// <param name="max">The exclusive upper bound of the random number.</param>
        /// <returns>An integer in the range [min, max).</returns>
        public static int RangeInt(SliceRandomInstance instance, int min, int max)
        {
            return instance.RNG.Next(min, max); // [min, max)
        }

        /// <summary>
        /// Returns a random float between the specified minimum (inclusive) and maximum (exclusive) values.
        /// </summary>
        /// <param name="min">The inclusive lower bound of the random number.</param>
        /// <param name="max">The exclusive upper bound of the random number.</param>
        /// <returns>A float in the range [min, max).</returns>
        public static float RangeFloat(float min, float max)
        {
            return min + (float)rng.NextDouble() * (max - min); // [min, max)
        }
        /// <summary>
        /// Returns a random float between the specified minimum (inclusive) and maximum (exclusive) values.
        /// </summary>
        /// <param name="min">The inclusive lower bound of the random number.</param>
        /// <param name="max">The exclusive upper bound of the random number.</param>
        /// <returns>A float in the range [min, max).</returns>
        public static float RangeFloat(SliceRandomInstance instance, float min, float max)
        {
            return min + (float)instance.RNG.NextDouble() * (max - min); // [min, max)
        }

        /// <summary>
        /// Returns a random double between the specified minimum (inclusive) and maximum (exclusive) values.
        /// </summary>
        /// <param name="min">The inclusive lower bound of the random number.</param>
        /// <param name="max">The exclusive upper bound of the random number.</param>
        /// <returns>A double in the range [min, max).</returns>
        public static double RangeDouble(double min, double max)
        {
            return min + rng.NextDouble() * (max - min); // [min, max)
        }

        /// <summary>
        /// Returns a random double between the specified minimum (inclusive) and maximum (exclusive) values.
        /// </summary>
        /// <param name="min">The inclusive lower bound of the random number.</param>
        /// <param name="max">The exclusive upper bound of the random number.</param>
        /// <returns>A double in the range [min, max).</returns>
        public static double RangeDouble(SliceRandomInstance instance, double min, double max)
        {
            return min + instance.RNG.NextDouble() * (max - min); // [min, max)
        }
    }
}
