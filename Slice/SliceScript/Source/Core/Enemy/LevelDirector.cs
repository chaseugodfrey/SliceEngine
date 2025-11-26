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
        public float randomRadius       = 1f;

        public string spawnTags = "Spawn Location";

        public string enemyPrefab = "EnemyTest";

        ///public List<Vector3> spawnPoints = new List<Vector3>();

        private List<Transform> possibleTransforms = new List<Transform>();

        private List<EnemySpawner> enemySpawners = new List<EnemySpawner>();


        public override void OnCreate()
        {          
            enemySpawners.Clear();
            //enemySpawners.Add(gameObject.FindGameObjectWithName("EnemySpawner").GetComponent<EnemySpawner>());
            //
            SetUpSpawnLocations();
            //Look for SpawnPoint

        }

        private void SetUpSpawnLocations()
        {
           GameObject[] temp =  gameObject.FindGameObjectsWithTag(spawnTags);

            foreach(GameObject local in temp)
            {
                possibleTransforms.Add(local.GetComponent<Transform>());
            }
        }


        public override void OnUpdate(float dt)
        {
            SpawnSpawnerEnemies();
        }

        private void SpawnSpawnerEnemies()
        {
            if (CanSpawn())
            {
                foreach (EnemySpawner spawner in enemySpawners)
                {
                    for (int i = 0; i < enemyPerSpawn; i++)
                    {
                        GameObject just = CreateGameObject(enemyPrefab);
                        Vector3 ran = Utilities.RandomInsideSphere(randomRadius);
                        just.GetComponent<Transform>().Position = spawner.transform.Position + new Vector3(ran.x, 0, ran.z);
                    }
                }
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
