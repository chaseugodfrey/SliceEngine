using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class L2Controller : SliceBehaviour, IInitializable
    {
        // keep track of spawn points for projectile enemies
        public List<GameObject> projectileSpawnPoints = new List<GameObject>();
        // keep track of active projectile enemies
        //public List<GameObject> activeProjectileEnemies = new List<GameObject>();
        // Keep track of which spawn points are currently occupied
        // spawnPoint to occupied
        Dictionary<GameObject, bool> spawnPointOccupied = new Dictionary<GameObject, bool>();
        Dictionary<GameObject, GameObject> projectileEnemyToSpawnPoint = new Dictionary<GameObject, GameObject>();

        // All of this is testing for now
        // I'm not sure whether it'll be in waves, or just checking for X enemies at all times
        // or just periodically spawn them
        public int enemyWaves = 5;
        public int enemiesPerWave = 3;
        public int currWave = 0;
        public float timeBetweenWaves = 5.0f;
        public bool waveDone = false;
        private bool spawningDone = false;
        private float waveTimer = 0.0f;
        public List<GameObject> enemySpawnPoints = new List<GameObject>();
        public List<GameObject> activeEnemies = new List<GameObject>();

        public string projectilePrefabPath;
        public string enemyPrefabPath;

        public void Initialize()
        {

        }

        public override void OnCreate()
        {
            spawnPointOccupied.Clear();
            foreach (GameObject spawnPoint in projectileSpawnPoints)
            {
                Console.WriteLine("Setting spawn occupied to false");
                spawnPointOccupied[spawnPoint] = false;
            }

            debugTest();

            //Bootstrap.CameraController.LockCamera = true;
        }

        public void debugTest()
        {
            int index = 0; 
            foreach (var entry in spawnPointOccupied)
            {
                if (entry.Value == false)
                {
                    Console.WriteLine($"{index} point is unoccupied");
                }
                else
                {
                    Console.WriteLine($"{index} point is occupied");
                }

                index++;
            }
        }

        public override void OnUpdate(float dt)
        {
            //debugTest();
            foreach (GameObject spawnPoint in projectileSpawnPoints)
            {
                //Console.WriteLine($"{spawnPointOccupied.Count}");
                if (spawnPointOccupied.ContainsKey(spawnPoint) && spawnPointOccupied[spawnPoint])
                {
                    // This spawn point is currently occupied, skip it
                   // Console.WriteLine("Spawn point occupied");
                    continue;
                }

                if (projectilePrefabPath.Length == 0)
                {
                    SliceLog.Error("Projectile Prefab Path is empty!");
                    continue;
                }

                GameObject newProjectileEnemy = CreateGameObject(projectilePrefabPath);
               // Console.WriteLine("Creating projectile Enemy");
                //activeProjectileEnemies.Add(newProjectileEnemy);
                spawnPointOccupied[spawnPoint] = true;
                projectileEnemyToSpawnPoint[newProjectileEnemy] = spawnPoint;

                // any set up for projectile enemy goes here
                newProjectileEnemy.GetComponent<Transform>().Position = spawnPoint.GetComponent<Transform>().Position;
            }

            if (!waveDone)
            {               
                if (!spawningDone)
                {
                    for(int i = 0; i < enemiesPerWave; i++)
                    {
                        GameObject spawnPoint = enemySpawnPoints[i];//GetAvailableSpawnPoint();
                        if (spawnPoint == null)
                        {
                            continue;
                        }
                        GameObject newEnemy = CreateGameObject(enemyPrefabPath);
                        activeEnemies.Add(newEnemy);
                        newEnemy.GetComponent<Transform>().Position = spawnPoint.GetComponent<Transform>().Position;
                    }

                    spawningDone = true;
                }
            }
            else
            {
                waveTimer += dt;
                if (waveTimer >= timeBetweenWaves)
                {
                    waveDone = false;
                    spawningDone = false;
                }
            }
        }

        GameObject GetAvailableSpawnPoint()
        {
            foreach(GameObject spawnPoint in enemySpawnPoints)
            {
                bool occupied = false;
                foreach(GameObject enemy in activeEnemies)
                {
                    if (enemy.GetComponent<Transform>().Position.Distance(spawnPoint.GetComponent<Transform>().Position) < 2.0f)
                    {
                        // This spawn point is currently occupied, skip it
                        occupied = true;
                    }
                }

                if (!occupied)
                {
                    return spawnPoint;
                }
            }

            return null;
        }

        public void OnProjectileEnemyDestroyed(GameObject projectileEnemy)
        {
            if (projectileEnemyToSpawnPoint.ContainsKey(projectileEnemy))
            {
                GameObject spawnPoint = projectileEnemyToSpawnPoint[projectileEnemy];
                spawnPointOccupied[spawnPoint] = false;
                projectileEnemyToSpawnPoint.Remove(projectileEnemy);
            }
        }

        public void OnEnemyDestroyed(GameObject enemy)
        {
            if (activeEnemies.Contains(enemy))
            {
                activeEnemies.Remove(enemy);
            }

            if (activeEnemies.Count == 0)
            {
                waveDone = true;
                currWave++;
            }
        }
    }
}
