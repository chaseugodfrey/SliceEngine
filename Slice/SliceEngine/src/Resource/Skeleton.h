/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			Skeleton.h
 author:		Elton leosantosa
 email:			leosantosa@digipen.edu
 brief:			Loads Skeleton and Animation packages

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#ifndef SKELETON_H
#define SKELETON_H

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <vector>
//#include <GL/glew.h>
#include <string>

/*
* I think can prob set these hard limits for now
* according to quick google search, seems like usually max 4 bone per vert
* and 100 bones per model is prob good nuff
*
* Unity says more then 4 might cook performance
* and some forums usually dont mention more then 100 bones a model
*/
constexpr unsigned char MAX_BONES = 100;
constexpr unsigned char MAX_BONE_INFLUENCE = 4;

namespace SliceEngine
{
	namespace SliceEngineTypes
	{
		struct Bone {
			int parentIndex{ -1 };
			glm::mat4 offset{ glm::identity<glm::mat4>() };
			glm::mat4 neutral{ glm::identity<glm::mat4>() };
			std::string name{};

			void unpack_data(char const* const, uint64_t& offset);
		};

		struct BoneInfo {
			glm::mat4 offset;
			unsigned int idx;
		};

		class Skeleton {
			void unpack_data(char const* const, uint64_t& offset);
		public:
			std::string name{};
			std::vector<Bone> bones{};
			std::unordered_map<std::string, BoneInfo> bone_map;

			bool LoadSkeletonResource(std::string const&);	//this one is new one
		};

		struct Transform {
			glm::vec3 scale{ 1.f,1.f,1.f };
			glm::vec3 position{ 0.f,0.f,0.f };
			glm::quat rotation{ 1.f, 0.f, 0.f, 0.f };

			void SetIdentity();
			glm::mat4 ToMatrix() const;

			static Transform Blend(Transform const& lhs, Transform const& rhs, float inter);
		};

		struct BoneKeyFrames {
			std::vector<Transform> transforms;
			bool animated{ false };
		};
		struct Animation {
			std::string name{};
			unsigned int fps{ 60 };
			float duration{};
			unsigned int num_frames{ 0 };

			std::vector<BoneKeyFrames> boneKeyFrames;	//follows the order of bones in skeleton

			void UpdateTransforms(std::vector<glm::mat4>&, float time, Skeleton const&) const;
			void ApplyParentTransforms(std::vector<glm::mat4>&, Skeleton const&, glm::mat4 const& world) const;
			void ApplyInverseBind(std::vector<glm::mat4>&, Skeleton const&) const;

			void unpack_data(char const* const buffer, uint64_t& offset);
		};

		class AnimationPackage {
		public:
			std::vector<Animation> animations;

			//Create anim package file
			bool LoadAnimPackageResource(const char* file);

			std::string name;
		private:
			void unpack_data(char const* const buffer, uint64_t& offset);
		};
	}
}

#endif
