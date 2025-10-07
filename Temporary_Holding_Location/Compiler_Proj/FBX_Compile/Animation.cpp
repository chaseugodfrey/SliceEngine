#include "Animation.h"
#include "Assimp_Importer.h"

namespace Geometry {
	namespace {
		glm::mat4 AssimpMatToGLM(aiMatrix4x4 const& mat) {
			glm::mat4 glm_mat = {
				{mat.a1,mat.b1,mat.c1,mat.d1},
				{mat.a2,mat.b2,mat.c2,mat.d2},
				{mat.a3,mat.b3,mat.c3,mat.d3},
				{mat.a4,mat.b4,mat.c4,mat.d4}
			};
			return glm_mat;
		}
	}

	Animator::Animator() {
		final_tforms.resize(MAX_BONES, glm::mat4(1.0f));
	}

	void Animator::UpdateAnimation(float dt) {
		if (curr_anim) {
			current_time += curr_anim->ticks_per_second * dt;	//needed to convert actual time to animation time
			//for now just inf loop it
			while (current_time > curr_anim->duration) {
				current_time -= curr_anim->duration;
			}
			glm::mat4 id = glm::mat4(1.0f);
			CalculateBoneTransform(curr_anim->root, id);
		}
	}

	void Animator::PlayAnimation(Animation* anim) {
		current_time = 0.f;
		curr_anim = anim;
	}

	/*
	* There is a clash here because this takes into account of hierachy, where the model load dosent
	* probably need to resolve that somehow
	*/
	void Animator::CalculateBoneTransform(AssimpNodeData const& node, glm::mat4 const& parent_tform) {
		std::string node_name = node.name;
		
		glm::mat4 tform = node.tform;
		glm::mat4 bone_tform;
		//node may not be a bone
		Bone* b{};
		for (auto& bone : curr_anim->bones) {
			if (bone.name == node_name) {
				b = &bone;
				break;
			}
		}
		//if (b) {
		//	b->Update(current_time);
		//	//tform = b->Get_LocalTform();
		//	bone_tform = b->Get_LocalTform();
		//	auto& boneinfo = curr_anim->bone_map[node_name];
		//	final_tforms[boneinfo.id] = bone_tform * boneinfo.offset;
		//	tform = bone_tform;
		//}
		//else {
		//	tform = parent_tform * tform;
		//}
		if (b) {
			b->Update(current_time);
			//tform = b->Get_LocalTform();
			//bone_tform = b->Get_LocalTform();
		}

		tform =  parent_tform * tform;

		if (b) {
			auto& boneinfo = curr_anim->bone_map[node_name];
			bone_tform = bone_tform * boneinfo.offset;
			final_tforms[boneinfo.id] =  tform *  boneinfo.offset;
		}

		for (auto& c : node.children) {
			CalculateBoneTransform(c, tform);
		}
	}

	/*
	* Animation
	*/


	void Animation::Init(const char* file, Model& mdl) {
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(file, aiProcess_Triangulate);
		assert(scene && scene->mRootNode);
		auto animation = scene->mAnimations[0];
		duration = animation->mDuration;
		ticks_per_second = animation->mTicksPerSecond;
		ReadHierachyData(root, scene->mRootNode);
		ReadBones(animation, mdl);

		globalinv = glm::inverse(AssimpMatToGLM(scene->mRootNode->mTransformation));
	}

	void Animation::ReadBones(aiAnimation const* anim, Model& mdl) {
		int num_ch = anim->mNumChannels;

#if !COMPILE_ONLY
		auto& mdl_bonemap = mdl.bone_map;

		bones.reserve(num_ch);
		for (int i = 0; i < num_ch; ++i) {
			auto& ch = anim->mChannels[i];
			std::string boneName = ch->mNodeName.C_Str();
			
			//check incase mdl's bone map has some missing
			if (mdl_bonemap.find(boneName) == mdl_bonemap.end()) {
				BoneInfo boneInfo;
				boneInfo.id = mdl_bonemap.size();
				//ngl the offset is completely useless for model
				boneInfo.offset = glm::mat4(1.f);	//but here cant access the offsets ig? honestly no idea why this is here when offset dosent exist
				mdl_bonemap[boneName] = boneInfo;
			}
			bones.emplace_back(Bone{ boneName, ch });
		}
		bone_map = mdl_bonemap;
#endif
	}

	void Animation::ReadHierachyData(AssimpNodeData& dest, const aiNode* src) {
		dest.name = src->mName.C_Str();
		dest.tform = AssimpMatToGLM(src->mTransformation);
		dest.children.resize(src->mNumChildren);

		for (int i = 0; i < src->mNumChildren; ++i) {
			ReadHierachyData(dest.children[i], src->mChildren[i]);
		}
	}


	/*
	* Bone
	*/

	Bone::Bone(std::string const& n, const aiNodeAnim* channel) :
	name(n), local_tform(1.0f) {

		positions.resize(channel->mNumPositionKeys);
		rotations.resize(channel->mNumRotationKeys);
		scales.resize(channel->mNumScalingKeys);

		for (int i = 0; i < channel->mNumPositionKeys; ++i) {
			auto& pos = channel->mPositionKeys[i].mValue;
			positions[i].pos = { pos.x, pos.y, pos.z };
			positions[i].timestamp = channel->mPositionKeys[i].mTime;
		}
		for (int i = 0; i < channel->mNumRotationKeys; ++i) {
			auto& quat = channel->mRotationKeys[i].mValue;
			rotations[i].orientation = { quat.x, quat.y, quat.z, quat.w };
			rotations[i].timestamp = channel->mRotationKeys[i].mTime;
		}
		for (int i = 0; i < channel->mNumScalingKeys; ++i) {
			auto& scale = channel->mScalingKeys[i].mValue;
			scales[i].scale = { scale.x, scale.y, scale.z };
			scales[i].timestamp = channel->mScalingKeys[i].mTime;
		}
	}

	void Bone::Update(float anim_time) {
		glm::mat4 translation = InterpolatePosition(anim_time);
		glm::mat4 rotation = InterpolateRotation(anim_time);
		glm::mat4 scale = InterpolateScale(anim_time);
		local_tform = translation * rotation * scale;
	}

	float Bone::GetInterpValue(float last_time, float next_time, float anim_time) {
		return (anim_time - last_time) / (next_time - last_time);
	}

	glm::mat4 Bone::InterpolatePosition(float anim_time) {
		if (positions.size() == 1) {
			return glm::translate(glm::mat4(1.0f), positions[0].pos);
		}

		//Get keyframe idx
		unsigned int idx = GetKeyPositionIdx(anim_time);
		float interp = GetInterpValue(positions[idx].timestamp, positions[idx + 1].timestamp, anim_time);

		glm::vec3 interp_pos = glm::mix(positions[idx].pos, positions[idx + 1].pos, interp);
		return glm::translate(glm::mat4(1.0f), interp_pos);
	}
	glm::mat4 Bone::InterpolateRotation(float anim_time) {
		if (rotations.size() == 1) {
			glm::quat q = glm::normalize(rotations[0].orientation);
			return glm::toMat4(q);
		}

		//Get keyframe idx
		unsigned int idx = GetKeyRotationIdx(anim_time);
		float interp = GetInterpValue(rotations[idx].timestamp, rotations[idx + 1].timestamp, anim_time);

		glm::quat interp_rot = glm::normalize(glm::slerp(rotations[idx].orientation, rotations[idx + 1].orientation, interp));
		return glm::toMat4(interp_rot);
	}
	glm::mat4 Bone::InterpolateScale(float anim_time) {
		if (scales.size() == 1) {
			return glm::scale(glm::mat4(1.0f), scales[0].scale);
		}

		//Get keyframe idx
		unsigned int idx = GetKeyScaleIdx(anim_time);
		float interp = GetInterpValue(scales[idx].timestamp, scales[idx + 1].timestamp, anim_time);

		glm::vec3 interp_scale = glm::mix(scales[idx].scale, scales[idx + 1].scale, interp);
		return glm::scale(glm::mat4(1.0f), interp_scale);
	}

	unsigned int Bone::GetKeyPositionIdx(float time) {
		for (unsigned int i{}; i < positions.size() - 1; ++i) {
			if (time > positions[i + 1].timestamp) {
				return i;
			}
		}
		return 0;
	}
	unsigned int Bone::GetKeyRotationIdx(float time) {
		for (unsigned int i{}; i < rotations.size() - 1; ++i) {
			if (time > rotations[i + 1].timestamp) {
				return i;
			}
		}
		return 0;
	}
	unsigned int Bone::GetKeyScaleIdx(float time) {
		for (unsigned int i{}; i < scales.size() - 1; ++i) {
			if (time > scales[i + 1].timestamp) {
				return i;
			}
		}
		return 0;
	}
}