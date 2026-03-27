using SliceEngine;

namespace SliceEngine
{
    public class OrbitalLaser : SliceBehaviour
    {
        public int damage = 20;
        private bool hasHitPlayer = false;


        public void DamagePlayer(GameObject hit)
        {

            SliceLog.Log("Damage player called for Orbital lASER ALOY");

            if (hit.Has<PlayerController>() && hit.As<PlayerController>() == Bootstrap.Player)
            {
                SliceLog.Log("Player is hit");
                Bootstrap.Player.TakeDamage(damage, this.gameObject);


                AudioSettings.PlaySFX("PlayerHitLazer");

                // CreateGameObject("Prefabs/FX_Hit.prefab").GetComponent<Transform>().Position = transform.Position;
            }
        }
        public override void OnTriggerEnter(uint other)
        {
            SliceLog.Log("TRIGGER COLLIDE Orbital lASER ALOY");
            GameObject collidedGO = FindGameObjectWithID(other);
            if (collidedGO != null && collidedGO.tag == "Player")
            {
                DamagePlayer(collidedGO);
            }
        }
    }
}