#ifndef ANIMATION_IMPORTER
#define	ANIMATION_IMPORTER

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <string>
#include <unordered_map>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>

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

namespace Geometry {
	class Model;
	class Bone;
	//used to keep track when bones r repeated
	struct BoneInfo {
		int id{};
		glm::mat4 offset;
	};
	//extracted replica of scene hierachy in assimp
	//Todo: figure out how to process this into a resource file and fit it into engine
	struct AssimpNodeData {
		glm::mat4 tform;
		std::string name;
		std::vector<AssimpNodeData> children;
	};

	//probably a good idea to think whether an animation should be attached to model or can be independent so long as bones match
	//currently leaning towards the 2nd
	class Animation {
	public:
		//also for now just hack it and re-import the .fbx file for animation
		void Init(const char* file, Model& mdl);

		float duration{};
		int ticks_per_second{};
		std::vector<Bone> bones;
		/* 
		* tbh this one is closer to a ref/copy of the one in model
		* main reason for having this is so that animator can access it,
		* and accessing bones via aiAnimation.mChannels can sometimes capture missing bones
		*/
		std::unordered_map<std::string, BoneInfo> bone_map;

		void ReadBones(aiAnimation const* anim, Model& mdl);
		void ReadHierachyData(AssimpNodeData& dest, const aiNode* src);

		AssimpNodeData root;
		glm::mat4 globalinv;
	};

	class Animator {
	public:
		Animator();
		void UpdateAnimation(float dt);
		void PlayAnimation(Animation*);
		void CalculateBoneTransform(const AssimpNodeData& node, glm::mat4 const& parent_tform);

		std::vector<glm::mat4> const& GetFinalTform() const {
			return final_tforms;
		}
		float current_time{};
	private:
		std::vector<glm::mat4> final_tforms;
		Animation* curr_anim{};
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
		Bone(std::string const& n, const aiNodeAnim* channel);

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


}

#endif