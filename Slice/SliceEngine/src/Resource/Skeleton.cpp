/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			Skeleton.cpp
 author:		Elton leosantosa
 email:			leosantosa@digipen.edu
 brief:			Loads Skeleton and Animation packages

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#include <pch.h>
#include "Skeleton.h"
#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Serializer/JSONSerializer.h>
#include <Graphics/TransformHelper.h>


#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace {
	//some consts to help typing
	constexpr uint16_t version_number = 1;	//i think having a vers number could be useful, maybe
	constexpr uint64_t i_size = sizeof(unsigned int);
	constexpr uint64_t f_size = sizeof(float);
}

namespace SliceEngine
{
	namespace SliceEngineTypes
	{
		/*
		* Skeleton
		*/
		bool Skeleton::LoadSkeletonResource(std::string const& file) {
			std::ifstream fs(file, std::ios::binary);

			if (!fs)
			{
				SLICE_LOG_WARNING("Unable to open Obj:" + file);
				return false;
			}

			/*
			* Checking for valid header
			*/
			constexpr uint64_t header_size = 3 + sizeof(version_number) + sizeof(uint64_t);
			char header_buffer[header_size]{};
			try {
				fs.read(header_buffer, header_size);
			}
			catch (...) {
				SLICE_LOG_WARNING("Error reading file: " + file);
				fs.close();
				return false;
			}

			if (fs.fail() || fs.eof()) {
				SLICE_LOG_WARNING("Unknown file format: " + file);
				fs.close();
				return false;
			}

			if (header_buffer[0] != 'S' || header_buffer[1] != 'K' || header_buffer[2] != 'L') {
				SLICE_LOG_WARNING("Not a proper skeleton file: " + file);
				fs.close();
				return false;
			}
			auto vers = version_number;
			vers = *((decltype(version_number)*)(header_buffer + 3));
			if (vers != version_number) {
				SLICE_LOG_WARNING("Wrong version, please recompile: " + file);
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
		void Bone::unpack_data(char const* const buffer, uint64_t& offSet) {
			uint32_t dest{};
			//name
			memcpy(&dest, buffer + offSet, i_size); offSet += i_size;
			name.resize(dest);
			memcpy(name.data(), buffer + offSet, dest); offSet += dest;
			//parent index
			memcpy(&parentIndex, buffer + offSet, i_size); offSet += i_size;
			//offset, neutral
			memcpy(&this->offset, buffer + offSet, sizeof(glm::mat4)); offSet += sizeof(glm::mat4);
			memcpy(&this->neutral, buffer + offSet, sizeof(glm::mat4)); offSet += sizeof(glm::mat4);
		}

		bool SequencePackage::LoadSequencePkgResource(std::string const& filename)
		{
			std::ifstream file(filename);
			if (!file.is_open())
			{
				return false;
			}

			nlohmann::json ctrlJson;
			try
			{
				ctrlJson = nlohmann::json::parse(file);
			}
			catch (nlohmann::json::parse_error& e)
			{
				SLICE_LOG_ERROR("Invalid Sequence Pkg JSON file" + std::string(e.what()));

				return false;
			}

			std::vector<std::string> animationNames{};

			animationNames = ctrlJson["Animations"];

			for (std::string anim : animationNames)
			{
				Sequence tmpAnim{};
				tmpAnim.LoadAnimResource(anim);

				animations.push_back(tmpAnim);
			}


			return true;
		}

		bool Sequence::LoadAnimResource(std::string const& animName)
		{
			std::filesystem::path animationsPath = std::filesystem::current_path();

			if (animationsPath.filename() != "Animations")
			{
				animationsPath = animationsPath / "Assets" / "Animations";
			}

			animationsPath = animationsPath / animName;
			if (animationsPath.extension() != ".seq")
			{
				animationsPath += ".seq";
			}


			std::ifstream file(animationsPath);
			if (!file.is_open())
			{
				return false;
			}

			nlohmann::json ctrlJson;
			try
			{
				ctrlJson = nlohmann::json::parse(file);
			}
			catch (nlohmann::json::parse_error& e)
			{
				SLICE_LOG_ERROR("Invalid Anim JSON file" + std::string(e.what()));

				return false;
			}

			name = ctrlJson["Name"];
			fps = ctrlJson["FPS"];
			duration = ctrlJson["Duration"];
			num_frames = ctrlJson["Number of Frames"];
			transform = ctrlJson["Transforms"].get<std::vector<std::pair<unsigned int, glm::vec3>>>();
			rotation = ctrlJson["Rotations"].get<std::vector<std::pair<unsigned int, glm::vec3>>>();
			scale = ctrlJson["Scales"].get<std::vector<std::pair<unsigned int, glm::vec3>>>();

			return true;
		}

		void Sequence::UpdateTransforms(entt::registry& reg,entt::entity& entity, float time)
		{
			Transform& comp = reg.get<Transform>(entity);

			if (!initialised)
			{
				startPos = comp.position;
				startScale = comp.scale;
				startEuler = comp.eulerAnglesHint;

				initialised = true;
			}

			float wrappedTime = std::fmod(time * fps, (float)num_frames);

			auto interpolate = [&](const std::vector<std::pair<unsigned int, glm::vec3>>& keys, glm::vec3 currentVal) -> glm::vec3
				{
					if (keys.empty()) return currentVal;

					auto it1 = std::upper_bound(keys.begin(), keys.end(), wrappedTime,
						[](float val, const std::pair<unsigned int, glm::vec3>& pair) {
							return val < (float)pair.first;
						});

					glm::vec3 v0, v1;
					float t = 0.0f;

					if (it1 == keys.begin()) 
					{
						//v0 = currentVal;
						//v1 = it1->second;
						//t = (it1->first == 0) ? 1.0f : wrappedTime / (float)it1->first;

						float t1 = (float)it1->first;
						if (t1 > 0.0f) {
							float t = wrappedTime / t1;
							return glm::mix(currentVal, it1->second, t);
						}
						return it1->second;
					}
					if (it1 == keys.end()) 
					{
						/*v0 = keys.back().second;
						v1 = currentVal;

						float frameOfLastKey = (float)keys.back().first;
						float timeSinceLastKey = wrappedTime - frameOfLastKey;
						float timeUntilEnd = (float)num_frames - frameOfLastKey;

						if (timeUntilEnd > 0.0f) {
							t = timeSinceLastKey / timeUntilEnd;
						}
						else {
							t = 1.0f;
						}*/
						return keys.back().second;
					}
					//else 
					{

						auto it0 = std::prev(it1);
						v0 = it0->second;
						v1 = it1->second;
						t = (wrappedTime - (float)it0->first) / (float)(it1->first - it0->first);
					}

					return glm::mix(v0, v1, t);
				};

			// Update the components
			comp.position = interpolate(transform, startPos);
			comp.scale = interpolate(scale, startScale);

			glm::vec3 finalEuler = interpolate(rotation, startEuler);
			
			comp.rotation = SliceEngine::Vec3ToQuat(finalEuler);
		}


		/*
		* Animation Package
		*/

		bool AnimationPackage::LoadAnimPackageResource(std::string const& file) {
			std::ifstream fs(file, std::ios::binary);

			if (!fs)
			{
				//SLICE_LOG_WARNING("Unable to open Obj:" + file);
				return false;
			}

			/*
			* Checking for valid header
			*/
			constexpr uint64_t header_size = 3 + sizeof(version_number) + sizeof(uint64_t);
			char header_buffer[header_size]{};
			try {
				fs.read(header_buffer, header_size);
			}
			catch (...) {
				SLICE_LOG_WARNING("Error reading file: " + file);
				SLICE_LOG_WARNING("Error reading file: " + file);
				fs.close();
				return false;
			}

			if (fs.fail() || fs.eof()) {
				SLICE_LOG_WARNING("Unknown file format: " + file);
				fs.close();
				return false;
			}

			if (header_buffer[0] != 'A' || header_buffer[1] != 'P' || header_buffer[2] != 'G') {
				SLICE_LOG_WARNING("Not a proper skeleton file: " + file);
				fs.close();
				return false;
			}
			auto vers = version_number;
			vers = *((decltype(version_number)*)(header_buffer + 3));
			if (vers != version_number) {
				SLICE_LOG_WARNING("Wrong version, please recompile: " + file);
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

		void Animation::UpdateTransforms(std::vector<glm::mat4>& final_tforms, float time, Skeleton const& skeleton) const {

			float frameTime = time * fps;
			int frame0 = (int)frameTime;
			//int frame1 = (frame0 + 1) % num_frames;	//lerp back to 0 is somehow this goes to max(it shouldnt)
			//float interp = frameTime - frame0;
			for (int i = 0; i < boneKeyFrames.size(); ++i) {
				auto& keyframe = boneKeyFrames[i];
				glm::mat4 local_tform;
				//int parent = skeleton.bones[i].parentIndex;
				if (keyframe.animated) {
					auto const& local = keyframe.transforms[frame0];//Transform::Blend(keyframe.transforms[frame0], keyframe.transforms[frame1], interp);
					local_tform = local.ToMatrix();
				}
				else {
					local_tform = skeleton.bones[i].neutral;
				}

				final_tforms[i] = local_tform;
			}

		}

		void Animation::UpdateTransformsBlend(std::vector<glm::mat4>& final_tforms, float time1, float time2, Animation const& anim, float lerpVal, Skeleton const& skeleton) const
		{
			float frameTime1 = time1 * fps;
			float frameTime2 = time2 * anim.fps;
			int frame0_1 = (int)frameTime1;
			int frame1_1 = (frame0_1 + 1) % num_frames;	//lerp back to 0 is somehow this goes to max(it shouldnt)
			int frame0_2 = (int)frameTime2;
			int frame1_2 = (frame0_2 + 1) % anim.num_frames;	//lerp back to 0 is somehow this goes to max(it shouldnt)

			for (int i = 0; i < boneKeyFrames.size(); ++i) 
			{
				auto& keyframe1 = boneKeyFrames[i];
				auto& keyframe2 = anim.boneKeyFrames[i];
				glm::mat4 local_tform;
				//int parent = skeleton.bones[i].parentIndex;
				if (keyframe1.animated) 
				{
					auto const& local1 = keyframe1.transforms[frame1_1];//Transform::Blend(keyframe.transforms[frame0], keyframe.transforms[frame1], interp);
					auto const& local2 = keyframe2.transforms[frame1_2];//Transform::Blend(keyframe.transforms[frame0], keyframe.transforms[frame1], interp);
					local_tform = Frame::Blend(local1, local2, lerpVal).ToMatrix();
				}
				else {
					local_tform = skeleton.bones[i].neutral;
				}

				final_tforms[i] = local_tform;
			}
		}

		void Animation::ApplyParentTransforms(std::vector<glm::mat4>& final_tforms, Skeleton const& skeleton, glm::mat4 const& world) const {
			final_tforms[0] = world * final_tforms[0];
			for (int i = 1; i < boneKeyFrames.size(); ++i) {
				//auto& keyframe = boneKeyFrames[i];
				int parent = skeleton.bones[i].parentIndex;

				final_tforms[i] = final_tforms[parent] * final_tforms[i];

			}
		}

		void Animation::ApplyInverseBind(std::vector<glm::mat4>& final_tforms, Skeleton const& skeleton) const {
			//inverse bind
			for (int i = 0; i < skeleton.bones.size(); ++i) {
				final_tforms[i] *= skeleton.bones[i].offset;
			}
		}



		void Frame::SetIdentity() {
			scale = { 1.f,1.f,1.f };
			position = { 0.f,0.f,0.f };
			rotation = { 1.f, 0.f, 0.f, 0.f };
		}

		glm::mat4 Frame::ToMatrix() const {
			auto translate = glm::translate(glm::mat4(1.0f), position);
			auto rotate = glm::toMat4(rotation);
			auto scaling = glm::scale(glm::mat4(1.0f), scale);
			return translate * rotate * scaling;
		}

		Frame Frame::Blend(Frame const& lhs, Frame const& rhs, float inter) {
			Frame blended;
			blended.position = glm::mix(lhs.position, rhs.position, inter);
			blended.scale = glm::mix(lhs.scale, rhs.scale, inter);
			blended.rotation = glm::normalize(glm::slerp(lhs.rotation, rhs.rotation, inter));
			return blended;
		}

		

	}
}