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

        public override void OnCreate()
        {
            generators = gameObject.FindGameObjectsWithTag("ShieldGenerator");
            numberOfGenerators = generators.Length;

            if (numberOfGenerators == 0)
            {
                SliceLog.Log("No shield generators found! Make sure to tag them with 'ShieldGenerator' and place them in the scene.");
            }
            else
            {
                SliceLog.Log("Found " + numberOfGenerators + " shield generators.");
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
            Console.WriteLine("A generator was destroyed! Remaining: " + (numberOfGenerators - 1));
            numberOfGenerators--;
            if (numberOfGenerators <= 0)
            {
                enemyController.ShieldGeneratorDestroyed();
            }
        }

        public void Update(float dt)
        {
            
        }

    }
}