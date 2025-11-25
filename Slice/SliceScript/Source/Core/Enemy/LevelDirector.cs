using SliceEngine;
using System;
using System.Collections.Generic;


namespace SliceEngine
{

    public class LevelDirector : SliceBehaviour, IInitializable
    {

        private float internalTimer     = 0f;
        public float waitBetweenSpawns  = 5f;
        public int enemyPerSpawn        = 4;

        public string enemyPrefab = "EnemyTest";

        public List<Vector3> spawnPoints = new List<Vector3>();

        private List<Transform> possibleTransforms = new List<Transform>();

        private List<EnemySpawner> enemySpawners = new List<EnemySpawner>();


        public override void OnCreate()
        {          
            enemySpawners.Clear();

            //Look for SpawnPoint

        }

        public override void OnUpdate(float dt)
        {
            SpawnSpawnerEnemies();
        }

        private void SpawnSpawnerEnemies()
        {
            if (CanSpawn())
            {

            }
        }

        private bool CanSpawn()
        {
            internalTimer += Time.deltaTime;
            
            if (internalTimer >= waitBetweenSpawns)
            {
                internalTimer = 0f;
                return true;
            }
            return false;
        }

        public void Initialize()
        { 
        }
        
    }
}
