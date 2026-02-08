using SliceEngine;
using System;
using System.Collections;
using System.Diagnostics;
using System.IO.Pipes;


namespace SliceEngine
{
    public class EnemySpawner : Entity
    {

        #region --- Level Director Field ---
        public int stage = 0;
        #endregion


        #region --- Spawning Field ---
        private bool isSpawning = false;

        public bool IsSpawning { get {return isSpawning; } }
        public void StartSpawning() 
        { 
            if (isSpawning != true) 
            { 
                isSpawning = true; //StartCoroutine(Jiggle()); 
            } 
        }
        public void StopSpawning() { isSpawning = false; }
        #endregion

        //private RigidBody rb; 
        //private Renderer

        public void Initialize()
        {

        }

        public override void OnCreate() 
        { 
            /*
            rb = this.GetComponent<RigidBody>(); 
            
            if (rb == null) 
            {
                Console.WriteLine("NO RIGID BODY FOUND FOR ENEMY SPAWNER"); 
            } 
            else 
            {
                Console.WriteLine("RIGID BODY FOUND FOR ENEMY SPAWNER FOUND"); 
            } 
            */


        }

        public override void OnUpdate(float dt)
        {
            //if (Input.IsKeyDown(Keys.KEY_0))
            //{            //if (Input.IsKeyDown(Keys.KEY_0))
            //{
            //if (Input.IsKeyDown(Keys.KEY_9))
            //{
            //    StopSpawning();
            //}
            //if (Input.IsKeyDown(Keys.KEY_8))
            //{
            //    TakeDamage(50, gameObject);
            //}
        }

        /*
        public override void TakeDamage(int amount, GameObject source = null)
        {
            base.TakeDamage(amount, source);
        }
        */
        IEnumerator Jiggle()
        {
            bool jiggleleft = false;

            while(isSpawning == true)
            {
                if (jiggleleft) { this.transform.Position += new Vector3(2, 0, 0); jiggleleft = false; }
                else { this.transform.Position -= new Vector3(2, 0, 0); jiggleleft = true; }

                yield return new WaitForSeconds(Time.deltaTime);
            }
            yield break;
        }




        protected override void OnHeal(){}
        protected override void OnDamaged(GameObject source) 
        { /*rb.AddForce(new Vector3(0, 1, 1), ForceMode.Impulse); */}

        private bool isDead = false;

        public override void OnDeath()
        {
            if (!isDead)
            {
                isDead = true;
                this.gameObject.Destroy();
            }
        }
    }
}