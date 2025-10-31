/*!
\file		AnimatorSystem.h
\author		Muhammad Rayan (muhammadrayan.b@digipen.edu)
\co-author
\brief
	Functions that deal with the Animator for animations

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
*/

#ifndef ANIMATORSYSTEM_H
#define	ANIMATORSYSTEM_H

#include "../Resource/Model.h"
#include "../ECS/BaseSystem.h"
#include "../ECS/ECSTypes.h"
#include "FSMSystem.h"

#define GLM_ENABLE_EXPERIMENTAL

constexpr unsigned char MAX_BONES = 100;
constexpr unsigned char MAX_BONE_INFLUENCE = 4;

namespace SliceEngine
{
	//include the real one once done

	struct BoneInfo
	{
		int id{};
		glm::mat4 offset;
	};
	struct Key_Position {
		glm::vec3 pos{};
		float timestamp{};
	};

	struct Key_Rotation {
		glm::quat orientation{};
		float timestamp{};
	};

	struct Key_Scale {
		glm::vec3 scale{};
		float timestamp{};
	};



	class Bone {
	private:
		std::vector<Key_Position> positions;
		std::vector<Key_Rotation> rotations;
		std::vector<Key_Scale> scales;

		glm::mat4 local_tform{};
	public:
		//Bone(std::string const& n, const aiNodeAnim* channel);

		std::string name{};
		void Update(float anim_time);
		glm::mat4 const& Get_LocalTform() const {
			return local_tform;
		}
	private:
		float GetInterpValue(float last_time, float next_time, float anim_time);

		glm::mat4 InterpolatePosition(float anim_time);
		glm::mat4 InterpolateRotation(float anim_time);
		glm::mat4 InterpolateScale(float anim_time);

		unsigned int GetKeyPositionIdx(float time);
		unsigned int GetKeyRotationIdx(float time);
		unsigned int GetKeyScaleIdx(float time);
	};

	class Animation {
	public:
		//also for now just hack it and re-import the .fbx file for animation
		//void Init(const char* file, SliceEngineTypes::Model& mdl);
		std::string animName;
		float duration{};
		int ticks_per_second{};
		std::vector<Bone> bones;
		/*
		* tbh this one is closer to a ref/copy of the one in model
		* main reason for having this is so that animator can access it,
		* and accessing bones via aiAnimation.mChannels can sometimes capture missing bones
		*/
		std::unordered_map<std::string, BoneInfo> bone_map;

		//void ReadBones(aiAnimation const* anim, Model& mdl);
		//void ReadHierachyData(AssimpNodeData& dest, const aiNode* src);

		SliceEngineTypes::ModelNode root;
		glm::mat4 globalinv;
	};



	

	struct animatorEntity {};

	class AnimatorSystem : BaseSystem<animatorEntity, Animator>
	{
	public:
		AnimatorSystem();
		void EntityOnEnter(entt::registry& reg, entt::entity entity) override;
		void EntityOnExit(entt::registry& reg, entt::entity entity) override;
		void EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt) override;
		void UpdateAnimation(float dt);
		void PlayAnimation(Animation*);
		void CalculateBoneTransform(const SliceEngineTypes::ModelNode& node, glm::mat4 const& parent_tform);

		std::vector<glm::mat4> const& GetFinalTform() const 
		{
			return final_tforms;
		}
		float current_time{};
	private:
		std::vector<glm::mat4> final_tforms;
		Animation* curr_anim{};
	};

	

}




#endif
