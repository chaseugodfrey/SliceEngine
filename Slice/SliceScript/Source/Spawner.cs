using System;
using System.Collections;

namespace SliceEngine
{
    public class Spawner : SliceBehaviour
    {
        public float spawnTime = 5.0f;     // delay before wave starts
        public int spawnCount = 4;         // how many enemies per wave
        public string enemyPrefab = "EnemyTest";
        public float spawnInterval = 1.0f; // delay between spawns
        public float[] test4 = new float [5];
        #pragma warning disable CS0414
        private bool spawning = false;
        #pragma warning restore CS0414

        public override void OnCreate()
        {
            StartCoroutine(SpawnLoop());            
            for(int i = 0; i < 5; ++i)
            {
                SliceLog.Log(test4[i].ToString());
            }
        }

        private IEnumerator SpawnLoop()
        {
            while (true)
            {
                // Wait before the next wave
                yield return new WaitForSeconds(spawnTime);

                spawning = true;
                Console.WriteLine("Starting spawn wave...");                
                // Spawn N enemies with interval
                for (int i = 0; i < spawnCount; i++)
                {
                    CreateGameObject(enemyPrefab);
                    yield return new WaitForSeconds(spawnInterval);

                    //Doing this will stop the coroutine after the first spawn
                    //GetComponent<SliceBehaviour>().activeSelf = false;
                }

                spawning = false;
                Console.WriteLine("Wave complete!");
            }
        }
    }
}
