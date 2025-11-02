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
			memcpy(&this->neutral, buffer + offset, sizeof(glm::mat4)); offset += sizeof(glm::mat4);
		}

		/*
		* Animation Package
		*/

		bool AnimationPackage::LoadAnimPackageResource(const char* file) {
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

				final_tforms[i] = local_tform;
			}

		}

		void Animation::ApplyParentTransforms(std::vector<glm::mat4>& final_tforms, Skeleton const& skeleton, glm::mat4 const& world) const {
			final_tforms[0] = world * final_tforms[0];
			for (int i = 1; i < boneKeyFrames.size(); ++i) {
				auto& keyframe = boneKeyFrames[i];
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



		void Transform::SetIdentity() {
			scale = { 1.f,1.f,1.f };
			position = { 0.f,0.f,0.f };
			rotation = { 1.f, 0.f, 0.f, 0.f };
		}

		glm::mat4 Transform::ToMatrix() const {
			auto translate = glm::translate(glm::mat4(1.0f), position);
			auto rotate = glm::toMat4(rotation);
			auto scaling = glm::scale(glm::mat4(1.0f), scale);
			return translate * rotate * scaling;
		}

		Transform Transform::Blend(Transform const& lhs, Transform const& rhs, float inter) {
			Transform blended;
			blended.position = glm::mix(lhs.position, rhs.position, inter);
			blended.scale = glm::mix(lhs.scale, rhs.scale, inter);
			blended.rotation = glm::normalize(glm::slerp(lhs.rotation, rhs.rotation, inter));
			return blended;
		}
	}
}