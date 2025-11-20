using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;

namespace SliceEngine
{ 
    public static class FunctionCalls
    {
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Transform_GetPosition(uint entityID, out Vector3 position);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Transform_SetPosition(uint entityID, ref Vector3 position);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Transform_GetScale(uint entityID, out Vector3 scale);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Transform_SetScale(uint entityID, ref Vector3 scale);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Transform_GetRotation(uint entityID, out Vector3 rotation);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Transform_SetRotation(uint entityID, ref Vector3 rotation);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]                                                                   
        internal extern static void RigidBody_GetVelocity(uint entityID, out Vector3 rotation);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void RigidBody_SetVelocity(uint entityID, ref Vector3 rotation);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void RigidBody_AddForce(uint entityID, out Vector3 force, int mode);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string Audio_GetSoundName(uint entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Audio_Play(uint entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Audio_Stop(uint entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Audio_IsPlaying(uint entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Audio_SetPaused(uint entityID, bool paused);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Audio_GetPaused(uint entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Audio_SetLoop(uint entityID, bool loop);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Audio_GetLoop(uint entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Audio_SetVolume(uint entityID, float volume);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Audio_GetVolume(uint entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Audio_SetPitch(uint entityID, float pitch);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Audio_GetPitch(uint entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Audio_SetSpatialBlend(uint entityID, float blend);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Audio_GetSpatialBlend(uint entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Audio_SetPan(uint entityID, float pan);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Audio_GetPan(uint entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Audio_SetMute(uint entityID, bool mute);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Audio_GetMute(uint entityID);

        //[MethodImplAttribute(MethodImplOptions.InternalCall)]
        //internal extern static void Audio_SetSoundName(uint entityID, ref Audio audioName);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool IsKeyPressed(Keys key);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool IsKeyDown(Keys key);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Log(string msg);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void LogWarn(string msg);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void LogError(string msg);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Entity_HasComponent(uint entityID, Type componentType);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void ChangeAnim(uint entityID, uint animID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static uint[] Entity_FindEntitiesWithTag(string tag);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static uint Entity_FindEntityWithName(string name);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static uint CreateNewGameObject(string prefabName);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static uint CloneGO(string prefabName);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static object GetScriptInstance(uint entityID, string baseClassName);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool HasScriptInstance(uint entityID, string baseClassName);

        //[MethodImplAttribute(MethodImplOptions.InternalCall)]
        //internal extern static void Load_Scene(Scene scene);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SetBool(uint entity, string parameter, bool val);
        
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SetInt(uint entity, string parameter, int val);
        
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SetFloat(uint entity, string parameter, float val);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string GetCurrAnimName(uint entity);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool IsCurrAnimFin(uint entity);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Destroy(uint entity);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Entity_SetTag(uint entity, string tag);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string Entity_GetTag(uint entity);
    }
}
