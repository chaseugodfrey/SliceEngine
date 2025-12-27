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
#include <DetourNavMesh.h>
#include <DetourNavMeshQuery.h>

//#include "PropConfig.h"
//#include <xprop/xproperty.h>

using Entity = entt::entity;
using Registry = entt::registry;

namespace SliceEngine
{
	struct PrefabEditingEntity
	{

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

		glm::vec3 GetWorldPosition()
		{
			return glm::vec3(transform[3][0], transform[3][1], transform[3][2]);
		}

		glm::quat GetWorldRotation()
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

		glm::vec3 GetWorldScale()
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
		DEBUG_ALL_DEBUG		= 0xFF,

		RENDER_NONE			= 0x00,
		RENDER_FOG			= 0x01,
		RENDER_BLUR			= 0x02,
		RENDER_BLOOM		= 0x04,
		RENDER_VIGNETTE		= 0x08,
		RENDER_TAG_ALL		= 0xFF
	};

	struct Renderer
	{
		// May need to change if rendering pipeline is diff
		//GUID model = (GUID)Type<SliceEngineTypes::Model>::defaultResourceGUID; 
		//GUID material = (GUID)Type<SliceEngineTypes::Material>::defaultResourceGUID;

		Handle<SliceEngineTypes::Model> modelHandle;
		Handle<SliceEngineTypes::Material> materialHandle;

		unsigned char meshOffset{ 0 };
		unsigned char renderTag{};
		bool componentEnabled{ true };
		bool skinned{ false };

		RTTR_ENABLE();
	};

	struct Camera
	{
		int width{ 1920 }, height{ 1080 };
		float pov{ 60.f }, near{ 0.01f }, far{ 200.f };// Pov is the angle of y of the screen
		GLuint textureID{}, depthTex{};
		glm::vec3 fogColor{ 0.2f, 0.2f, 0.2f };
		float fogIntensity{ 0.04f };
		float bloomFilterRadius{ 5.f };
		float bloomStrength{ 0.4f };
		float exposure{ 10.f };
		glm::vec2 vignetteCenter{ 0.5f, 0.5f };
		float vignetteIntensity{ 0.336f };
		float vignetteSmoothness{ 0.7f };
		unsigned char debugRenderToggles{};
		unsigned char postRenderToggles{};
		bool componentEnabled{ true };
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
		glm::vec3 color{1.0f, 1.0f, 1.0f};
		float intensity{ 0.5f };
		GLuint depthTex{};
		GLuint shadowCubeMap{};
		LightType type = LightType::Light_Point;

		RTTR_ENABLE();
	};

	struct Prefab
	{
		unsigned int prefabID;

		// GUID reference to original prefab
		GUID prefabGUID;

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

		JPH::BodyID bodyID;													  // Jolt body reference
		std::variant<BoxData, SphereData, CapsuleData> shapeData = BoxData{}; // will add more if we have more shapes :D
		JPH::ShapeRefC shape{ nullptr };												  // Jolt shape ref
		JPH::Vec3 offSet{ 0.f,0.f,0.f };									  // if we need to offset the collision shape relative to the transform :D
		JPH::Vec3 prevOffSet{ 0.f,0.f,0.f };
		bool isTrigger = false;	
		bool componentEnabled = true;

		ColliderShape() = default;
		ColliderShape(BoxData data) : shapeData(data) {};
		ColliderShape(SphereData data) : shapeData(data) {};
		ColliderShape(CapsuleData data) : shapeData(data) {};

	private:
		inline static const BoxData defaultBoxData{};
		inline static const SphereData defaultSphereData{};
		inline static const CapsuleData defaultCapsuleData{};		
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

		// Setters
		void SetBoxData(const BoxData& data) { shapeData = data; }
		void SetSphereData(const SphereData& data) { shapeData = data; }
		void SetCapsuleData(const CapsuleData& data) { shapeData = data; }

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
			UI,
			EditorSounds
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
		VolumeRollOff volumeRollOff = Logarithmic;
		float minDistance = 1.0f;
		float maxDistance = 500.0f;
		bool playOnAwake = false;
		bool playPreview = false;

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
		float age{};             // how long this particle has been alive
		
		glm::vec3 finalPosition{};	// including parent transform position if localspace
		glm::vec3 position{};
		glm::quat rotation{};
		glm::vec3 scale{};
		glm::vec3 velocity{};    // derived from speed + angle
		glm::vec4 colour{};       // if you want per-particle tint
	};

	struct ParticleRenderPart
	{
		glm::mat4 transform{}; // has position, rotation, scale calculated
		glm::vec4 colour{};
		GLuint textureID{};
	};
	struct ParticleSystem
	{
		enum ValueType : unsigned int
		{
			CONSTANT,
			CURVE,
			TWO_CONSTANTS
		};

		Transform* parentTransform{ nullptr };

		// System Settings
		float duration{};                       // how long the system should last, 0.0f = forever
		float speed{};							
		bool isRepeating{ false };
		bool isLocalSpace{ false };				// false means world space

		// Lifetime
		ValueType initialLifetimeType{ CONSTANT };
		float lifetime{};
		float minParticleLifetime{};
		float maxParticleLifetime{};
		// Rotation

		bool isInitialRotation3D{ false };
		ValueType initialRotationType{ CONSTANT };
		glm::quat rotation{};
		glm::quat minRandomRotation{};
		glm::quat maxRandomRotation{};
		glm::vec3 eulerHint{};					// unimplemented
		glm::vec3 minEulerHint{};				// unimplemented
		glm::vec3 maxEulerHint{};				// unimplemented
		
		inline void Set1DRotation(float val)
		{
			eulerHint.x = val;
		}

		inline float Get1DRotation()			
		{
			return eulerHint.x;
		}

		// Size/Scale
		ValueType scaleType{ CONSTANT };
		glm::vec3 scale{ 1.0f };
		glm::vec3 minRandomScale{ 1.0f };
		glm::vec3 maxRandomScale{ 1.0f };

		bool destroyOnExpire{ false };
		uint64_t maxParticles{ 1000 };            // pool size. default 200

		float gForce{0.0f};

		// EMISSION
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
		std::vector<Burst> bursts{};


		// Shape Settings
		enum ShapeType
		{
			CONE,
			SPHERE,
			BOX,
			EDGE,
			CIRCLE,
			RECTANGLE
		} shapeType;

		float coneAngle{};
		float shapeRadius{};					
		float shapeArc{};						

		glm::vec3 axis = glm::vec3(0, 0, 0);   // emission spread - can be internal
		// Initial Position
		bool hasRandomSpawnPos{ false };        // can be calculated - can be internal
		glm::vec3 minRandomSpawnPos{};
		glm::vec3 maxRandomSpawnPos{};

		// Color
		ValueType colorValueType{ CONSTANT };
		glm::vec4 colour{ 0.0f, 0.0f, 0.0f, 1.0f };
		glm::vec4 minRandomColour{ 0.0f, 0.0f, 0.0f, 1.0f };
		glm::vec4 maxRandomColour{ 0.0f, 0.0f, 0.0f, 1.0f };
		bool colorOverLifetime{ false };			// to add
		std::map<float, glm::vec4> colorLifeTimeMap;	// to add

		//bool hasRandomVelocity{ false };			// can remove
		ValueType velocityValueType{ CONSTANT };
		glm::vec3 velocity{ 1.0f };
		glm::vec3 minRandomVelocity{ 1.0f };
		glm::vec3 maxRandomVelocity{ 1.0f };

		//bool fadeOverLifetime{ false };				// can remove
		bool hasCollision{ false };

		// Renderer
		GLuint textureID;							// change to guid
		enum RenderMode
		{
			BILLBOARD,
			MESH
		} renderMode;
		GUID textureGUID;
		GUID materialGUID;
		GUID meshGUID;
		Handle<SliceEngineTypes::Texture> textureHandle;
		Handle<SliceEngineTypes::Material> materialHandle;
		Handle<SliceEngineTypes::Mesh> meshHandle;

		// Internal
		std::vector<Particle> particles{};
		uint64_t awaitingIndex{};				// index that is waiting for ActivateParticle
		uint64_t oldestIndex{};					// oldest particle index as backup when exceeding maxParticles, use this particle then +1 the index

		// Main Particle Storage Poooool

		bool systemEnding{ false };				// Turns true when particle system expired and just waiting for its particles to all expire
		bool expired{ false };					// Turns true when all particles have expired + systemEnding is true
		bool isActive{ true };
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

		Handle<SliceEngineTypes::AnimationPackage> Handle_curr_anim_pkg;
		Handle<SliceEngineTypes::Skeleton> Handle_skeleton;

		SliceEngineTypes::AnimationPackage curr_anim_pkg;


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
			OVERLAY
			//CAMERA
			//WORLD
		};

		bool componentEnabled{ true };
		Type canvas_type{ OVERLAY };
		unsigned int sort_order{};	//smaller number = draw first = behind others
		bool graphic_raycastable{ true };	//bool that determines if images in its hierachy can be raycasted
									//only for overlay canvas

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
		int pos_x{}, pos_y{};			//pixel coord
		int width{ 100 }, height{ 100 };//pixel size
		int left{}, right{}, top{}, bot{};		//only used when pivots are stretch

		//Actual settings used to draw
		int final_x{}, final_y{};				//position with center of quad as position
		int final_width{ 100 }, final_height{ 100 };

		//Parent/Canvas reference - done via passing param through the recursive func call maybe
		void Update(Canvas const& ctx, RectTransform const& parent);

		glm::mat4 ToMatrix() const;

		RTTR_ENABLE();
	};


	struct SpriteRenderer {
		bool componentEnabled{ true };
		GUID textureHandle{ (GUID)DefaultResourceIDs::COLOR_DEADED_DEFAULT };	//resource handle for texture
		glm::vec4 rgba{1.f, 0.f, 0.f, 1.f};
		float alphathreshold{ 0.5f };	//alpha cutoff for raycasting
		bool raycast_target{ true };
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
		glm::vec4 color_transitions[Total_States]{
			{1.f, 1.f, 1.f, 1.f},	//white
			{0.75f, 0.75f, 0.75f, 1.f},//light grey
			{0.5f, 0.5f, 0.5f, 1.f}//dark grey
		};
		GUID sprite_transitions[Total_States]{
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
		//for now only allow a normalized value - 0 to 1
		float value{ 0 };
	};

	// Not a component but a base data obj for nav mesh
	struct NavMeshObj
	{
		dtNavMesh* navMesh;
		dtNavMeshQuery* navMeshQuery;
	};

	struct NavMeshDebugObj
	{
		struct data
		{
			uint32_t vao;
			uint32_t vbo;
			uint32_t drawCnt;
		};

		data data[2];
	};

	// Component
	struct NavAgent
	{
		bool componentEnabled{ true };
		glm::vec3 target = glm::vec3(0.0f);
		std::vector<glm::vec3> currentPath;
		int currentPathIndex = 0;

		float speed = 2.0f;
		bool hasNewTarget = false;
	};
}

#endif