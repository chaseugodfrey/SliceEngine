using SliceEngine;
using System;
using System.Collections;
using System.Collections.Generic;

namespace SliceEngine
{
    public class ShieldGeneratorManager : SliceBehaviour
    {
        public List<GameObject> generatorsLeft;
        public List<GameObject> generatorsFunctioning;

        public GameObject RegeneratorTop;
        public GameObject RegeneratorBot;

        public GameObject Boss;

        int index = 0;

        public override void OnCreate()
        {
            generatorsLeft = new List<GameObject>();
            generatorsFunctioning = new List<GameObject>();

            GameObject[] generators = gameObject.FindGameObjectsWithTag("ShieldGenerator");

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
                    generatorsLeft.Add(generators[i]);
                }
            }
        }

        public override void OnAwake()
        {

        }

        public bool StartGenerators(int count)
        {
            CheckIfGeneratorsLeft();

            SliceLog.Console("generatorsLeft count: " + generatorsLeft.Count);

            if (count > generatorsLeft.Count)
                count = generatorsLeft.Count;

            generatorsFunctioning.Clear();

            TurnOnOffRegenerators(true);

            SliceLog.Console("Attempting to start " + count + " generators.");

            Random rnd = new Random();

            while (count > 0)
            {
                int randomIndex = rnd.Next(generatorsLeft.Count);
                GameObject generator = generatorsLeft[randomIndex];

                if (generatorsFunctioning.Contains(generator))
                    continue;

                generator.As<ShieldGenerator>().StartGenerating();
                generatorsFunctioning.Add(generator);
                --count;
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

        public bool CheckIfGeneratorsLeft()
        {
            if (generatorsLeft.Count <= 0)
            {
                SliceLog.Console("No Generators left!");
                return false;
            }

            return true;
        }

        public void StopAllGenerators()
        {
            TurnOnOffRegenerators(false);

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

        void TurnOnOffRegenerators(bool active)
        {
            RegeneratorTop.As<RotatingEnvironment>().StartOrStopRotating(active, 1f);
            RegeneratorBot.As<RotatingEnvironment>().StartOrStopRotating(active, 1f);
        }
    }
}