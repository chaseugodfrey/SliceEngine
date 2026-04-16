using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;
using static SliceEngine.ParticleSystem;

namespace SliceEngine
{
    public static class FunctionCalls
    {
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Time_GetDeltaTimeUnscaled();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Time_GetTimeScale();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Time_SetTimeScale(float time);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Camera_SetMainCamera(uint entityID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Camera_SetEnabled(uint entityID, bool enable);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Camera_GetEnabled(uint entityID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Camera_SetGamma(float gamma);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Camera_GetGamma();
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Camera_GetFOV(uint entityID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Camera_SetFOV(uint entityID, float fov);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Camera_ToggleImpactFrames(uint entityID, bool isEnable);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Camera_SetImpactFrameWorldPosition(uint entityID, ref Vector3 position);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Camera_SetImpactFrameColor1(uint entityID, ref Vector3 position);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Camera_SetImpactFrameColor2(uint entityID, ref Vector3 position);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Camera_SetImpactFrameSmooth(uint entityID, bool isSmooth);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Camera_SetImpactFrameSpeed(uint entityID, float speed);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Camera_SetImpactFrameSharpness(uint entityID, float sharpness);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Camera_SetImpactFrameDensity(uint entityID, float density);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Camera_SetImpactBlend(uint entityID, float blend);
        // Light
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Light_SetEnabled(uint entityID, bool enable);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Light_GetEnabled(uint entityID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Light_SetCastShadow(uint entityID, bool castsShadow);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Light_GetCastShadow(uint entityID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Light_SetColor(uint entityID, ref Vector3 color);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Light_GetColor(uint entityID, out Vector3 color);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Light_SetIntensity(uint entityID, float intensity);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Light_GetIntensity(uint entityID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Light_SetAngle(uint entityID, float angle);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Light_GetAngle(uint entityID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Light_SetLightType(uint entityID, int lightType);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static int Light_GetLightType(uint entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string Application_GetFilePath();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Debug_Console(string[] callStack, string msg, int level);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Scene_LoadScene(string sceneName);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Scene_UnloadCurrentScene();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void QuitGame();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Transform_GetPosition(uint entityID, out Vector3 position);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Transform_GetWorldPosition(uint entityID, out Vector3 position);

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
        internal extern static void Transform_GetRotationQuat(uint entityID, out Quaternion rotation);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Transform_GetWorldRotationQuat(uint entityID, out Quaternion rotation);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Transform_SetRotationQuat(uint entityID, ref Quaternion rotation);

        //Physics
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void RigidBody_GetVelocity(uint entityID, out Vector3 rotation);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void RigidBody_SetVelocity(uint entityID, ref Vector3 rotation);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void RigidBody_AddForce(uint entityID, out Vector3 force, int mode);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float RigidBody_GetGravityFactor(uint entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void RigidBody_SetGravityFactor(uint entityID, float factor);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool RigidBody_IsGravityOff(uint entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void RigidBody_OffGravity(uint entityID, bool condition);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool ColliderShape_IsEnabled(uint entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void ColliderShape_SetEnabled(uint entityID, bool enabled);

        //End Physics

        //Layer
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static uint LayerMask_GetCollisionMask(string layerName);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static uint LayerMask_ToMask(string layerName);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string LayerMask_LayerToName(uint layer);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static uint LayerMask_NameToLayer(string layerName);
        //End Layer

        //Raycasting
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Physics_Raycast(out Vector3 origin, out Vector3 direction, ref uint bodyHitID, ref Vector3 hitPos, ref Vector3 normal, bool triggerInteraction, uint mask);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Physics_Spherecast(out Vector3 origin, out Vector3 direction, float radius, ref uint bodyHitID, ref Vector3 hitPos, ref Vector3 normal, bool triggerInteraction, uint mask);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Physics_DrawRay(ref Vector3 origin, ref Vector3 direction, float magnitude);


        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Physics_RayUpdateMovement(uint entityID, out Vector3 d_m);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string Audio_GetSoundName(uint entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Audio_Play(uint entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Audio_PlaySFX(string key, ref Vector3 position, uint entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static uint Audio_PlaySFXWithGO(string key, ref Vector3 position, uint entityID);

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
        internal extern static void Audio_SetCategoryVolume(string category, ref float volume);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Audio_GetCategoryVolume(string category);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Audio_SetMasterVolume(float volume);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Audio_GetMasterVolume();

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
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Audio_StopAllSound();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Audio_SetSoundName(uint entityID, ref Audio audioName);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_IsKeyPressed(Keys key);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_IsKeyDown(Keys key);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_IsKeyHold(Keys key);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_IsKeyUp(Keys key);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_IsKeyReleased(Keys keyCode);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_IsMousePressed(MouseButtons button);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_IsMouseDown(MouseButtons button);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_IsMouseHold(MouseButtons button);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_IsMouseUp(MouseButtons button);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Input_IsMouseReleased(MouseButtons button);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Input_GetMousePosition(out Vector2 position);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Input_GetMouseDelta(out Vector2 position);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static int Input_GetCursorState();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Input_SetCursorState(int cursorState);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void AM_EnableMap(string map, bool enable);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static bool AM_PerformedThisFrame(string map, string action);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static float AM_GetValue1D(string map, string action);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void AM_GetValue2D(string map, string action, out Vector2 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_GetDuration(uint entityID, out float duration);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetDuration(uint entityID, ref float duration);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_GetRepeating(uint entityID, out bool repeating);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetRepeating(uint entityID, ref bool repeating);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_GetLocalSpace(uint entityID, out bool localSpace);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetLocalSpace(uint entityID, ref bool localSpace);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_GetDestroyOnExpire(uint entityID, out bool destroyOnExpire);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetDestroyOnExpire(uint entityID, ref bool destroyOnExpire);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_GetMaxParticles(uint entityID, out ulong maxParticles);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetMaxParticles(uint entityID, ref ulong maxParticles);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_GetGForce(uint entityID, out float gForce);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetGForce(uint entityID, ref float gForce);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_GetCollision(uint entityID, out bool collision);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetCollision(uint entityID, ref bool collision);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_GetBounce(uint entityID, out bool bounce);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetBounce(uint entityID, ref bool bounce);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_GetEmissionRate(uint entityID, out float emissionRate);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetEmissionRate(uint entityID, ref float emissionRate);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_GetShapeType(uint entityID, out ShapeType shapeType);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetShapeType(uint entityID, ref ShapeType shapeType);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_GetConeArc(uint entityID, out float coneArc);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetConeArc(uint entityID, ref float coneArc);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_GetConeRadius(uint entityID, out float coneRadius);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetConeRadius(uint entityID, ref float coneRadius);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_GetSphereRadius(uint entityID, out float sphereRadius);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetSphereRadius(uint entityID, ref float sphereRadius);

        // Scale

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_GetScaleValueType(uint entityID, out ParticleSystem.ValueType scaleType);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetScaleValueType(uint entityID, ref ParticleSystem.ValueType scaleType);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_GetScale(uint entityID, out Vector3 scale);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetScale(uint entityID, ref Vector3 scale);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_GetScaleMin(uint entityID, out Vector3 minScale);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetScaleMin(uint entityID, ref Vector3 minScale);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_GetScaleMax(uint entityID, out Vector3 maxScale);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetScaleMax(uint entityID, ref Vector3 maxScale);

        // Lifetime

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_GetLifetimeValueType(uint entityID, out ParticleSystem.ValueType lifetimeType);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetLifetimeValueType(uint entityID, ref ParticleSystem.ValueType lifetimeType);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_GetLifetime(uint entityID, out float lifetime);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetLifetime(uint entityID, ref float lifetime);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_GetMinLifetime(uint entityID, out float minLifetime);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetMinLifetime(uint entityID, ref float minLifetime);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_GetMaxLifetime(uint entityID, out float maxLifetime);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetMaxLifetime(uint entityID, ref float maxLifetime);

        // Rotation

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_GetRotationValueType(uint entityID, out ParticleSystem.ValueType rotationType);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetRotationValueType(uint entityID, ref ParticleSystem.ValueType rotationType);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_GetRotation(uint entityID, out float rotation);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetRotation(uint entityID, ref float rotation);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_GetRotationMin(uint entityID, out float minRotation);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetRotationMin(uint entityID, ref float minRotation);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_GetRotationMax(uint entityID, out float maxRotation);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetRotationMax(uint entityID, ref float maxRotation);

        // 3D Rotation

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_GetIsRotation3D(uint entityID, out bool isRotation3D);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetIsRotation3D(uint entityID, ref bool isRotation3D);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_GetRotation3DHint(uint entityID, out Vector3 rotation3DHint);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetRotation3DHint(uint entityID, ref Vector3 rotation3DHint);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_GetMinRotation3DHint(uint entityID, out Vector3 minRotation3DHint);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetMinRotation3DHint(uint entityID, ref Vector3 minRotation3DHint);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_GetMaxRotation3DHint(uint entityID, out Vector3 maxRotation3DHint);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetMaxRotation3DHint(uint entityID, ref Vector3 maxRotation3DHint);


        // Spawn Position

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_GetSpawnPosValueType(uint entityID, out ParticleSystem.ValueType spawnType);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetSpawnPosValueType(uint entityID, ref ParticleSystem.ValueType spawnType);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_GetSpawnPos(uint entityID, out Vector3 spawnPos);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetSpawnPos(uint entityID, ref Vector3 spawnPos);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_GetSpawnPosMin(uint entityID, out Vector3 minSpawnPos);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetSpawnPosMin(uint entityID, ref Vector3 minSpawnPos);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_GetSpawnPosMax(uint entityID, out Vector3 maxSpawnPos);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetSpawnPosMax(uint entityID, ref Vector3 maxSpawnPos);

        // Colour

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_GetColourValueType(uint entityID, out ParticleSystem.ValueType colourType);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetColourValueType(uint entityID, ref ParticleSystem.ValueType colourType);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_GetColour(uint entityID, out Vector4 colour);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetColour(uint entityID, ref Vector4 colour);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_GetColourMin(uint entityID, out Vector4 minColour);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetColourMin(uint entityID, ref Vector4 minColour);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_GetColourMax(uint entityID, out Vector4 maxColour);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetColourMax(uint entityID, ref Vector4 maxColour);

        // Speed

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_GetSpeedValueType(uint entityID, out ParticleSystem.ValueType speedType);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetSpeedValueType(uint entityID, ref ParticleSystem.ValueType speedType);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_GetSpeed(uint entityID, out float speed);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetSpeed(uint entityID, ref float speed);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_GetSpeedMin(uint entityID, out float minSpeed);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetSpeedMin(uint entityID, ref float minSpeed);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_GetSpeedMax(uint entityID, out float maxSpeed);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetSpeedMax(uint entityID, ref float maxSpeed);

        // Post processing effects

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_GetGlow(uint entityID, out bool hasGLow);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetGlow(uint entityID, ref bool hasGlow);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_GetGlowIntensity(uint entityID, out float glowIntensity);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetGlowIntensity(uint entityID, ref float glowIntensity);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_GetMinGlowIntensity(uint entityID, out float minGlowIntensity);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetMinGlowIntensity(uint entityID, ref float minGlowIntensity);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_GetMaxGlowIntensity(uint entityID, out float maxGlowIntensity);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetMaxGlowIntensity(uint entityID, ref float maxGlowIntensity);

        // Misc

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_GetColourOverLifetime(uint entityID, out bool colourOverLifetime);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetColourOverLifetime(uint entityID, ref bool colourOverLifetime);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_GetTextureID(uint entityID, out uint textureID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetTextureID(uint entityID, ref uint textureID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_IsExpired(uint entityID, out bool isExpired);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_GetSystemTimer(uint entityID, out float systemTimer);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void ParticleSystem_SetSystemTimer(uint entityID, ref float systemTimer);


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
        internal extern static uint[] Entity_GetAllChildren(uint entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static uint Entity_FindEntityWithTag(string tag);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static uint Entity_FindEntityWithName(string name);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static uint Entity_FindEntityWithID(uint id);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static uint Entity_SetParent(uint id, uint parent);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Entity_IsValid(uint entity);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static UInt32 Entity_GetLayer(uint entity);

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
        internal extern static float GetCurrAnimTime(uint entity);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float GetCurrAnimFPS(uint entity);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool IsCurrAnimFin(uint entity);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool SafeToChange(uint entity, string name);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Destroy(uint entity);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Entity_SetTag(uint entity, string tag);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string Entity_GetTag(uint entity);




        /************************* UI Functions *************************/
        /************************* Rect Transform *************************/
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void RectTransform_GetHoriAlign(uint entityID, out RectTransform.HoriPivot hori);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void RectTransform_SetHoriAlign(uint entityID, ref RectTransform.HoriPivot hori);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void RectTransform_GetVertAlign(uint entityID, out RectTransform.VertPivot vert);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void RectTransform_SetVertAlign(uint entityID, ref RectTransform.VertPivot vert);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static int RectTransform_GetPosX(uint entityID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void RectTransform_SetPosX(uint entityID, int value);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static int RectTransform_GetPosY(uint entityID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void RectTransform_SetPosY(uint entityID, int value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static int RectTransform_GetWidth(uint entityID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void RectTransform_SetWidth(uint entityID, int value);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static int RectTransform_GetHeight(uint entityID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void RectTransform_SetHeight(uint entityID, int value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static int RectTransform_GetTop(uint entityID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void RectTransform_SetTop(uint entityID, int value);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static int RectTransform_GetBot(uint entityID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void RectTransform_SetBot(uint entityID, int value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static int RectTransform_GetLeft(uint entityID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void RectTransform_SetLeft(uint entityID, int value);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static int RectTransform_GetRight(uint entityID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void RectTransform_SetRight(uint entityID, int value);


        /************************* Slider *************************/
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Slider_GetValue(uint entityID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Slider_SetValue(uint entityID, float value);

        /************************* Button *************************/
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float Button_SetEnabled(uint entityID, bool enabled);

        /************************* Font *************************/
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static string FontRenderer_GetText(uint entityID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void FontRenderer_SetText(uint entityID, string value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float FontRenderer_GetFontsize(uint entityID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void FontRenderer_SetFontsize(uint entityID, float value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float FontRenderer_GetLinespacing(uint entityID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void FontRenderer_SetLinespacing(uint entityID, float value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void FontRenderer_GetColor(uint entityID, out Vector4 color);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void FontRenderer_SetColor(uint entityID, ref Vector4 value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void FontRenderer_GetAlignment(uint entityID, out FontRenderer.FontAlignment shapeType);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void FontRenderer_SetAlignment(uint entityID, ref FontRenderer.FontAlignment shapeType);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void FontRenderer_SetEnabled(uint entityID, bool enabled);

        /************************* Sprite Renderer *************************/
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SpriteRenderer_GetColor(uint entityID, out Vector4 color);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SpriteRenderer_SetColor(uint entityID, ref Vector4 value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SpriteRenderer_SetEnabled(uint entityID, bool enabled);

        /************************* Sprite Animator *************************/
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SpriteAnimator_SetEnabled(uint entityID, bool enabled);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool SpriteAnimator_GetPlaying(uint entityID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SpriteAnimator_SetPlaying(uint entityID, bool value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool SpriteAnimator_GetLoop(uint entityID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SpriteAnimator_SetLoop(uint entityID, bool value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static uint SpriteAnimator_GetRows(uint entityID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SpriteAnimator_SetRows(uint entityID, uint value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static uint SpriteAnimator_GetCols(uint entityID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SpriteAnimator_SetCols(uint entityID, uint value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static uint SpriteAnimator_GetFrameCnt(uint entityID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SpriteAnimator_SetFrameCnt(uint entityID, uint value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float SpriteAnimator_GetFPS(uint entityID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SpriteAnimator_SetFPS(uint entityID, float value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static uint SpriteAnimator_GetCurrFrame(uint entityID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SpriteAnimator_SetCurrFrame(uint entityID, uint value);

        /************************* Sprite Animator *************************/
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SpriteGammaOverride_SetEnabled(uint entityID, bool enabled);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float SpriteGammaOverride_GetGamma(uint entityID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SpriteGammaOverride_SetGamma(uint entityID, float value);

        /************************* UI End *************************/
        //Material
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Renderer_GetCastShadow(uint entityID, out bool castShadow);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Renderer_SetCastShadow(uint entityID, bool castShadow);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Renderer_IsEnabled(uint entityID);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Renderer_SetEnabled(uint entityID, bool enabled);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Material_GetColor(uint entityID, out Vector4 color);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Material_SetColor(uint entityID, ref Vector4 value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Material_GetColorEmission(uint entityID, out Vector4 color);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Material_SetColorEmission(uint entityID, ref Vector4 value);

        // Skybox
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Skybox_GetLightingPower(out float value);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Skybox_GetZenithColor(out Vector3 value);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Skybox_GetHorizonColor(out Vector3 value);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Skybox_GetGroundColor(out Vector3 value);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Skybox_GetSunDirection(out Vector3 value);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Skybox_GetSunColor(out Vector3 value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Skybox_SetLightingPower(ref float value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Skybox_SetZenithColor(ref Vector3 value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Skybox_SetHorizonColor(ref Vector3 value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Skybox_SetGroundColor(ref Vector3 value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Skybox_SetSunDirection(ref Vector3 value);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Skybox_SetSunColor(ref Vector3 value);

        //Entity active
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void Entity_SetActive(uint entityID, bool active);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool Entity_IsActive(uint entityID);
    }
}
