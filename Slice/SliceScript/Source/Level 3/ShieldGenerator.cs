using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{
    public class ShieldGenerator : EnemyBase
    {
        public delegate void ShieldGeneratorDestroyedEvent(GameObject gen);
        public event ShieldGeneratorDestroyedEvent DestroyTrigger;

        public bool generating = false;
        private ColliderShape hitbox;
        public GameObject toplid;
        public GameObject glass;
        public GameObject glassLit;
        public GameObject light;
        public GameObject vfx;

        public GameObject z_Pipe;
        CollapsingPipe z_PipeScript;

        //Function called when you want the enemy to be active
        public override void OnCreate()
        {
            enemyT = GetComponent<Transform>();
            hitbox = GetComponent<ColliderShape>();
            active = true;

            GameObject[] children = gameObject.GetAllChildren();
            foreach (GameObject child in children)
            {
                if (child.tag == "GenTopLid")
                    toplid = child;
                else if (child.tag == "GenGlass")
                    glass = child;
                else if (child.tag == "GenGlassLit")
                    glassLit = child;
                else if (child.tag == "GenLight")
                    light = child;
            }

            TurnOnLitGlass(false);

            if (z_Pipe != null)
                z_PipeScript = z_Pipe.As<CollapsingPipe>();
        }

        public override void OnAwake()
        {
            //base.OnAwake();
            maxHealth = 3;
            currentHealth = maxHealth;
        }

        public override void OnUpdate(float dt)
        {
            //base.OnUpdate(dt);
        }

        public override void TakeDamage(int amount, GameObject source = null)
        {
            if (!active) return;
            if (!generating) return;

            SliceLog.Console("SHIELD GENERATOR is taking damage");

            base.TakeDamage(1, source);
        }

        protected override void OnDamaged(GameObject source)
        {
            // put some vfx here
        }

        public override void OnDeath()
        {
            DestroyTrigger?.Invoke(this.gameObject);
            active = false;
            generating = false;
            hitbox.ComponentEnabled = false;
            SliceLog.Console("SHIELD GENERATOR DESTROYED");

            TurnOnLitGlass(false);

            GameObject go = gameObject.CreateGameObject("Prefabs/FX_Environment_Sparks.prefab");
            go.GetComponent<Transform>().Position = transform.Position;

            //Vector3 force = Bootstrap.Player.transform.Position - transform.Position;
            //force = force.Normalize();
            //toplid.GetComponent<RigidBody>().AddForce(force * 10.0f, ForceMode.Impulse);

            if (vfx != null)
                vfx.Destroy();

            vfx = gameObject.CreateGameObject("Prefabs/FX_ShieldGenDischarge.prefab");
            vfx.GetComponent<Transform>().Position = transform.Position;
        }

        public void StartGenerating()
        {
            generating = true;
            hitbox.ComponentEnabled = true;
            vfx = gameObject.CreateGameObject("Prefabs/FX_ShieldGenRecharge.prefab");
            vfx.GetComponent<Transform>().Position = transform.Position;
            vfx.SetParent(gameObject);

            TurnOnLitGlass(true);

            z_PipeScript.StartCoroutine(z_PipeScript.ResetPipe());
        }

        public void GenerateShield()
        {
            // animation here
        }

        public void StopGenerating()
        {
            generating = false;
            hitbox.ComponentEnabled = false;
            vfx.Destroy();
            TurnOnLitGlass(false);

            SliceLog.Console("SHIELD GENERATOR STOPPED GENERATING SHIELDS");
        }

        public void TurnOnLitGlass(bool isOn)
        {
            glass.SetActive(!isOn);
            glassLit.SetActive(isOn);
            light.SetActive(isOn);
        }
    }
}
