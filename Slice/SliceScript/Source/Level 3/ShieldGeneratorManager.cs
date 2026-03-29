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
        public List<GameObject> generatorsLeft;
        public List<GameObject> generatorsFunctioning;

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

        public bool StartGenerators(int count)
        {
            if (generatorsLeft.Count <= 0)
            {
                SliceLog.Console("No Generators left!");
                return false;
            }

            SliceLog.Console("Attempting to start " + count + " generators.");

            if (count > generatorsLeft.Count)
                count = generatorsLeft.Count;

            generatorsFunctioning = new List<GameObject>();

            Random rnd = new Random();

            for (int i = 0; i < count; i++)
            {
                int randomIndex = rnd.Next(generatorsLeft.Count);
                GameObject generator = generatorsLeft[randomIndex];
                generator.As<ShieldGenerator>().StartGenerating();
                generatorsFunctioning.Add(generator);
            }

            SliceLog.Console("Started " + count + " generators.");

            return true;
        }

        public bool RegenerateShields()
        {
            if (generatorsFunctioning.Count <= 0)
                return false;

            foreach (GameObject gen in generatorsFunctioning)
            {
                gen.As<ShieldGenerator>().GenerateShield();
            }

            return true;
        }

        public void StopAllGenerators()
        {
            foreach (GameObject gen in generatorsFunctioning)
            {
                gen.As<ShieldGenerator>().StopGenerating();
            }

            generatorsFunctioning.Clear();
        }

        public void OnGeneratorDestroyed(GameObject generator)
        {
            generatorsFunctioning.Remove(generator);
            generatorsLeft.Remove(generator);
            SliceLog.Console("A generator was destroyed! Remaining: " + (generatorsLeft.Count));

            if (generatorsFunctioning.Count == 0)
            {
                SliceLog.Console("All generators stopped.");
            }

            if (generatorsLeft.Count == 0)
            {
                SliceLog.Console("All generators destroyed! Boss is now vulnerable.");
            }
        }

        public override void OnUpdate(float dt)
        {
            if (Input.IsKeyPressed(Keys.KEY_K))
            {
                foreach (GameObject gen in generatorsFunctioning)
                {
                    gen.As<ShieldGenerator>().TakeDamage(1000);
                }
            }
        }
    }
}