using SliceEngine;
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Security.Permissions;


namespace SliceEngine
{
    public class ShieldGeneratorManager : SliceBehaviour
    {

        private int numberOfGenerators = 0;
        private EnemyLevel3 enemyController;
        private GameObject[] generators;

        public GameObject Boss;

        int index = 0;

        public override void OnCreate()
        {
            generators = gameObject.FindGameObjectsWithTag("ShieldGenerator");
            numberOfGenerators = generators.Length;

            if (numberOfGenerators == 0)
            {
                SliceLog.Console("No shield generators found! Make sure to tag them with 'ShieldGenerator' and place them in the scene.");
            }
            else
            {
                SliceLog.Console("Found " + numberOfGenerators + " shield generators.");
                for (int i = 0; i < generators.Length; i++)
                {
                    generators[i].As<ShieldGenerator>().Destroyedtrigger += OnGeneratorDestroyed;
                }
            }

            if (Boss != null)
            {
                enemyController = Boss.As<EnemyLevel3>();
            }

        }

        public void OnGeneratorDestroyed()
        {
            SliceLog.Console("A generator was destroyed! Remaining: " + (numberOfGenerators - 1));
            numberOfGenerators--;
            if (numberOfGenerators <= 0)
            {
                enemyController.ShieldGeneratorDestroyed();
            }

            // for testing
            index++;
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