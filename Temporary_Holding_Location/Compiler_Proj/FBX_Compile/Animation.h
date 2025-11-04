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
	//class Model;
	//class Bone;
	////used to keep track when bones r repeated
	//struct BoneInfo {
	//	int id{};
	//	glm::mat4 offset;
	//};
	////extracted replica of scene hierachy in assimp
	////Todo: figure out how to process this into a resource file and fit it into engine
	//struct AssimpNodeData {
	//	glm::mat4 tform;
	//	std::string name;
	//	std::vector<AssimpNodeData> children;
	//};

	////probably a good idea to think whether an animation should be attached to model or can be independent so long as bones match
	////currently leaning towards the 2nd
	//class Animation {
	//public:
	//	//also for now just hack it and re-import the .fbx file for animation
	//	void Init(const char* file);

	//	float duration{};
	//	int ticks_per_second{};
	//	std::vector<Bone> bones;
	//	/* 
	//	* tbh this one is closer to a ref/copy of the one in model
	//	* main reason for having this is so that animator can access it,
	//	* and accessing bones via aiAnimation.mChannels can sometimes capture missing bones
	//	*/
	//	std::unordered_map<std::string, BoneInfo> bone_map;

	//	void ReadBones(aiAnimation const* anim);
	//	void ReadHierachyData(AssimpNodeData& dest, const aiNode* src);

	//	AssimpNodeData root;
	//	glm::mat4 globalinv;
	//};

	//class Animator {
	//public:
	//	Animator();
	//	void UpdateAnimation(float dt);
	//	void PlayAnimation(Animation*);
	//	void CalculateBoneTransform(const AssimpNodeData& node, glm::mat4 const& parent_tform);

	//	std::vector<glm::mat4> const& GetFinalTform() const {
	//		return final_tforms;
	//	}
	//	float current_time{};
	//private:
	//	std::vector<glm::mat4> final_tforms;
	//	Animation* curr_anim{};
	//};

	//struct Key_Position {
	//	glm::vec3 pos{};
	//	float timestamp{};
	//};

	//struct Key_Rotation {
	//	glm::quat orientation{};
	//	float timestamp{};
	//};

	//struct Key_Scale {
	//	glm::vec3 scale{};
	//	float timestamp{};
	//};

	//class Bone {
	//private:
	//	std::vector<Key_Position> positions;
	//	std::vector<Key_Rotation> rotations;
	//	std::vector<Key_Scale> scales;

	//	glm::mat4 local_tform{};
	//public:
	//	Bone(std::string const& n, const aiNodeAnim* channel);

	//	std::string name{};
	//	void Update(float anim_time);
	//	glm::mat4 const& Get_LocalTform() const {
	//		return local_tform;
	//	}
	//private:
	//	float GetInterpValue(float last_time, float next_time, float anim_time);

	//	glm::mat4 InterpolatePosition(float anim_time);
	//	glm::mat4 InterpolateRotation(float anim_time);
	//	glm::mat4 InterpolateScale(float anim_time);

	//	unsigned int GetKeyPositionIdx(float time);
	//	unsigned int GetKeyRotationIdx(float time);
	//	unsigned int GetKeyScaleIdx(float time);
	//};

	struct Bone {
		int parentIndex{ -1 };
		glm::mat4 offset{glm::identity<glm::mat4>()};
		glm::mat4 neutral{ glm::identity<glm::mat4>()};
		std::string name{};

		uint64_t get_bone_size() const;
		void pack_data(char* const, uint64_t& offset) const;
		void unpack_data(char const* const, uint64_t& offset);
	};

	/*
	* treat the entire tree as a skeleton
	*/
	struct BoneInfo {
		glm::mat4 offset;
		unsigned int idx;
	};

	struct Skeleton {
		std::vector<Bone> bones{};
		std::unordered_map<std::string, BoneInfo> bone_map;
		void load(const char*);

		std::string name{};
		void Save_Skeleton(const char*);
		bool Load_Skeleton(const char*);
	private:
		void Init(aiScene const* scene);
		void add_nodebone(aiNode const*, int parenht);
		uint64_t get_skeleton_size() const;
		void pack_data(char* const, uint64_t& offset) const;
		void unpack_data(char const* const, uint64_t& offset);
	};

	struct Transform {
		glm::vec3 scale{ 1.f,1.f,1.f };
		glm::vec3 position{ 0.f,0.f,0.f };
		glm::quat rotation{ glm::quat_identity<float, glm::packed_highp>() };
		
		void SetIdentity() {
			scale = { 1.f,1.f,1.f };
			position = { 0.f,0.f,0.f };
			rotation = glm::quat_identity<float, glm::packed_highp>();
		}
		glm::mat4 ToMatrix() const {
			auto translate = glm::translate(glm::mat4(1.0f), position);
			auto rotate = glm::toMat4(rotation);
			auto scaling = glm::scale(glm::mat4(1.0f), scale);
			return translate * rotate * scaling;
		};

		static Transform Blend(Transform const& lhs, Transform const& rhs, float inter) {
			Transform blended;
			blended.position = glm::mix(lhs.position, rhs.position, inter);
			blended.scale = glm::mix(lhs.scale, rhs.scale, inter);
			blended.rotation = glm::normalize(glm::slerp(lhs.rotation, rhs.rotation, inter));
			return blended;
		}
	};

	struct BoneKeyFrames{
		std::vector<Transform> transforms;
		bool animated{ false };
	};

	struct Animation {
		std::string name{};
		unsigned int fps{ 60 };
		float duration{};
		unsigned int num_frames{ 0 };

		std::vector<BoneKeyFrames> boneKeyFrames;	//follows the order of bones in skeleton
		void Init(aiAnimation const* scene, Skeleton const& skeleton, aiScene const*);
		void TargetAnim(aiAnimation const*, Skeleton const& skeleton);

		void UpdateTransforms(std::vector<glm::mat4>&, float time, Skeleton const&) const;
		void ApplyInverseBind(std::vector<glm::mat4>&, Skeleton const&) const;

		uint64_t get_anim_size() const;
		void pack_data(char* const buffer, uint64_t& offset);
		void unpack_data(char const* const buffer, uint64_t& offset);
	};

	struct AnimationPackage {
		std::vector<Animation> animations;
		void Load(const char* file, Skeleton const& skeleton);

		//Create anim package file
		bool LoadPackage(const char* file);
		void SavePackage(const char* file);
	
		std::string name;
	private:
		uint64_t get_package_size() const;
		void pack_data(char* const buffer, uint64_t& offset);
		void unpack_data(char const* const buffer, uint64_t& offset);
	};

	struct Node;
	struct AnimationPlayer {
	private:
		unsigned int curr_anim{};
		float current_time{};
	public:
		AnimationPackage* animations{};
		Skeleton* skeleton{};
		Node* root_node{};
		std::vector<glm::mat4> final_transforms{ MAX_BONES };

		void Play(unsigned int idx);
		void Update(float dt);

		void SetNodeTransform(Node&, int& index);
	};
}

#endif