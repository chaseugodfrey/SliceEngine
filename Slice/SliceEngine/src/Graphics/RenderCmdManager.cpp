#include <pch.h>
#include "RenderCmdManager.h"
#include <glm/glm.hpp>
#include <glm/common.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/euler_angles.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "Core/Core.h"

#include "WorldSpaceGraphicsSystem.h"
#include "CameraSystem.h"
#include "LightingSystem.h"
#include "Physics/PhysicsSystem.h"
#include "Systems/ParticleSystemManager.h"

#include "Resource/Shader.h"
#include "Resource/Model.h"

namespace SliceEngine
{
	RenderCmdManager::RenderCmdManager()
	{
		glCreateBuffers(1, &mTextureVBO);
		glNamedBufferStorage(mTextureVBO, mMaxInstance * sizeof(GLuint64), NULL, GL_DYNAMIC_STORAGE_BIT);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mTextureVBO);
		
		glCreateBuffers(1, &mBasicVBO);
		glNamedBufferStorage(mBasicVBO, mMaxInstance * sizeof(ShadowInstanceData), NULL, GL_DYNAMIC_STORAGE_BIT);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, mBasicVBO);
		
		glCreateBuffers(1, &mDefaultVBO);
		glNamedBufferStorage(mDefaultVBO, mMaxInstance * sizeof(InstanceData), NULL, GL_DYNAMIC_STORAGE_BIT);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, mDefaultVBO);
	}
	RenderCmdManager::~RenderCmdManager()
	{
		glDeleteBuffers(1, &mTextureVBO);
		glDeleteBuffers(1, &mBasicVBO);
		glDeleteBuffers(1, &mDefaultVBO);
	}
	void RenderCmdManager::GatherDrawCalls()
	{
		renderCmds.clear();
		shadowRenderCmds.clear();

		auto core = Core::GetInstance();
		auto view = Core::GetInstance()->GetRegistry().view<renderEntity>(); // renderEntity // visibleEntity
		
		for (auto entity : view)
		{
			auto& rend = core->GetRegistry().get<Renderer>(entity);
			auto model = rend.modelHandle;
			if (!model.IsValid()) return;
			const auto& material = rend.materialHandle.get();

			uint64_t shaderID = 9461939409271178249;// --TODO-- Should be responsibility of material
			RCK_ModelT mdlDet = GetModelDetails(model.getGUID().GetGUID(), rend.meshOffset, rend.skinned && !model.get()->is_static);

			RCK_Size key = MRCK_OPAQUE |
				(static_cast<RCK_Size>(mdlDet) << RCK_ModelOffset);
			
			InstanceData data;
			data.mdlMtx = Core::GetInstance()->mFactory.mRegistry.get<Transform>(entity).transform;
			data.color = glm::vec4(material->color, 1.f);
			data.roughness = material->roughness;
			data.metallic = material->metallic;
			data.texID = GetTextureDetails(material->albedo.get()->bindless_id);
			data.entityID = static_cast<unsigned int>(entity);

			if (rend.castShadow)
			{
				shadowRenderCmds[mdlDet].emplace_back(ShadowInstanceData(data.mdlMtx));
			}

			renderCmds[key].push_back(std::move(data));
		}
	
	}
	void RenderCmdManager::SortTranslucent(Entity camEntity)
	{

	}
	void RenderCmdManager::UseDrawCalls(GLuint mShader, DrawType drawType)
	{
		// Tags I need
			// Cast Shadows
			// Blend Mode: Opaque, Translucent
			// Shading Mode: Lit / unLit
			// Two-Sided?

		// Internal Sort:
			// Opaque - By Model
			// Translucent - By Dist -> then by model if possible

		// SSBO Data
			// Transform
			// Textures Samplers
			// Texture to use
			// Extra Data(?)l9

		switch (drawType)
		{
		case DrawType::DRAW_MODELS:
		{

			for (auto& i : shadowRenderCmds)
			{
				const auto& id = i.first;
				auto& batch = i.second;

				if (batch.empty())
					continue;

				ModelBasic& mdlRef = modelReferences[id];
				auto mdl = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Model>((GUID)mdlRef.mdl);
				auto& mesh = mdl.get()->meshes[mdlRef.meshOffset];
				glBindVertexArray(mesh.vao);

				//if (mdlRef.isSkin)
				{
					//for (auto& i : batch)
					//{
					//	SetModelSkinUniform(mShader, i.mdlMtx, true, i.entityID);
					//
					//}
				}
				//else
				{
					//SetModelSkinUniform(mShader, mdlRef.isSkin, i.entityID);
					glNamedBufferSubData(mBasicVBO, 0, sizeof(ShadowInstanceData) * batch.size(), batch.data());
					glDrawElementsInstanced(mesh.drawMode, mesh.drawCnt, GL_UNSIGNED_INT, nullptr, batch.size());
				}
			}
			break;
		}
		case DrawType::DRAW_OPAQUE:
		{
			for (auto& i : renderCmds)
			{
				const auto& id = i.first;
				auto& batch = i.second;

				if (batch.empty())
					continue;

				// Change Shader // TODO - Currently all Deferred Shader
				//if (currentShader != id.dat[RCK_SHADER])
				//{
				//	currentShader = id.dat[RCK_SHADER];
				//	mShader = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Shader>((GUID)currentShader).get()->s;
				//	//glUseProgram(mShader);
				//}
				RCK_ModelT mdlID = (id >> RCK_ModelOffset) & MRCK_EXTRACT_SHORT;
				ModelBasic& mdlRef = modelReferences[mdlID];
				auto mdl = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Model>((GUID)mdlRef.mdl);
				auto& mesh = mdl.get()->meshes[mdlRef.meshOffset];
				glBindVertexArray(mesh.vao);

				GLint uniformLoc;
				if (mdlRef.isSkin)
				{
					for (auto& i : batch)
					{
						SetModelSkinUniform(mShader, mdlRef.isSkin, i.entityID);
						glNamedBufferSubData(mDefaultVBO, 0, sizeof(InstanceData), &i);
						glDrawElements(mesh.drawMode, mesh.drawCnt, GL_UNSIGNED_INT, nullptr);
					}
				}
				else
				{
					SetModelSkinUniform(mShader, mdlRef.isSkin, 0);
					glNamedBufferSubData(mDefaultVBO, 0, sizeof(InstanceData) * batch.size(), batch.data());
					glDrawElementsInstanced(mesh.drawMode, mesh.drawCnt, GL_UNSIGNED_INT, nullptr, batch.size());
				}
			}
			break;
		}
		}
	}

	void RenderCmdManager::SingleDraw(GLuint mShader, const Entity& entity, DrawType drawType)
	{
		auto core = Core::GetInstance();
		auto& transform = Core::GetInstance()->mFactory.mRegistry.get<Transform>(entity);
		auto& rend = core->GetRegistry().get<Renderer>(entity);
		if (!rend.modelHandle.IsValid()) return;
		auto meshOffset = std::min(rend.meshOffset, static_cast<unsigned char>(rend.modelHandle.get()->meshes.size() - 1));

		auto& mesh = rend.modelHandle.get()->meshes[meshOffset];
		glBindVertexArray(mesh.vao);

		SetModelSkinUniform(mShader, (rend.skinned && !rend.modelHandle.get()->is_static), static_cast<unsigned int>(entity));
		switch (drawType)
		{
		case DrawType::DRAW_MODELS:
		{
			ShadowInstanceData i;
			i.entityID = static_cast<unsigned int>(entity);
			i.mdlMtx = transform.transform;
			glNamedBufferSubData(mBasicVBO, 0, sizeof(ShadowInstanceData), &i);
			break;
		}
		case DrawType::DRAW_OPAQUE:
		{
			const auto& material = rend.materialHandle.get();

			InstanceData data{};
			data.mdlMtx = transform.transform;
			data.color = glm::vec4(material->color, 1.f);
			data.roughness = material->roughness;
			data.metallic = material->metallic;
			data.texID = GetTextureDetails(material->albedo.get()->bindless_id);
			data.entityID = static_cast<unsigned int>(entity);
			glNamedBufferSubData(mDefaultVBO, 0, sizeof(InstanceData), &data);
			break;
		}
		}
		glDrawElements(mesh.drawMode, mesh.drawCnt, GL_UNSIGNED_INT, nullptr);
	}

	void RenderCmdManager::SetModelSkinUniform(GLuint mShader, bool isSkin, unsigned int entityID)
	{
		GLint uniformLoc;
		uniformLoc = glGetUniformLocation(mShader, "skinned");
		if (isSkin && uniformLoc != -1) {
			glUniform1ui(uniformLoc, 1);

			auto core = Core::GetInstance();
			auto const& bone = core->GetRegistry().get<Bone>(static_cast<Entity>(entityID));
			Entity root_entity = bone.skeleton_root;
			if (core->GetRegistry().any_of<Animator>(root_entity)) {

				auto const& animator = core->GetRegistry().get<Animator>(root_entity);
				if (animator.Handle_curr_anim_pkg.IsValid() && animator.Handle_skeleton.IsValid())
				{
					uniformLoc = glGetUniformLocation(mShader, "final_bones_matrices");
					glUniformMatrix4fv(uniformLoc, MAX_BONES, false, glm::value_ptr(animator.GetFinalTform().data()[0]));

					glm::mat4 inverse_root = animator.inverse_map.at(bone.frame_idx);
					uniformLoc = glGetUniformLocation(mShader, "inverse_root");
					glUniformMatrix4fv(uniformLoc, 1, false, glm::value_ptr(inverse_root[0]));
				}
			}
			else {/*SLICE_LOG_ERROR("Invalid root entity for bone component when rendering");*/ }
		}
		else { glUniform1ui(uniformLoc, 0); }
	}
#pragma region Own Stored copy of data
	RenderCmdManager::RCK_ModelT RenderCmdManager::GetModelDetails(uint64_t mdlID, unsigned char meshOffset, bool isSkin)
	{
		MdlFinder mdlFinder{ mdlID, meshOffset };

		auto i = modelToIdx.find(mdlFinder);
		if (i == modelToIdx.end())
		{
			modelToIdx[mdlFinder] = modelReferences.size();
			modelReferences.emplace_back(ModelBasic(mdlID, meshOffset, isSkin));
			i = modelToIdx.find(mdlFinder);
		}
		return i->second;
	}
	unsigned int RenderCmdManager::GetTextureDetails(GLuint64 bindlessID)
	{
		auto dat = textureLoaded.find(bindlessID);
		if (dat == textureLoaded.end())
		{
			unsigned int texIDX = textureList.size();
			textureLoaded.emplace(bindlessID, texIDX);
			textureList.push_back(bindlessID);
			glNamedBufferSubData(mTextureVBO, sizeof(GLuint64) * texIDX, sizeof(GLuint64), &bindlessID);
			return texIDX;
		}
		return dat->second;
	}
#pragma endregion
}