using System;


namespace SliceEngine
{
    public class Spawner : SliceBehaviour
    {
        public float spawnTime = 5.0f;
        public int spawnCount = 4;
        public string enemyPrefab = "GameObject_1";
        float spawnTimer = 0.0f;
        float internalSpawnTimer = 0.0f;
        int spawnCounter = 0; 
        bool enemySpawn = false;
        public override void OnCreate()
        {
            
        }

        public override void OnUpdate(float dt)
        {
            // theres probably a btr way to do this
            // but im kinda lazy to think abt it now
            // mayb see if hafis can change spawning to a coroutine or smth

            spawnTimer += dt;

            if (spawnTimer  > spawnTime && enemySpawn == false)
            {
                enemySpawn = true;
                spawnCounter = 0;
                
            }

            if (enemySpawn)
            {
                internalSpawnTimer += dt;

                if (internalSpawnTimer > 1.0f)
                {
                    CreateGameObject(enemyPrefab);
                    internalSpawnTimer = 0.0f;
                    spawnCounter++;
                }

                if (spawnCounter >= spawnCount)
                {
                    enemySpawn = false;
                    spawnTimer = 0.0f;
                }
            }
        }
    }
}
