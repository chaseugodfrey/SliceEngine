using SliceEngine;
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Security.Permissions;


namespace SliceEngine
{
    public class ShieldGeneratorManager : SliceBehaviour
    {

        private EnemyLevel3 enemyController;
        private GameObject[] generators;
        private List<GameObject> generatorsLeft;

        public GameObject Boss;

        int index = 0;

        public override void OnCreate()
        {
            generators = gameObject.FindGameObjectsWithTag("ShieldGenerator");

            if (generators.Length == 0)
            {
                SliceLog.Console("No shield generators found! Make sure to tag them with 'ShieldGenerator' and place them in the scene.");
            }
            else
            {
                SliceLog.Console("Found " + generators.Length + " shield generators.");
                for (int i = 0; i < generators.Length; i++)
                {
                    generators[i].As<ShieldGenerator>().DestroyTrigger += OnGeneratorDestroyed;
                }
            }

            if (Boss != null)
            {
                enemyController = Boss.As<EnemyLevel3>();
            }

            generatorsLeft = new List<GameObject>(generators);
        }

        public bool GenerateShields(int count)
        {
            if (generatorsLeft.Count <= 0)
            {
                SliceLog.Console("No Generators left!");
                return false;
            }

            if (count > generatorsLeft.Count)
                count = generatorsLeft.Count;

            List<GameObject> toGenerate = generatorsLeft;
            Random rnd = new Random();

            for (int i = 0; i < count; i++)
            {
                int randomIndex = rnd.Next(toGenerate.Count);
                GameObject generator = toGenerate[randomIndex];
                generator.As<ShieldGenerator>().GenerateShields();
                toGenerate.RemoveAt(randomIndex);
            }

            return true;
        }

        public void StopAllGenerators()
        {
            foreach (GameObject gen in generatorsLeft)
            {
                gen.As<ShieldGenerator>().StopGenerating();
            }
        }

        public void OnGeneratorDestroyed(GameObject generator)
        {
            generatorsLeft.Remove(generator);
            SliceLog.Console("A generator was destroyed! Remaining: " + (generatorsLeft.Count));

            if (generatorsLeft.Count == 0)
            {
                SliceLog.Console("All generators destroyed! Boss is now vulnerable.");
            }
        }

        public override void OnUpdate(float dt)
        {
            if (Input.IsKeyPressed(Keys.KEY_K))
            {
                if (index < generators.Length)
                {
                    generators[index].As<ShieldGenerator>().TakeDamage(100);
                    SliceLog.Console("Simulating generator destruction for testing.");

                }
            }
        }
    }
}