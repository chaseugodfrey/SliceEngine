using SliceEngine;
using SliceScript.Source.Core;
using System;
using System.Collections.Generic;
using System.Linq;

namespace SliceEngine
{
    public class L1ArenaSpawner : SliceBehaviour
    {
        GameObject progressionBarrier;

        public float moveDuration = 2f;

        public int maxEnemies = 4;
        public List<GameObject> spawnPoints;
        List<GameObject> remainingEnemies;

        public bool begun = false;
        private int currentSpawnIndex = 0;

        public override void OnCreate()
        {
            progressionBarrier = FindGameObjectWithName("Arena_1_Barrier");
            remainingEnemies = new List<GameObject>();

            spawnPoints = new List<GameObject>();
            GameObject[] spawnsArray = FindGameObjectsWithTag("Arena_1_Spawn");
            spawnPoints = new List<GameObject>(spawnsArray);
        }

        public override void OnUpdate(float dt)
        {
            if (begun && (remainingEnemies.Count == 0))
            {
                progressionBarrier.Destroy();
                begun = false;
                SliceLog.Log("All enemies defeated. Progression barrier destroyed.");
            }
        }

        public override void OnCollideEnter(uint other)
        {
            SliceLog.Log("OnCollideEnter triggered. Other ID: " + other);

            if (other == Bootstrap.Player.gameObject.mID)
            {
                SliceLog.Log("Player collided. Beginning enemy spawn...");

                for (int i = 0; i < maxEnemies; ++i)
                {
                    SliceLog.Log("Spawning enemy #" + i);
                    GameObject enemy = CreateGameObject("Prefabs/EnemyGrunt.prefab");

                    if (enemy == null)
                    {
                        SliceLog.Log("ERROR: enemy is null after CreateGameObject");
                        continue;
                    }

                    SliceLog.Log("Enemy created. ID: " + enemy.mID);

                    if (enemy.mID == 0)
                    {
                        SliceLog.Log("ERROR: enemy ID is 0, skipping.");
                        continue;
                    }

                    // Check spawnPoints list
                    if (spawnPoints == null || spawnPoints.Count == 0)
                    {
                        SliceLog.Log("ERROR: spawnPoints is null or empty!");
                        break; // nothing to spawn to
                    }

                    SliceLog.Log("Current spawn index: " + currentSpawnIndex);

                    GameObject spawnPoint = spawnPoints[currentSpawnIndex];

                    if (spawnPoint == null)
                    {
                        SliceLog.Log("ERROR: spawnPoint at index " + currentSpawnIndex + " is null!");
                        continue;
                    }

                    var enemyTransform = enemy.GetComponent<Transform>();
                    var spawnTransform = spawnPoint.GetComponent<Transform>();

                    if (enemyTransform == null)
                    {
                        SliceLog.Log("ERROR: enemy Transform is null!");
                        continue;
                    }

                    if (spawnTransform == null)
                    {
                        SliceLog.Log("ERROR: spawnPoint Transform is null!");
                        continue;
                    }

                    SliceLog.Log("Moving enemy to spawn point: " + spawnPoint.mID);
                    enemyTransform.Position = spawnTransform.WorldPosition;

                    remainingEnemies.Add(enemy);
                    SliceLog.Log("Enemy added to remainingEnemies list. Count: " + remainingEnemies.Count);

                    // Increment spawn index safely
                    currentSpawnIndex = (currentSpawnIndex + 1) % spawnPoints.Count;
                    SliceLog.Log("Next spawn index: " + currentSpawnIndex);
                }

                begun = true;
                SliceLog.Log("Enemy spawning loop finished. Begun set to true.");
            }
        }
    }
}
