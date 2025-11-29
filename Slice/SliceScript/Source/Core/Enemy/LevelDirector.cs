using SliceEngine;
using System;
using System.Collections.Generic;
using System.Diagnostics.Contracts;


namespace SliceEngine
{

    public class LevelDirector : SliceBehaviour, IInitializable
    {

        private float internalTimer     = 0f;
        public float waitBetweenSpawns  = 5f;
        public int enemySlimePerSpawn        = 4;
        public float randomRadius       = 1f;

        private bool spawning = true;

        private int currentStage = 0;

        private void IncreaseStage()
        {
            ClearSpawner();
            firstSpawn = true;
            currentStage++;
            SpawnSpawners();
        }

        public string spawnTags = "Spawn Location";

        public string enemySlimePrefabName = "EnemyTest";
        public string spawnerPrefab = "EnemySpawner";

        private List<EnemySpawner> enemySpawners = new List<EnemySpawner>();

        private List<List<Transform>> stageTransforms = new List<List<Transform>>();

        public void ClearSpawner()
        {
            foreach(EnemySpawner spawner in enemySpawners)
            {
                spawner.gameObject.Destroy();
            }
            enemySpawners.Clear();
        }


        public override void OnCreate()
        {
        }

        public void Initialize()
        {
            enemySpawners.Clear();
            SetUpSpawnLocations();
            SpawnSpawners();
        }

        //Go through the list of transform for the current stage to spawn spawners
        private void SpawnSpawners()
        {
            //SliceLog.Console("[[LEVEL DIRECTOR]] SPAWN SPAWNER IS CALLED, stage tranform length at: ", stageTransforms.Count);
            foreach (Transform local in stageTransforms[currentStage])
            {
                SliceLog.Console("looping through stageTrasnforms");
                GameObject just = CreateGameObject(spawnerPrefab);
                just.GetComponent<Transform>().Position = local.Position;
                SliceLog.Console("Just spawned has been set to ", just.GetComponent<Transform>().Position);
                EnemySpawner a = just.As<EnemySpawner>();
                a.StartSpawning();
                enemySpawners.Add(a);
            }
        }

        //Looks for spawnpoints in the level and store their transforms for later spawning
        private void SetUpSpawnLocations()
        {
           GameObject[] temp =  gameObject.FindGameObjectsWithTag(spawnTags);

            foreach(GameObject local in temp)
            {
                if (local.Has<EnemySpawner>())
                {
                    SliceLog.Console("Enemy Spawner found at object ID:", local.mID);
                    EnemySpawner spwn = local.As<EnemySpawner>();

                    if (spwn.stage == -1)
                    {
                        continue;
                    }

                    if (spwn.stage >= stageTransforms.Count) // If index 0, the 
                    {
                        stageTransforms.Add(new List<Transform>());
                    }
                    else
                    {
                        stageTransforms[spwn.stage].Add(new Transform(local));
                    }
                    //possibleTransforms.Add(new Transform(local));
                }
            }

            for (int i = temp.Length -1; i >= 0; i--)
            {
                temp[i].Destroy();
            }
        }


        public override void OnUpdate(float dt)
        {
            if (spawning)
            { SpawnSpawnerEnemies(); }

            if (Input.IsKeyDown(Keys.KEY_0))
            {
                IncreaseStage();
            }
        }

        private void SpawnSpawnerEnemies()
        {
            if (CanSpawn())
            {
                foreach (EnemySpawner spawner in enemySpawners)
                {
                    if (!spawner.IsSpawning)
                    {
                        continue;
                    }

                    for (int i = 0; i < enemySlimePerSpawn; i++)
                    {
                        GameObject just = CreateGameObject(enemySlimePrefabName);
                        Vector3 ran = Utilities.RandomInsideSphere(randomRadius);
                        just.GetComponent<Transform>().Position = spawner.transform.Position + new Vector3(ran.x, 0, ran.z);
                        if (just.Has<EnemySlime>())
                        {
                            Console.WriteLine("THERE IS A SLIME COMPONENT BUT LETS SEE IF IT CRASHESSs");
                            SliceLog.Console("Enemy Slime component found");
                            just.As<EnemySlime>().SetUp();
                        }
                        else
                        {
                            SliceLog.Console("No Enemy Sline component found");
                        }
                    }
                }
            }
        }

        private bool firstSpawn = true;

        private bool CanSpawn()
        {
            internalTimer += Time.deltaTime;

            //if (firstSpawn == true)
            //{
            //    firstSpawn = false;
            //    return true;
            //}

            if (internalTimer >= waitBetweenSpawns)
            {
                internalTimer = 0f;
                return true;
            }
            return false;
        }

        
    }
}
