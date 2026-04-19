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

        public float lidRiseAmount = 5.0f;
        public float lidMoveDuration = 0.8f;

        public float shakeDuration = 0.2f;
        public float shakeMagnitude = 0.12f;
        public float dipMagnitude = 0.1f;

        private bool isShaking = false;
        private Vector3 shakeOrigin;
        private Vector3 lidClosedPos;

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

            if (toplid != null)
            {
                lidClosedPos = toplid.GetComponent<Transform>().Position;
                StartCoroutine(OpenLid());
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

            base.TakeDamage(1, source);
        }

        protected override void OnDamaged(GameObject source)
        {
            GameObject fx = gameObject.CreateGameObject("Prefabs/FX_TurretMechDamaged.prefab");
            fx.GetComponent<Transform>().Position = Bootstrap.Player.transform.WorldPosition;
            AudioSettings.PlaySFX("SwordHit");
            StartCoroutine(HitShake());
        }

        private IEnumerator HitShake()
        {
            if (!active) yield break;

            if (!isShaking)
            {
                shakeOrigin = transform.Position;
                isShaking = true;
            }

            float elapsed = 0f;
            while (elapsed < shakeDuration && active)
            {
                float x = SliceRandom.RangeFloat(-1f, 1f) * shakeMagnitude;
                float z = SliceRandom.RangeFloat(-1f, 1f) * shakeMagnitude;

                float t = elapsed / shakeDuration;
                float y = t < 0.5f
                    ? -dipMagnitude * (t / 0.5f)
                    : -dipMagnitude * (1f - (t - 0.5f) / 0.5f);

                transform.Position = shakeOrigin + new Vector3(x, y, z);
                elapsed += Time.deltaTime;
                yield return null;
            }

            if (active)
                transform.Position = shakeOrigin;
            isShaking = false;
        }

        public override void OnDeath()
        {
            if (!active) return;

            DestroyTrigger?.Invoke(this.gameObject);
            DestroyShieldGen();
        }

        public void DestroyShieldGen()
        {
            active = false;
            generating = false;
            TurnOnLitGlass(false);

            if (vfx != null)
                vfx.Destroy();

            GetComponent<AudioSource>().Play();
            StartCoroutine(DestroyAfterClose());
        }

        private IEnumerator DestroyAfterClose()
        {
            yield return StartCoroutine(CloseLid());

            GameObject go = gameObject.CreateGameObject("Prefabs/FX_Environment_Sparks.prefab");
            go.GetComponent<Transform>().Position = transform.Position;

            vfx = gameObject.CreateGameObject("Prefabs/FX_ShieldGenDischarge.prefab");
            vfx.GetComponent<Transform>().Position = transform.Position;

            SliceLog.Console("SHIELD GENERATOR DESTROYED");
        }

        public void StartGenerating()
        {
            generating = true;
            hitbox.ComponentEnabled = true;

            vfx = gameObject.CreateGameObject("Prefabs/FX_ShieldGenRecharge.prefab");
            vfx.GetComponent<Transform>().Position = transform.Position;
            vfx.SetParent(gameObject);

            TurnOnLitGlass(true);
            ActivatePipe();
        }

        public void GenerateShield()
        {
            // animation here
        }

        public void StopGenerating()
        {
            generating = false;

            if (vfx != null)
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

        public void ActivatePipe()
        {
            z_PipeScript.StartCoroutine(z_PipeScript.ResetPipe());
        }

        private IEnumerator OpenLid()
        {
            if (toplid == null) yield break;

            Transform lidT = toplid.GetComponent<Transform>();
            Vector3 openPos = lidClosedPos + Vector3.Up * lidRiseAmount;
            float elapsed = 0f;

            while (elapsed < lidMoveDuration)
            {
                elapsed += Time.deltaTime;
                lidT.Position = Vector3.Lerp(lidClosedPos, openPos, elapsed / lidMoveDuration);
                yield return null;
            }

            lidT.Position = openPos;
        }

        private IEnumerator CloseLid()
        {
            if (toplid == null) yield break;

            Transform lidT = toplid.GetComponent<Transform>();
            Vector3 openPos = lidClosedPos + Vector3.Up * lidRiseAmount;
            float elapsed = 0f;

            while (elapsed < lidMoveDuration)
            {
                elapsed += Time.deltaTime;
                lidT.Position = Vector3.Lerp(openPos, lidClosedPos, elapsed / lidMoveDuration);
                yield return null;
            }

            lidT.Position = lidClosedPos;
        }
    }
}
