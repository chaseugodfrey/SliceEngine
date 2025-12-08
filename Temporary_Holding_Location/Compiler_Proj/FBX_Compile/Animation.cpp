#include "Animation.h"
#include "Assimp_Importer.h"

namespace Geometry {
	constexpr uint16_t version_number = 1;
	constexpr uint64_t i_size = sizeof(unsigned int);
	constexpr uint64_t f_size = sizeof(float);

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

		glm::quat ToGLM(aiQuaternion const& q) {
			return { q.w, q.x,q.y,q.z };
		}
		glm::vec3 ToGLM(aiVector3D const& v) {
			return { v.x,v.y,v.z };
		}
	}

	/*
	* Skeleton
	*/
	void Skeleton::load(const char* file_name) {
		Assimp::Importer importer;
		int remove_flags{};
		remove_flags |= aiComponent_CAMERAS | aiComponent_COLORS | aiComponent_LIGHTS | aiComponent_MATERIALS;

		//remove components from the import itself
		importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, remove_flags);
		//remove lines and points - only triangles allowed
		importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT);


		const aiScene* scene = importer.ReadFile(file_name, aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_RemoveComponent);
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
			std::cout << "Assimp Error: " << importer.GetErrorString() << std::endl;
			return;
		}
		Init(scene);
	}
	void Skeleton::Init(aiScene const* scene) {
		aiNode* node = scene->mRootNode;
		bones.clear();
		bone_map.clear();
		for (int m{}; m < scene->mNumMeshes; ++m) {
			auto const* mesh = scene->mMeshes[m];
			for (int b{}; b < mesh->mNumBones; ++b) {
				auto const* bone = mesh->mBones[b];
				bone_map[bone->mName.C_Str()].offset = AssimpMatToGLM(bone->mOffsetMatrix);
			}
		}
		add_nodebone(node, -1);
		//assert(bones.size() == bone_map.size());
	}
	void Skeleton::add_nodebone(aiNode const* node, int parent_id) {
		Bone newBone{};
		newBone.parentIndex = parent_id;
		newBone.name = node->mName.C_Str();

		int curr_id = bones.size();

		if (bone_map.find(newBone.name) == bone_map.end()) {
			newBone.offset = glm::identity<glm::mat4>();
		} 
		else {
			newBone.offset = bone_map[newBone.name].offset;
		}
		bone_map[newBone.name].idx = curr_id;
		newBone.neutral = AssimpMatToGLM(node->mTransformation);//glm::identity<glm::mat4>();	//dont really need this for now
		bones.emplace_back(newBone);
		for (int c{}; c < node->mNumChildren; ++c) {
			auto const* child = node->mChildren[c];
			add_nodebone(child, curr_id);
		}
	}


	/*
	* Animation
	*/

	void Animation::Init(aiAnimation const* animation, Skeleton const& skeleton, aiScene const* scene) {
		duration = animation->mDuration / animation->mTicksPerSecond;
		fps = 60;
		name = animation->mName.C_Str();
		// 
		TargetAnim(animation, skeleton);

		//temporary

		//go through all the keyframes and set init the non-bone nodes to be local tform
		for (auto& keyframe : boneKeyFrames) {
			if (!keyframe.animated) {
				keyframe.transforms.clear();
				keyframe.transforms.shrink_to_fit();
			}
		}
		
	}

	void Animation::TargetAnim(aiAnimation const* anim, Skeleton const& skeleton) {
		boneKeyFrames.resize(skeleton.bones.size());
		num_frames = (unsigned int)(duration * (float)(fps)) + 1;
		for (auto& keyframes : boneKeyFrames) {
			keyframes.transforms.resize(num_frames);
		}
		for (int b{}; b < anim->mNumChannels; ++b) {
			auto const* channel = anim->mChannels[b];
			//get correct index in bonekeyframe based on bone name
			std::string nodeName = channel->mNodeName.C_Str();
			unsigned int idx = skeleton.bone_map.at(nodeName).idx;
			auto& keyframes = boneKeyFrames[idx];
			keyframes.animated = true;

			//convert assimp's keyframe system to a frame based animation
			//loop through pos
			//init starting frame
			const float ticks_per_frame = anim->mTicksPerSecond / fps;
			float pos_time{ 0 }, scale_time{ 0 }, rot_time{ 0 };
			unsigned int pos_idx{}, scale_idx{}, rot_idx{};
			for (int frame = 0; frame < num_frames; ++frame) {
				//position
				if (pos_idx < channel->mNumPositionKeys - 1) {
					auto const& curr_pos = channel->mPositionKeys[pos_idx];
					auto const& next_pos = channel->mPositionKeys[pos_idx + 1];

					float interp = (pos_time - curr_pos.mTime) / (next_pos.mTime - curr_pos.mTime);
					keyframes.transforms[frame].position = glm::mix(ToGLM(curr_pos.mValue), ToGLM(next_pos.mValue), interp);

					pos_time += ticks_per_frame;
					while (pos_time >= channel->mPositionKeys[pos_idx + 1].mTime && pos_idx < channel->mNumPositionKeys - 1) {
						++pos_idx;
					}
				}
				else if (pos_idx == channel->mNumPositionKeys - 1) {
					keyframes.transforms[frame].position = ToGLM(channel->mPositionKeys[pos_idx].mValue);
				}
				//scale
				if (scale_idx < channel->mNumScalingKeys - 1) {
					auto const& curr_scale = channel->mScalingKeys[scale_idx];
					auto const& next_scale = channel->mScalingKeys[scale_idx + 1];

					float interp = (scale_time - curr_scale.mTime) / (next_scale.mTime - curr_scale.mTime);
					keyframes.transforms[frame].scale = glm::mix(ToGLM(curr_scale.mValue), ToGLM(next_scale.mValue), interp);

					scale_time += ticks_per_frame;
					while (scale_time >= channel->mScalingKeys[scale_idx + 1].mTime && scale_idx < channel->mNumScalingKeys - 1) {
						++scale_idx;
					}
				}
				else if (scale_idx == channel->mNumScalingKeys - 1) {
					keyframes.transforms[frame].scale = ToGLM(channel->mScalingKeys[scale_idx].mValue);
				}

				//rotation
				if (rot_idx < channel->mNumRotationKeys - 1) {
					auto const& curr_rot = channel->mRotationKeys[rot_idx];
					auto const& next_rot = channel->mRotationKeys[rot_idx + 1];

					float interp = (rot_time - curr_rot.mTime) / (next_rot.mTime - curr_rot.mTime);
					keyframes.transforms[frame].rotation = glm::normalize(glm::slerp(ToGLM(curr_rot.mValue), ToGLM(next_rot.mValue), interp));

					rot_time += ticks_per_frame;
					while (rot_time >= channel->mRotationKeys[rot_idx + 1].mTime && rot_idx < channel->mNumRotationKeys-1) {
						++rot_idx;
					}
				}
				else if (rot_idx == channel->mNumRotationKeys - 1) {
					keyframes.transforms[frame].rotation = ToGLM(channel->mRotationKeys[rot_idx].mValue);
				}
			}
		}
	}


	void Animation::UpdateTransforms(std::vector<glm::mat4>& final_tforms, float time, Skeleton const& skeleton) const {
		
		float frameTime = time * fps;
		int frame0 = (int)frameTime;
		int frame1 = (frame0 + 1) % num_frames;	//lerp back to 0 is somehow this goes to max(it shouldnt)
		float interp = frameTime - frame0;
		for (int i = 0; i < boneKeyFrames.size(); ++i) {
			auto& keyframe = boneKeyFrames[i];
			glm::mat4 local_tform;
			int parent = skeleton.bones[i].parentIndex;
			if (keyframe.animated) {
				auto const& local = keyframe.transforms[frame0];//Transform::Blend(keyframe.transforms[frame0], keyframe.transforms[frame1], interp);
				local_tform = local.ToMatrix();
			}
			else {
				local_tform = skeleton.bones[i].neutral;
			}
			
			if (parent < 0) {
				final_tforms[i] = local_tform;
			}
			else {
				final_tforms[i] = final_tforms[parent] * local_tform;
			}
		}

	}

	void Animation::ApplyInverseBind(std::vector<glm::mat4>& final_tforms, Skeleton const& skeleton) const {
		//inverse bind
		for (int i = 0; i < skeleton.bones.size(); ++i) {
			final_tforms[i] *= skeleton.bones[i].offset;
		}
	}


	void AnimationPackage::Load(const char* file_name, Skeleton const& skeleton) {
		animations.clear();

		Assimp::Importer importer;
		int remove_flags{};
		remove_flags |= aiComponent_CAMERAS | aiComponent_COLORS | aiComponent_LIGHTS | aiComponent_MATERIALS;

		//remove components from the import itself
		importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, remove_flags);
		//remove lines and points - only triangles allowed
		importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT);


		const aiScene* scene = importer.ReadFile(file_name, aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_RemoveComponent);
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
			std::cout << "Assimp Error: " << importer.GetErrorString() << std::endl;
			return;
		}
		for (int i = 0; i < scene->mNumAnimations; ++i) {
			auto* anim = scene->mAnimations[i];
			animations.push_back(Animation());
			animations.back().Init(anim, skeleton, scene);
		}
	}

	void AnimationPlayer::Play(unsigned int idx) {
		curr_anim = idx;
		current_time = 0.f;
		final_transforms.resize(MAX_BONES);
		for (auto& m : final_transforms) {
			m = glm::identity<glm::mat4>();
		}
	}
	void AnimationPlayer::Update(float dt) {
		current_time += dt;

		auto const& anim = animations->animations[curr_anim];
	
		while (current_time > anim.duration) {
			current_time -= anim.duration;
			curr_anim = (curr_anim + 1) % animations->animations.size();
			if (anim.duration <= 0.f) {
				return;
			}
		}

		anim.UpdateTransforms(final_transforms, current_time, *skeleton);
		//Set node's values
		int idx = 0;
		SetNodeTransform(*root_node, idx);

		anim.ApplyInverseBind(final_transforms, *skeleton);
	}

	void AnimationPlayer::SetNodeTransform(Node& node, int& index) {
		//need to do inverse local tform for 
		node.local_tform = final_transforms[index];

		for (auto& c : node.children) {
			SetNodeTransform(c, ++index);
		}
	}

	/*
	* Serialization
	*/

	void Skeleton::Save_Skeleton(const char* file) {
		std::filesystem::path file_path(file);
		std::filesystem::create_directories(file_path.parent_path());

		constexpr uint64_t header_size = 3 + sizeof(version_number) + sizeof(uint64_t);

		//Get total size of animations
		uint64_t buffer_size = get_skeleton_size();

		char* const buffer = new char[buffer_size + header_size];
		uint64_t offset{};
		//write header first
		*(buffer + offset) = 'S'; offset += 1;
		*(buffer + offset) = 'K'; offset += 1;
		*(buffer + offset) = 'L'; offset += 1;
		memcpy(buffer + offset, &version_number, sizeof(version_number)); offset += sizeof(version_number);
		memcpy(buffer + offset, &buffer_size, sizeof(uint64_t)); offset += sizeof(uint64_t);

		//write payload
		pack_data(buffer, offset);
		
		//write binary blob to file
		std::fstream fs(file, std::ios::binary | std::ios::out);
		if (!fs.good()) {
			delete[] buffer;
			return;
		}

		fs.write(buffer, buffer_size + header_size);
		fs.close();

		delete[] buffer;
	}

	bool Skeleton::Load_Skeleton(const char* file) {
		if (!std::filesystem::exists(file)) {
			return false;
		}
		//write binary blob to file
		std::fstream fs(file, std::ios::binary | std::ios::in);

		if (!fs.good()) {
			std::cout << "Unable to open file: " << file << std::endl;
			return false;
		}

		constexpr uint64_t header_size = 3 + sizeof(version_number) + sizeof(uint64_t);
		char header_buffer[header_size]{};
		try {
			fs.read(header_buffer, header_size);
		}
		catch (...) {
			std::cout << "Error reading file: " << file << std::endl;
			fs.close();
			return false;
		}

		if (fs.fail() || fs.eof()) {
			std::cout << "Unknown file format: " << file << std::endl;
			fs.close();
			return false;
		}

		if (header_buffer[0] != 'S' || header_buffer[1] != 'K' || header_buffer[2] != 'L') {
			std::cout << "Not a proper skeleton file: " << file << std::endl;
			fs.close();
			return false;
		}

		auto vers = version_number;
		vers = *((decltype(version_number)*)(header_buffer + 3));
		if (vers != version_number) {
			std::cout << "Wrong version, please recompile: " << file << std::endl;
			fs.close();
			return false;
		}

		//finally, get the file size
		uint64_t buffer_size = *((uint64_t*)(header_buffer + 3 + sizeof(version_number)));

		char* const buffer = new char[buffer_size];
		uint64_t offset{};

		fs.read(buffer, buffer_size);

		fs.close();

		unpack_data(buffer, offset);

		delete[] buffer;

		return true;
	}

	uint64_t Skeleton::get_skeleton_size() const {
		uint64_t size{};

		size += i_size + name.size() + i_size;	//name, bone count
		for (auto const& bone : bones) {
			size += bone.get_bone_size();
		}

		return size;
	}

	uint64_t Bone::get_bone_size() const {
		uint64_t size{};

		size += i_size + name.size() + i_size;	//name, parent index
		size += sizeof(glm::mat4) * 2;		//offset, neutral

		return size;
	}

	void Skeleton::pack_data(char* const buffer, uint64_t& offset) const {
		uint32_t source{};
		//name
		source = name.size();
		memcpy(buffer + offset, &source, i_size); offset += i_size;
		memcpy(buffer + offset, name.data(), source); offset += source;
		//bones
		source = bones.size();
		memcpy(buffer + offset, &source, i_size); offset += i_size;
		for (auto const& bone : bones) {
			bone.pack_data(buffer, offset);
		}
	}

	void Skeleton::unpack_data(char const* const buffer, uint64_t& offset) {
		uint32_t dest{};
		memcpy(&dest, buffer + offset, i_size); offset += i_size;
		name.resize(dest);
		memcpy(name.data(), buffer + offset, dest); offset += dest;
		memcpy(&dest, buffer + offset, i_size); offset += i_size;
		bones.resize(dest);

		for (auto& bone : bones) {
			bone.unpack_data(buffer, offset);
		}
		//create bonemap
		for (unsigned int i{}; i < bones.size(); ++i) {
			auto const& bone = bones[i];
			bone_map[bone.name].idx = i;
		}
	}

	void Bone::pack_data(char* const buffer, uint64_t& offset) const {
		uint32_t source{};
		//name
		source = name.size();
		memcpy(buffer + offset, &source, i_size); offset += i_size;
		memcpy(buffer + offset, name.data(), source); offset += source;
		//parent index
		memcpy(buffer + offset, &parentIndex, i_size); offset += i_size;
		//offset, neutral
		memcpy(buffer + offset, &this->offset, sizeof(glm::mat4)); offset += sizeof(glm::mat4);
		memcpy(buffer + offset, &this->neutral, sizeof(glm::mat4)); offset += sizeof(glm::mat4);
	}

	void Bone::unpack_data(char const* const buffer, uint64_t& offset) {
		uint32_t dest{};
		//name
		memcpy(&dest, buffer + offset, i_size); offset += i_size;
		name.resize(dest);
		memcpy(name.data(), buffer + offset, dest); offset += dest;
		//parent index
		memcpy(&parentIndex, buffer + offset, i_size); offset += i_size;
		//offset, neutral
		memcpy(&this->offset, buffer + offset, sizeof(glm::mat4)); offset += sizeof(glm::mat4);
		memcpy(&this->neutral, buffer + offset , sizeof(glm::mat4)); offset += sizeof(glm::mat4);
	}

	void AnimationPackage::SavePackage(const char* file) {
		std::filesystem::path file_path(file);
		std::filesystem::create_directories(file_path.parent_path());

		constexpr uint64_t header_size = 3 + sizeof(version_number) + sizeof(uint64_t);

		//Get total size of animations
		uint64_t buffer_size = get_package_size();

		char* const buffer = new char[buffer_size + header_size];
		uint64_t offset{};
		//write header first
		*(buffer + offset) = 'A'; offset += 1;
		*(buffer + offset) = 'P'; offset += 1;
		*(buffer + offset) = 'G'; offset += 1;
		memcpy(buffer + offset, &version_number, sizeof(version_number)); offset += sizeof(version_number);
		memcpy(buffer + offset, &buffer_size, sizeof(uint64_t)); offset += sizeof(uint64_t);
	
		//write payload
		pack_data(buffer, offset);

		//write binary blob to file
		std::fstream fs(file, std::ios::binary | std::ios::out);
		if (!fs.good()) {
			delete[] buffer;
			return;
		}

		fs.write(buffer, buffer_size + header_size);
		fs.close();

		delete[] buffer;
	}

	bool AnimationPackage::LoadPackage(const char* file) {
		if (!std::filesystem::exists(file)) {
			return false;
		}
		//write binary blob to file
		std::fstream fs(file, std::ios::binary | std::ios::in);

		if (!fs.good()) {
			std::cout << "Unable to open file: " << file << std::endl;
			return false;
		}

		constexpr uint64_t header_size = 3 + sizeof(version_number) + sizeof(uint64_t);
		char header_buffer[header_size]{};
		try {
			fs.read(header_buffer, header_size);
		}
		catch (...) {
			std::cout << "Error reading file: " << file << std::endl;
			fs.close();
			return false;
		}

		if (fs.fail() || fs.eof()) {
			std::cout << "Unknown file format: " << file << std::endl;
			fs.close();
			return false;
		}

		if (header_buffer[0] != 'A' || header_buffer[1] != 'P' || header_buffer[2] != 'G') {
			std::cout << "Not a proper animation package file: " << file << std::endl;
			fs.close();
			return false;
		}

		auto vers = version_number;
		vers = *((decltype(version_number)*)(header_buffer + 3));
		if (vers != version_number) {
			std::cout << "Wrong version, please recompile: " << file << std::endl;
			fs.close();
			return false;
		}

		//finally, get the file size
		uint64_t buffer_size = *((uint64_t*)(header_buffer + 3 + sizeof(version_number)));

		char* const buffer = new char[buffer_size];
		uint64_t offset{};

		fs.read(buffer, buffer_size);

		fs.close();

		unpack_data(buffer, offset);

		delete[] buffer;

		return true;
	}

	uint64_t AnimationPackage::get_package_size() const {
		uint64_t size{};

		size += i_size + name.size() + i_size;	//name, anim count
		for (auto const& anim : animations) {
			size += anim.get_anim_size();
		}

		return size;
	}

	uint64_t Animation::get_anim_size() const {
		uint64_t size{};
		size += i_size + name.size()	//name
			+ i_size				//fps
			+ f_size				//duration
			+ i_size				//num channels
			+ i_size				//num_frames
			+ i_size;				//sizeof transform

		/*
		* foreach channel
		*	bool
		*	foreach frame
		*		transform
		*/
		size += (sizeof(Transform) * num_frames + 1) * boneKeyFrames.size();

		return size;
	}

	void AnimationPackage::pack_data(char* const buffer, uint64_t& offset) {
		uint32_t source{};
		//name
		source = name.size();
		memcpy(buffer + offset, &source, i_size); offset += i_size;
		memcpy(buffer + offset, name.data(), source); offset += source;
		//anims
		source = animations.size();
		memcpy(buffer + offset, &source, i_size); offset += i_size;
		for (auto& anim : animations) {
			anim.pack_data(buffer, offset);
		}
	}

	void AnimationPackage::unpack_data(char const* const buffer, uint64_t& offset) {
		uint32_t dest{};
		memcpy(&dest, buffer + offset, i_size); offset += i_size;
		name.resize(dest);
		memcpy(name.data(), buffer + offset, dest); offset += dest;
		memcpy(&dest, buffer + offset, i_size); offset += i_size;
		animations.resize(dest);

		for (auto& anim : animations) {
			anim.unpack_data(buffer, offset);
		}
	}

	void Animation::pack_data(char* const buffer, uint64_t& offset) {
		uint32_t source{};
		//name
		source = name.size();
		memcpy(buffer + offset, &source, i_size); offset += i_size;
		memcpy(buffer + offset, name.data(), source); offset += source;
		//fps, duration, num channels, num frames, size of tform
		source = fps;
		memcpy(buffer + offset, &source, i_size); offset += i_size;	//fps
		memcpy(buffer + offset, &duration, f_size); offset += f_size;//duration
		source = boneKeyFrames.size();
		memcpy(buffer + offset, &source, i_size); offset += i_size;//num channels
		source = num_frames;
		memcpy(buffer + offset, &source, i_size); offset += i_size;//num frames
		source = sizeof(Transform);
		memcpy(buffer + offset, &source, i_size); offset += i_size;//tform size
		/*
		* foreach channel
		*	bool
		*	foreach frame
		*		transform
		*/
		for (auto const& keyframe : boneKeyFrames) {
			memcpy(buffer + offset, &keyframe.animated, 1); offset += 1;//bool
			for (auto const& tform : keyframe.transforms) {
				memcpy(buffer + offset, &tform, sizeof(Transform)); offset += sizeof(Transform);
			}
		}
	}

	void Animation::unpack_data(char const* const buffer, uint64_t& offset) {
		uint32_t dest{};
		memcpy(&dest, buffer + offset, i_size); offset += i_size;
		name.resize(dest);
		memcpy(name.data(), buffer + offset, dest); offset += dest;

		//fps, duration, num channels, num frames, size of tform
		memcpy(&fps, buffer + offset, i_size); offset += i_size;
		memcpy(&duration, buffer + offset, f_size); offset += f_size;
		memcpy(&dest, buffer + offset, i_size); offset += i_size;
		boneKeyFrames.resize(dest);
		memcpy(&num_frames, buffer + offset, i_size); offset += i_size;
		memcpy(&dest, buffer + offset, i_size); offset += i_size;

		uint32_t tform_size = dest;
		/*
		* foreach channel
		*	bool
		*	foreach frame
		*		transform
		*/
		for (auto& keyframe : boneKeyFrames) {
			memcpy(&keyframe.animated, buffer + offset, 1); offset += 1;//bool
			if (keyframe.animated) {
				keyframe.transforms.resize(num_frames);
				for (auto& tform : keyframe.transforms) {
					memcpy(&tform, buffer + offset, tform_size); offset += tform_size;
				}
			}
		}
	}


//	void Animation::ReadBones(aiAnimation const* anim) {
//		int num_ch = anim->mNumChannels;
//
//#if !COMPILE_ONLY
//
//		bones.reserve(num_ch);
//		for (int i = 0; i < num_ch; ++i) {
//			auto& ch = anim->mChannels[i];
//			std::string boneName = ch->mNodeName.C_Str();
//			
//			//check incase mdl's bone map has some missing
//			if (mdl_bonemap.find(boneName) == mdl_bonemap.end()) {
//				BoneInfo boneInfo;
//				boneInfo.id = mdl_bonemap.size();
//				//ngl the offset is completely useless for model
//				boneInfo.offset = glm::mat4(1.f);	//but here cant access the offsets ig? honestly no idea why this is here when offset dosent exist
//				mdl_bonemap[boneName] = boneInfo;
//			}
//			bones.emplace_back(Bone{ boneName, ch });
//		}
//		bone_map = mdl_bonemap;
//#endif
//	}
//
//	void Animation::ReadHierachyData(AssimpNodeData& dest, const aiNode* src) {
//		dest.name = src->mName.C_Str();
//		dest.tform = AssimpMatToGLM(src->mTransformation);
//		dest.children.resize(src->mNumChildren);
//
//		for (int i = 0; i < src->mNumChildren; ++i) {
//			ReadHierachyData(dest.children[i], src->mChildren[i]);
//		}
//	}
//
//
//	/*
//	* Bone
//	*/
//
//	Bone::Bone(std::string const& n, const aiNodeAnim* channel) :
//	name(n), local_tform(1.0f) {
//
//		positions.resize(channel->mNumPositionKeys);
//		rotations.resize(channel->mNumRotationKeys);
//		scales.resize(channel->mNumScalingKeys);
//
//		for (int i = 0; i < channel->mNumPositionKeys; ++i) {
//			auto& pos = channel->mPositionKeys[i].mValue;
//			positions[i].pos = { pos.x, pos.y, pos.z };
//			positions[i].timestamp = channel->mPositionKeys[i].mTime;
//		}
//		for (int i = 0; i < channel->mNumRotationKeys; ++i) {
//			auto& quat = channel->mRotationKeys[i].mValue;
//			rotations[i].orientation = { quat.w, quat.x, quat.y, quat.z };
//			rotations[i].timestamp = channel->mRotationKeys[i].mTime;
//		}
//		for (int i = 0; i < channel->mNumScalingKeys; ++i) {
//			auto& scale = channel->mScalingKeys[i].mValue;
//			scales[i].scale = { scale.x, scale.y, scale.z };
//			scales[i].timestamp = channel->mScalingKeys[i].mTime;
//		}
//	}
//
//	void Bone::Update(float anim_time) {
//		glm::mat4 translation = InterpolatePosition(anim_time);
//		glm::mat4 rotation = InterpolateRotation(anim_time);
//		glm::mat4 scale = InterpolateScale(anim_time);
//		local_tform = translation * rotation * scale;
//	}
//
//	float Bone::GetInterpValue(float last_time, float next_time, float anim_time) {
//		return (anim_time - last_time) / (next_time - last_time);
//	}
//
//	glm::mat4 Bone::InterpolatePosition(float anim_time) {
//		if (positions.size() == 1) {
//			return glm::translate(glm::mat4(1.0f), positions[0].pos);
//		}
//
//		//Get keyframe idx
//		unsigned int idx = GetKeyPositionIdx(anim_time);
//		float interp = GetInterpValue(positions[idx].timestamp, positions[idx + 1].timestamp, anim_time);
//
//		glm::vec3 interp_pos = glm::mix(positions[idx].pos, positions[idx + 1].pos, interp);
//		return glm::translate(glm::mat4(1.0f), interp_pos);
//	}
//	glm::mat4 Bone::InterpolateRotation(float anim_time) {
//		if (rotations.size() == 1) {
//			glm::quat q = glm::normalize(rotations[0].orientation);
//			return glm::toMat4(q);
//		}
//
//		//Get keyframe idx
//		unsigned int idx = GetKeyRotationIdx(anim_time);
//		float interp = GetInterpValue(rotations[idx].timestamp, rotations[idx + 1].timestamp, anim_time);
//
//		glm::quat interp_rot = glm::normalize(glm::slerp(rotations[idx].orientation, rotations[idx + 1].orientation, interp));
//		return glm::toMat4(interp_rot);
//	}
//	glm::mat4 Bone::InterpolateScale(float anim_time) {
//		if (scales.size() == 1) {
//			return glm::scale(glm::mat4(1.0f), scales[0].scale);
//		}
//
//		//Get keyframe idx
//		unsigned int idx = GetKeyScaleIdx(anim_time);
//		float interp = GetInterpValue(scales[idx].timestamp, scales[idx + 1].timestamp, anim_time);
//
//		glm::vec3 interp_scale = glm::mix(scales[idx].scale, scales[idx + 1].scale, interp);
//		return glm::scale(glm::mat4(1.0f), interp_scale);
//	}
//
//	unsigned int Bone::GetKeyPositionIdx(float time) {
//		for (unsigned int i{}; i < positions.size() - 1; ++i) {
//			if (time < positions[i + 1].timestamp) {
//				return i;
//			}
//		}
//		return 0;
//	}
//	unsigned int Bone::GetKeyRotationIdx(float time) {
//		for (unsigned int i{}; i < rotations.size() - 1; ++i) {
//			if (time < rotations[i + 1].timestamp) {
//				return i;
//			}
//		}
//		return 0;
//	}
//	unsigned int Bone::GetKeyScaleIdx(float time) {
//		for (unsigned int i{}; i < scales.size() - 1; ++i) {
//			if (time < scales[i + 1].timestamp) {
//				return i;
//			}
//		}
//		return 0;
//	}
}