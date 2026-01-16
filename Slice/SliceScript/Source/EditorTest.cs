using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class EditorTest : SliceBehaviour
    {
        public bool boolTest = false;
        public GameObject GOtest;
        public Prefab prefabObj = new Prefab("Asset/Test.prefab");

        //public List<int> intList = new List<int>();
        //public List<GameObject> goListTest = new List<GameObject>();
        //public Vector3 vec3test;
        public override void OnUpdate(float dt)
        {

            GOtest = FindGameObjectWithName("Cube");
            int i = 0;

            //foreach (var gameObject in goListTest)
            //{
            //    Console.WriteLine(gameObject.mID);
            //    SliceLog.Console(i);
            //    i++;
            //}
        }
    }
}
