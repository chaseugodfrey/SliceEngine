/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			ECSTypes.h
 author:		
 email:			
 brief:			Structs for the different components

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#ifndef ECS_TYPES
#define ECS_TYPES

#include <entt.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glfw3.h>
#include <../fmod/include/fmod.hpp>
#include <variant>
#include "../Physics/CollisionLayer.h"
#include <rttr/rttr_enable.h>
#include "Resource/Resource.h"
#include "Resource/ResourceManager.h"
#include "Animator/FSMSystem.h"
#include "Resource/Skeleton.h"

//#include "PropConfig.h"
//#include <xprop/xproperty.h>

using Entity = entt::entity;
using Registry = entt::registry;

namespace SliceEngine
{
	/// <summary>
	/// Purely for mapping C# prefab variable to our own
	/// Cause the prefab var we use has too many unnecessary variables in it
	/// </summary>
	struct PrefabVar
	{
		std::string prefabFileName;
	};

	struct PrefabEditingEntity
	{

	};

	struct InactiveEntity
	{
		bool mTest{ false };

		InactiveEntity() {}
	};

	struct SliceEntity 
	{
		std::string mName;
		std::string mTag{ "default" };
		uint32_t mLayer{ 0u };

		bool mActive;

		SliceEntity() : mActive(true) {}
	};

	struct EngineEntity
	{
		bool mActive;
		
		EngineEntity() : mActive(true) {}
	};

	struct SceneGraph
	{
		uint32_t entity_id{};

		enum Direction {
			UP = 0,
			DOWN,
			LEFT,
			RIGHT,
			DIRECTIONS
		};

		std::array<Entity, Direction::DIRECTIONS> neighbours{ entt::null, entt::null, entt::null, entt::null };

		RTTR_ENABLE();
	};

	struct Script
	{
		bool componentEnabled{ true };
		std::string scriptName;

		// purely for serialization and deserialization
		// to save scriptable field values in scenes and for prefabs(?)
		std::unordered_map<std::string, rttr::variant> scriptableFieldMap;

		RTTR_ENABLE();
	};

    struct Transform
    {

        glm::vec3 position{ 0.0f, 0.0f, 0.0f };
        glm::quat rotation{ 1.0f, 0.0f, 0.0f, 0.0f };
        glm::vec3 scale{ 1.0f, 1.0f, 1.0f };
        glm::mat4 transform_local{ 1.0f };
        glm::mat4 transform{ 1.0f };

		glm::vec3 eulerAnglesHint{ 0.0f, 0.0f, 0.0f };

		glm::vec3 GetWorldPosition() const
		{
			return glm::vec3(transform[3][0], transform[3][1], transform[3][2]);
		}

		glm::quat GetWorldRotation() const
		{
			glm::mat4 rotMat = transform;

			// Extract and normalize the basis vectors to remove scale
			glm::vec3 col0 = glm::normalize(glm::vec3(rotMat[0]));
			glm::vec3 col1 = glm::normalize(glm::vec3(rotMat[1]));
			glm::vec3 col2 = glm::normalize(glm::vec3(rotMat[2]));

			// Reconstruct a pure rotation matrix
			rotMat[0] = glm::vec4(col0, 0.0f);
			rotMat[1] = glm::vec4(col1, 0.0f);
			rotMat[2] = glm::vec4(col2, 0.0f);
			rotMat[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

			return glm::quat_cast(rotMat);
		}

		glm::vec3 GetWorldScale() const
		{
			glm::vec3 _scale{};
			_scale.x = glm::length(glm::vec3(transform[0]));
			_scale.y = glm::length(glm::vec3(transform[1]));
			_scale.z = glm::length(glm::vec3(transform[2]));
			return _scale;
		}

		void SetWorldPosition(const glm::vec3& newPos)
		{
			transform[3] = glm::vec4(newPos, 1.0f);
		}

		void SetWorldRotation(const glm::quat& newRot)
		{
			glm::vec3 currentScale = GetWorldScale();
			glm::vec3 currentPos = GetWorldPosition();

			glm::mat4 rotMat = glm::mat4_cast(newRot);
			transform = glm::scale(rotMat, currentScale);
			transform[3] = glm::vec4(currentPos, 1.0f);
		}

		void SetWorldScale(const glm::vec3& newScale)
		{
			glm::vec3 currentScale = GetWorldScale();

			// Scale each basis vector
			transform[0] *= (newScale.x / currentScale.x);
			transform[1] *= (newScale.y / currentScale.y);
			transform[2] *= (newScale.z / currentScale.z);
		}

		RTTR_ENABLE();
    };

	struct UITransform
	{
		// blank for now because I just need to use this for factory stuff
	};

	struct SelectedEntity{};

	//XPROPERTY_REG(Transform);

	enum RENDER_TAG : unsigned char
	{
		DEBUG_NONE			= 0x00,
		DEBUG_OBJ_TAG		= 0x01,
		DEBUG_FRUSTRUM_TAG	= 0x02,
		DEBUG_GRID_TAG		= 0x04,
		DEBUG_NAVMESH_TAG	= 0x08,
		DEBUG_OUTLINE_SELECTED_TAG	= 0x10,
		DEBUG_DRAW_RAY_TAG = 0x20,
		DEBUG_ALL_DEBUG		= 0xFF,

		RENDER_NONE			= 0x00,
		RENDER_FOG			= 0x01,
		RENDER_BLUR			= 0x02,
		RENDER_BLOOM		= 0x04,
		RENDER_VIGNETTE		= 0x08,
		RENDER_GROUND_CLOUD = 0x10,
		RENDER_GODRAY		= 0x20,
		RENDER_IMPACT		= 0x40,
		RENDER_TAG_ALL		= 0xFF
	};

	struct Renderer
	{
		// May need to change if rendering pipeline is diff
		//GUID model = (GUID)Type<SliceEngineTypes::Model>::defaultResourceGUID; 
		//GUID material = (GUID)Type<SliceEngineTypes::Material>::defaultResourceGUID;

		Handle<SliceEngineTypes::Model> modelHandle;
		Handle<SliceEngineTypes::Material> materialHandle;
		SliceEngineTypes::Material materialInstance;

		unsigned char meshOffset{ 0 };
		unsigned char renderTag{};
		bool componentEnabled{ true };
		bool skinned{ false };
		bool castShadow{ true };

		RTTR_ENABLE();
	};

	struct Camera
	{
		int width{ 1920 }, height{ 1080 };
		float pov{ 60.f }, near{ 0.01f }, far{ 3000.f };// Pov is the angle of y of the screen
		GLuint textureID{}, depthTex{}, lum[2]{};
		float luminanceLearningRate{ 10.f };
		glm::vec3 fogColor{ 0.2f, 0.2f, 0.2f };
		float fogIntensity{ 0.04f };
		float bloomFilterRadius{ 5.f };
		float bloomStrength{ 0.4f };
		float bloomLimit{ 1.f };
		float exposure{ 10.f };
		float gamma{ 45.4545f };
		float whiteBalance{ 0.98f };
		float minLuminance{ 0.0001f };
		float maxLuminance{ 10.0f };
		float godRayFilterRadius{ 5.f };
		float godRayStrength{ 0.4f };
		glm::vec2 vignetteCenter{ 0.5f, 0.5f };
		float vignetteIntensity{ 0.336f };
		float vignetteSmoothness{ 0.7f };
		glm::vec3 impactPos{ 0.0f };
		glm::vec3 impactColor{ 1.0f, 1.0f, 1.0f };
		glm::vec3 impactColor2{ 0.0f, 0.0f, 0.0f };
		bool impactSmooth{ false };
		float impactEpilepsy{ 7.0f };
		float impactAngle{ 18.0f };
		float impactNoise1{ 148.0f };
		float impactNoise2{ 21.0f };
		float impactBlend{ 1.0f };

		float cloudsHeight{ -110.f };
		float cloudsAmplitude{ 49.f };
		float cloudsIntensity{ 0.3f };
		float cloudsSmoothness{ 0.0027f };
		float cloudsCutoff{ 0.167f };
		glm::vec4 cloudsColor{ 1.f,1.f,1.f,0.25f };
		glm::vec3 cloudsSecondCloudOffset{40.f, 40.f, -20.f};
		float cloudsSecondCloudAmplitude{ 49.f };
		float cloudsSecondCloudIntensity{ 0.3f };
		float cloudsSecondCloudSmoothness{ 0.0027f };
		glm::vec4 cloudsSecondColor{ 1.f,1.f,1.f,0.25f };

		float translucentSelectCutoff{ 0.2f };
		unsigned char debugRenderToggles{};
		unsigned char postRenderToggles{};
		glm::mat4 V{};
		glm::mat4 P{};
		bool isMainCamera{ false };
		bool componentEnabled{ true };
		bool lumSelected{ false };
		bool camLoaded{ false };
		RTTR_ENABLE();
	};

	struct Light // TODO: Default 1 directional light for now
	{
		enum LightType : unsigned char
		{
			Light_Directional = 0
			,Light_Point
			,Light_Spot
		};
		bool componentEnabled{ true };
		bool castsShadow{ true };
		glm::vec3 color{1.0f, 1.0f, 1.0f};
		float intensity{ 0.5f };
		float angle{ 90.f };
		LightType type = LightType::Light_Point;

		RTTR_ENABLE();
	};

	struct Prefab
	{
		unsigned int prefabID{};

		// GUID reference to original prefab
		GUID prefabGUID{};

		Handle<SliceEngineTypes::Prefab> prefabHandle;

		RTTR_ENABLE();
	};

	struct RigidBody
	{
		
		bool isKinematic = false;		// Set to Kinematic :D
		float gravityFactor = 1.0f;		// Gravity multiplier:D
		JPH::EMotionQuality CollisionDetection = JPH::EMotionQuality::Discrete; // Motion quality(Discrete or Continuous) :D

		// Physics properties
		float mass = 1.0f; //:D
		float friction = 0.5f;
		float restitution = 0.0f;						   // Bounciness
		float linearDamping = 0.05f;    //:D
		float angularDamping = 0.05f;	//:D


		//To add in Inspector
		struct FreezeOptions
		{
			bool freezeX = false;
			bool freezeY = false;
			bool freezeZ = false;
		};

		FreezeOptions freezePosition;
		FreezeOptions freezeRotation;

		RTTR_ENABLE();
	};

	struct ColliderShape
	{
		struct BoxData
		{
			JPH::Vec3 scale{ 0.5f, 0.5f,0.5f };
		};

		struct SphereData
		{
			float radius{ 0.5f };
		};

		struct CapsuleData
		{
			float radius{ 0.5f };
			float height{ 0.5f };
		};

		struct MeshData
		{
			//emtpy because the verticies are in the renderer component
			//more for me to tell what shape it is
			bool temp;
		};

		struct CylinderData
		{
			float radius{ 0.5f };
			float height{ 0.5f };
		};

		JPH::BodyID bodyID;													  // Jolt body reference
		std::variant<BoxData, SphereData, CapsuleData, MeshData, CylinderData> shapeData = BoxData{}; // will add more if we have more shapes :D
		JPH::ShapeRefC shape{ nullptr };												  // Jolt shape ref
		JPH::Vec3 offSet{ 0.f,0.f,0.f };									  // if we need to offset the collision shape relative to the transform :D
		JPH::Vec3 prevOffSet{ 0.f,0.f,0.f };
		bool isTrigger = false;	
		bool componentEnabled = true;

		ColliderShape() = default;
		ColliderShape(BoxData data) : shapeData(data) {};
		ColliderShape(SphereData data) : shapeData(data) {};
		ColliderShape(CapsuleData data) : shapeData(data) {};
		ColliderShape(MeshData data) : shapeData(data) {};
		ColliderShape(CylinderData data) : shapeData(data) {};

	private:
		inline static const BoxData defaultBoxData{};
		inline static const SphereData defaultSphereData{};
		inline static const CapsuleData defaultCapsuleData{};	
		inline static const MeshData defaultMeshData{};
		inline static const CylinderData defaultCylinderData{};
	public:
		// Getters
		const BoxData& GetBoxData() const {
			return std::holds_alternative<BoxData>(shapeData) ?
				std::get<BoxData>(shapeData) : defaultBoxData;
		}

		const SphereData& GetSphereData() const {
			return std::holds_alternative<SphereData>(shapeData) ?
				std::get<SphereData>(shapeData) : defaultSphereData;
		}

		const CapsuleData& GetCapsuleData() const {
			return std::holds_alternative<CapsuleData>(shapeData) ?
				std::get<CapsuleData>(shapeData) : defaultCapsuleData;
		}
		const MeshData& GetMeshData() const {
			return std::holds_alternative<MeshData>(shapeData) ?
				std::get<MeshData>(shapeData) : defaultMeshData;
		}

		const CylinderData& GetCylinderData() const {
			return std::holds_alternative<CylinderData>(shapeData) ?
				std::get<CylinderData>(shapeData) : defaultCylinderData;
		}

		// Setters
		void SetBoxData(const BoxData& data) { shapeData = data; }
		void SetSphereData(const SphereData& data) { shapeData = data; }
		void SetCapsuleData(const CapsuleData& data) { shapeData = data; }
		void SetMeshData(const MeshData& data) { shapeData = data; }
		void SetCylinderData(const CylinderData& data) { shapeData = data; }

		RTTR_ENABLE();
	};

	struct AudioSource
	{
		enum VolumeRollOff : int
		{
			Logarithmic = 0,
			Linear = 1
		};

		enum Category : int
		{
			SFX,
			BGM,
			UI
		};
		//std::string soundName;
		bool componentEnabled{ true };
		GUID soundGUID = (GUID)10155432597037438324;
		FMOD::Channel* channel = nullptr;
		FMOD::Channel* previewChannel = nullptr;
		int priority = 128;
		bool isMute = false;
		bool isLoop = false;
		bool isPaused = false;
		float currentVolume = 1.0f;
		Category category = SFX;
		float pitch = 1.0f;
		float stereoPan = 0.0f;
		float spatialBlend = 1.0f;
		//3D effects
		float dopplerLevel = 1.0f;
		float spread = 1.0f;
		VolumeRollOff volumeRollOff = Linear;
		float minDistance = 1.0f;
		float maxDistance = 500.0f;
		bool playOnAwake = false;
		bool playPreview = false;
		bool enablePathfinding = false;
		float directOcclusion = 0.0f;
		float reverbOcclusion = 0.0f;
		bool destroyOnEnd = false;

		RTTR_ENABLE();
	};

	struct AudioListener
	{
		bool componentEnabled{ true };
		glm::vec3 listenerPos{};

		RTTR_ENABLE();
	};

	struct Particle
	{
		bool active{ false };

		float maxAge{};
		float age{};
		float rotation{};

		inline float normalizedAge() const { return age / maxAge; }	

		glm::vec3 position{};
		glm::vec3 scale{};
		glm::vec3 velocity{};
		glm::vec4 colour{};
		glm::quat rotation3D{};
	};

	struct ParticleRenderPart
	{
		glm::mat4 transform{}; // has position, rotation, scale calculated
		glm::vec4 colour{};		
		GLuint64 textureID{};

		float glowIntensity{};
		bool isMeshParticle{false};
		bool isIgnoreLights{ false };

		GUID modelGUID;
		GUID materialGUID;
	};
	struct ParticleSystem
	{
		enum ValueType : unsigned int
		{
			CONSTANT,
			TWO_CONSTANTS
		};

		// Editor
		bool playPreview{ false };
		bool resetPreview{ false };
		bool pausePreview{ false };

		Transform* parentTransform{ nullptr };

		// System Settings
		float duration{5.0f};                // how long the system should last, 0.0f = forever					
		float initialDelay{};
		bool isRepeating{ false };
		bool isLocalSpace{ false };				// false means world space
		bool followTransformRotation{ true };

		inline float WrapAngle(float deg)
		{
			while (deg > 180.f) deg -= 360.f;
			while (deg < -180.f) deg += 360.f;
			return deg;
		}

		inline glm::vec3 WrapEuler(glm::vec3 e)
		{
			return {
				WrapAngle(e.x),
				WrapAngle(e.y),
				WrapAngle(e.z)
			};
		}

		bool destroyOnExpire{ false };
		uint64_t maxParticles{ 200 };            // pool size. default 200

		// Physics
		float gForce{0.0f};
		bool hasCollision{ false };
		float friction{ 0.9f };
		float bounciness{ 0.0f };
		float bounceDampening{ 0.6f };
		float stickiness{ 0.0f };


		// Emission
		float emissionRate{ 0.0f };              // particles/sec
		// Bursts		
		struct Burst
		{
			uint64_t numParticles{};
			uint64_t burstRepetitions{};		// how many times to do the burst
			float burstPeriod{};				// how far apart in time should each repetition be
			float triggerTime{};				// if greater than systemTimer, trigger burst
			bool triggered{ false };

			uint64_t repsDone{};
			float repTimer{};
		};
		uint64_t numBursts{};
		std::vector<Burst> bursts{};


		// Shape Settings
		enum ShapeType
		{
			SPHERE,
			CONE,
			CUBE,
			CIRCLE,
			RECT,
		} shapeType{ SPHERE };

		// Cone
		float coneArc{90.0f};				

		// Sphere
		float sphereArc{180.0f};
		
		// Cube

		// Circle

		// Rect
		glm::vec2 rectScale{ 1.0f };

		// Shape-Shared params
		float shapeRadius{ 0.1f };
		glm::vec3 shapeScale{ 1.0f };
		float innerShapeRadius{ 0.0f };

		// Shape params that doesnt need to be saved
		float inner3{};
		float outer3{};

		glm::vec3 axis = glm::vec3(0, 0, 0);   // emission spread - can be internal

		// Start Size/Scale
		ValueType scaleType{ CONSTANT };
		glm::vec3 scale{ 1.0f };
		glm::vec3 minRandomScale{ 1.0f };
		glm::vec3 maxRandomScale{ 1.0f };

		// Start Lifetime
		ValueType initialLifetimeType{ CONSTANT };
		float lifetime{5.0f};
		float minParticleLifetime{ 5.0f };
		float maxParticleLifetime{ 5.0f };

		// Start Rotation (1-D spins to reduce workload for a cosmetic system, referencing Unity3D)
		ValueType initialRotationType{ CONSTANT };
		float rotation{};
		float minRandomRotation{};
		float maxRandomRotation{};
		
		// 3D Rotation
		bool isRotation3D{};
		glm::vec3 rotation3DHint{};
		glm::vec3 minRotation3DHint{};
		glm::vec3 maxRotation3DHint{};		

		// Start Position Offset
		ValueType posValueType{ CONSTANT };
		glm::vec3 spawnPos{};					// offset from component owner position
		glm::vec3 minRandomSpawnPos{};
		glm::vec3 maxRandomSpawnPos{};

		// Start Colour
		ValueType colourValueType{ CONSTANT };
		glm::vec4 colour{ 0.0f, 0.0f, 0.0f, 1.0f };
		glm::vec4 minRandomColour{ 0.0f, 0.0f, 0.0f, 1.0f };
		glm::vec4 maxRandomColour{ 0.0f, 0.0f, 0.0f, 1.0f };

		// Start Speed
		ValueType speedValueType{ CONSTANT };
		float speed{1.0f};
		float minRandomSpeed{ 1.0f };
		float maxRandomSpeed{ 1.0f };

		// Size over lifetime
		bool sizeOverLifetime{ false };
		bool sizeSeparateAxis{ false };
		std::map<float, glm::vec3> sizeMap;
		std::vector <std::pair<float, glm::vec3>> sizeMapIntermediary{};
			
		// Rotate over lifetime
		bool rotateOverLifetime{ false };	
		bool rotateSeparateAxis{ false };
		glm::vec3 rotateVelocity{0.f, 0.f, 45.0f};

		// Colour over lifetime
		bool colourOverLifetime{ false };
		std::map<float, glm::vec4> colourLifetimeMap;
		std::vector <std::pair<float, glm::vec4>> colourMapIntermediary{};

		// Velocity over lifetime
		bool velocityOverLifetime{ false };
		bool velocitySeparateAxis{ false };
		std::map<float, glm::vec3> velocityMap;
		std::vector <std::pair<float, glm::vec3>> velocityMapIntermediary{};

		// Orbit over lifetime
		bool orbitOverLifetime{ false };
		glm::vec3 orbitAxis{ glm::vec3(0,0,1) };
		glm::vec3 startOrbitVelocity{1.0f};
		glm::vec3 endOrbitVelocity{0.f};

		// Post processing
		ValueType glowValueType{ CONSTANT };
		bool glow{ false };
		float glowIntensity{};
		float minGlowIntensity{};
		float maxGlowIntensity{};

		// Renderer
		GLuint GetTextureID() const { return static_cast<GLuint>(textureGUID.GetGUID()); }

		enum RenderMode
		{
			BILLBOARD,
			MESH
		} renderMode{ BILLBOARD };

		bool alwaysFaceCamera{ true };
		bool ignoreLights{ false };

		GUID textureGUID;

		Handle<SliceEngineTypes::Texture> textureHandle;
		Handle<SliceEngineTypes::Model> modelHandle;
		Handle<SliceEngineTypes::Material> materialHandle;

		// Internal
		uint32_t particleLayer{ 0 };
		std::vector<Particle> particles{};		// Main Storage of all particles
		uint64_t awaitingIndex{};				// index that is waiting for ActivateParticle
		uint64_t oldestIndex{};					// oldest particle index as backup when exceeding maxParticles, use this particle then +1 the index
		std::vector<ParticleRenderPart> renderData;

		bool systemEnding{ false };				// Turns true when particle system expired and just waiting for its particles to all expire
		bool expired{ false };					// Turns true when all particles have expired + systemEnding is true
		bool isActive{ true };
		float delayTimer{};
		float systemTimer{};					// system's overall lifetime

		float emissionAccumulator{};

		RTTR_ENABLE();
	};

	struct Timeline
	{
		int32_t f_current{}, f_min{ 0 }, f_max{ 60 };
		bool isPlaying{}, isLoop{};
	};

	struct Animator
	{
		bool componentEnabled{ true };
		Handle<SliceEngineTypes::StateMachine> Handle_stateMachine;
		FSMSystem stateMachine;
		
		float current_time{};
		Timeline timeline;

		bool is_bone{ true };

		std::vector<glm::mat4> final_tforms;
		std::bitset<MAX_BONES> inverse_flags{};
		std::unordered_map<unsigned int, glm::mat4> inverse_map{};
		std::unordered_map<unsigned int, glm::vec2> node_position_map{};

		Handle<SliceEngineTypes::AnimationPackage> Handle_curr_anim_pkg;
		Handle<SliceEngineTypes::Skeleton> Handle_skeleton;
		Handle<SliceEngineTypes::SequencePackage> Handle_Anims;

		SliceEngineTypes::AnimationPackage curr_anim_pkg;
		SliceEngineTypes::SequencePackage curr_anims;
		std::vector<SliceEngineTypes::AnimationKeyFrame> eventFrames;


		//tbh these 2 set_x stuff shld be taking in a guid/handle to these resources, then creating and instance of it

		/*
		void SetAnimationPackage(SliceEngine::GUID anim)
		{
			curr_anim_pkg.mGUID = anim;
		}
		void SetSkeleton(SliceEngineTypes::Skeleton* skele) 
		{
			skeleton = skele;
		}
		*/


		void SetInverseRoots() {
			for (unsigned int i{}; i < inverse_flags.size(); ++i) {
				if (inverse_flags[i]) {
					inverse_map[i] = glm::inverse(final_tforms[i]);
				}
			}
		}
		void SetInverseRoot(unsigned int idx) 
		{
			assert(idx < final_tforms.size());
			inverse_map[idx] = glm::inverse(final_tforms[idx]);
		}

		void PlayAnimation(unsigned int idx) 
		{
			stateMachine.EFSM.currState->curr_anim_idx = idx;
		}

		std::vector<glm::mat4> const& GetFinalTform() const
		{
			return final_tforms;
		}

		bool IsValid() const
		{
			return (Handle_skeleton.IsValid() && Handle_curr_anim_pkg.IsValid() && Handle_stateMachine.IsValid());
		}

		RTTR_ENABLE();
	};

	struct Bone 
	{
		Entity skeleton_root{};
		unsigned int frame_idx{};

		RTTR_ENABLE();
	};

	//for canvas, sprite renderer, rect transform, read comments in canvas system.h
	struct Canvas
	{
		enum Type {
			OVERLAY,
			//CAMERA
			WORLD
		};

		bool componentEnabled{ true };
		Type canvas_type{ OVERLAY };
		unsigned int sort_order{};	//smaller number = draw first = behind others
		bool graphic_raycastable{ true };				//bool that determines if images in its hierachy can be raycasted
														//only for overlay canvas

		bool billboardX{ false }, billboardY{ false };	//only for world

		RTTR_ENABLE();
	};

	struct RectTransform {
		enum HoriPivot {
			LEFT,
			CENTER,
			RIGHT,
			STRETCH_H
		};
		enum VertPivot {
			TOP,
			MIDDLE,
			BOTTOM,
			STRETCH_V
		};
	
		//Settings only for imgui's display and component function calls
		//old pivot serves as a flag to know how to update intermediate values during the update call
		HoriPivot hori_pivot{ CENTER };// , old_hori{ CENTER };
		VertPivot vert_pivot{ MIDDLE };// , old_vert{ MIDDLE };

		//Intermediate settings used by imgui, all in local space
		float pos_x{}, pos_y{};			//pixel coord
		float width{ 100 }, height{ 100 };//pixel size
		float left{}, right{}, top{}, bot{};		//only used when pivots are stretch

		//Actual settings used to draw
		float final_x{}, final_y{};				//position with center of quad as position
		float final_width{ 100 }, final_height{ 100 };
		float final_rot{};						//local rotation only, unaffected by parent-child relation

		//scales used for world space transformation only
		float scale_x{}, scale_y{};

		//Parent/Canvas reference - done via passing param through the recursive func call maybe
		void Update(RectTransform const& parent);

		glm::mat4 ToMatrix() const noexcept;

		RTTR_ENABLE();
	};


	struct SpriteRenderer {
		bool componentEnabled{ true };
		GUID textureHandle{ (GUID)DefaultResourceIDs::COLOR_DEADED_DEFAULT };	//resource handle for texture
		glm::vec4 rgba{1.f, 1.f, 1.f, 1.f};
		glm::vec4 uv{ 0.f,1.f,0.f,1.f };
		float alphathreshold{ 0.5f };	//alpha cutoff for raycasting
		bool raycast_target{ true };
		RTTR_ENABLE();
	};

	struct SpriteRendererGammaOverride {
		bool componentEnabled{ true };
		float gamma{ 45.4545f };
		RTTR_ENABLE();
	};

	struct SpriteAnimator {
		bool componentEnabled{ true };
		bool is_playing{ false };
		bool loop{ false };
		unsigned char row { 1 };
		unsigned char col { 1 };
		unsigned char num_frames { 1 };
		float curr_frame { 0 };
		float fps{ 1.f };
		RTTR_ENABLE();
	};

	/*
	* Current assumptions:
	* horizontal wrap
	* text box is same size as rect transform
	* 
	* vertical overflow
	*/
	struct FontRenderer {
		bool componentEnabled{ true };
		bool token_updated{ false };
		GUID fontHandle{};
		glm::vec4 rgba{ 1.f };

		enum Alignment {
			LEFT = 0,
			CENTER,
			RIGHT
		} alignment{ LEFT };


		float font_size{};
		float line_spacing{};	//multiplier of font_size
		
		std::string text{"Hello World"};

		struct Token {
			//std::string text{};
			//const char* pos{};
			unsigned int pos{};
			float size{};
			unsigned int char_cnt{};
		};
		std::vector<Token> token_list{};

		RTTR_ENABLE();
	};

	struct Button {
		RTTR_ENABLE();
	public:
		enum Transition : unsigned char {
			Color,
			Sprite
		} transition;

		enum ButtonState : unsigned char {
			Normal = 0,
			Highlighted = 1,
			Pressed = 2,
			Total_States
		} state;

		bool componentEnabled{ true };
		std::array<glm::vec4, Total_States> color_transitions{
			glm::vec4(1.f, 1.f, 1.f, 1.f),	//white
			glm::vec4(0.75f, 0.75f, 0.75f, 1.f),//light grey
			glm::vec4(0.5f, 0.5f, 0.5f, 1.f)//dark grey
		};
		std::array<GUID, Total_States> sprite_transitions{
			(GUID)DefaultResourceIDs::COLOR_DEADED_DEFAULT,
			(GUID)DefaultResourceIDs::COLOR_DEADED_DEFAULT,
			(GUID)DefaultResourceIDs::COLOR_DEADED_DEFAULT
		};
		//Entity target_graphic;	//if the entity that gets modified by transition not the same
		//im gona move the click stuff to script only
	};

	struct Slider {
		RTTR_ENABLE();
	public:
		//the direction the handle will move along(no diagonal sliders)
		enum Axis : unsigned char {
			X_Axis,
			Y_Axis
		} axis{ X_Axis };
		
		//whehter the value moves in the positive or negative axis
		enum Direction : unsigned char {
			Positive,
			Negative
		} direction{ Positive };


		/*
		* There entities are always children of the slider
		* and their positions will always be relative to it
		*/
		Entity handle{ entt::null };	//basically the slider knob
		Entity fill{ entt::null };		//basically the "filled" portion of a slider, gets stretched depending on val

		//sets the value, positions the handle and fill, and calls c# callback
		void SetValue(float, Entity self);
		float GetValue() const;	//not actually sure if this func is needed

		bool componentEnabled{ true };
		bool contained{ false };	//whether handle should be contained within the slider bg
		//for now only allow a normalized value - 0 to 1
		float value{ 0 };
	};


	// Component

}

#endif