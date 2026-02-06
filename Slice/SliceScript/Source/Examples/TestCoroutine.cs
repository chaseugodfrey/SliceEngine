using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class TestCoroutine : SliceBehaviour
    {
        public override void OnCreate()
        {            
            RunTests();
        }

        public override void OnUpdate(float dt)
        {
            SliceRandom.RangeDouble(0, 0);
        }

        public void RunTests()
        {
            CoroutineManager.StopAllCoroutines();
            StartCoroutine(ParentRoutine());

            for (int i = 0; i < 5; i++)
            {
                //Console.WriteLine($"Frame {i}");
                new CoroutineManager().OnUpdate(0.016f);
            }
        }

        private IEnumerator SimpleRoutine()
        {
            //Console.WriteLine("SimpleRoutine started");

            for (int i = 0; i < 3; i++)
            {
                //Console.WriteLine($"SimpleRoutine step {i}");
                yield return null; // wait one frame
            }

            //Console.WriteLine("SimpleRoutine finished");
        }

        private IEnumerator TimedRoutine()
        {
            //Console.WriteLine("TimedRoutine started");
            yield return new WaitForSeconds(5.0f);
            //Console.WriteLine("TimedRoutine resumed after 5s");
        }

        private IEnumerator ParentRoutine()
        {
            //Console.WriteLine("ParentRoutine started");
            StartCoroutine(ChildRoutine());
            yield return new WaitForSeconds(2.0f);
            //Console.WriteLine("ParentRoutine finished");
        }

        private IEnumerator ChildRoutine()
        {
            //Console.WriteLine("ChildRoutine started");
            yield return new WaitForSeconds(2.0f);
            //Console.WriteLine("ChildRoutine finished");
        }
    }
}
