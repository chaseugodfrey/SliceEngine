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
#include <unordered_map>


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
		struct Frame {
			glm::vec3 scale{ 1.f,1.f,1.f };
			glm::vec3 position{ 0.f,0.f,0.f };
			glm::quat rotation{ 1.f, 0.f, 0.f, 0.f };

			void SetIdentity();
			glm::mat4 ToMatrix() const;

			static Frame Blend(Frame const& lhs, Frame const& rhs, float inter);
		};

		struct BoneKeyFrames {
			std::vector<Frame> transforms;
			bool animated{ false };
		};

		struct AnimationKeyFrame
		{
			std::string scriptName{};
			std::string scriptFunc{};

			unsigned int animIdx{};
			unsigned int frameNumber{};

			//AnimationKeyFrame() = default;
		};

		inline void to_json(nlohmann::json& j, const AnimationKeyFrame& f) {
			j = nlohmann::json{
				{"scriptName", f.scriptName},
				{"scriptFunc", f.scriptFunc},
				{"animIdx", f.animIdx},
				{"frameNumber", f.frameNumber}
			};
		}

		inline void from_json(const nlohmann::json& j, AnimationKeyFrame& f) {
			f.scriptName = j.at("scriptName").get<std::string>();
			f.scriptFunc = j.at("scriptFunc").get<std::string>();
			f.animIdx = j.at("animIdx").get<unsigned int>();
			f.frameNumber = j.at("frameNumber").get<unsigned int>();
		}


		struct Animation {
			std::string name{};
			unsigned int fps{ 60 };
			float duration{};
			unsigned int num_frames{ 0 };

			std::vector<BoneKeyFrames> boneKeyFrames;	//follows the order of bones in skeleton

			void UpdateTransforms(std::vector<glm::mat4>&, float time, Skeleton const&) const;
			void UpdateTransformsBlend(std::vector<glm::mat4>&, float time1,float time2,Animation const& anim,float lerpVal, Skeleton const&) const;
			void ApplyParentTransforms(std::vector<glm::mat4>&, Skeleton const&, glm::mat4 const& world) const;
			void ApplyInverseBind(std::vector<glm::mat4>&, Skeleton const&) const;

			void unpack_data(char const* const buffer, uint64_t& offset);
		};

		class AnimationPackage {
		public:
			std::vector<Animation> animations;

			//Create anim package file
			bool LoadAnimPackageResource(std::string const&);

			std::string name{};
		private:
			void unpack_data(char const* const buffer, uint64_t& offset);
		};

		class Sequence
		{
		public:
			std::string name{};
			unsigned int fps{ 60 };
			float duration{1.0f};
			unsigned int num_frames{ 60 };
			std::vector<std::pair<unsigned int, glm::vec3>> transform{};
			std::vector<std::pair<unsigned int, glm::vec3>> rotation{};
			std::vector<std::pair<unsigned int, glm::vec3>> scale{};

			glm::vec3 startPos{};
			glm::vec3 startEuler{};
			glm::vec3 startScale{};

			bool initialised{ false };

			bool LoadAnimResource(SliceEngine::GUID const&, std::string const& name);
			void UpdateTransforms(entt::registry& ,entt::entity& entity, float time);
		};

		class SequencePackage 
		{
		public:
			std::vector<Sequence> animations{};
			std::string name{};

			bool LoadSequencePkgResource(std::string const&);
		};

	}
}

#endif
