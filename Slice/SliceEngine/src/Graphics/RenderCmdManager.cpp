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
			uint64_t modelID = model.getGUID().GetGUID();
			uint64_t meshOffset = std::min(rend.meshOffset, static_cast<unsigned char>(model.get()->meshes.size() - 1));
			bool isSkinNMeshStatic = rend.skinned && !model.get()->is_static;

			RenderCmdID key(RCK_MAXBITS, shaderID, modelID, meshOffset);

			RenderBatch& batch = renderCmds[key];

			if (batch.instances.empty())
			{
				batch.mdl = model;
				batch.isSkin = static_cast<bool>(isSkinNMeshStatic);
			}
			
			InstanceData data;
			data.mdlMtx = Core::GetInstance()->mFactory.mRegistry.get<Transform>(entity).transform;
			data.color = glm::vec4(material->color, 1.f);
			data.roughness = material->roughness;
			data.metallic = material->metallic;
			data.albedoTextureHandle = material->albedo.get()->texture_id; // -TODO- Get ARB HAndle
			data.entityID = static_cast<unsigned int>(entity);

			batch.instances.push_back(std::move(data));

			if (rend.castShadow)
			{
				ShadowRenderCmdID sKey(modelID, meshOffset);
				ShadowRenderBatch& sBatch = shadowRenderCmds[sKey];
				if (sBatch.instances.empty())
				{
					sBatch.mdl = model;
					sBatch.isSkin = static_cast<bool>(isSkinNMeshStatic); // --TODO-- Currently 0 support for shadows lol
				}
				sBatch.instances.emplace_back(ShadowInstanceData(data.mdlMtx, data.entityID));
			}
		}
	}
	void RenderCmdManager::UseDrawCalls(GLuint mShader, bool isForShadows)
	{
		// Tags I need
			// Cast Shadows
			// Blend Mode: Opaque, Translucent
			// Shading Mode: Lit / unLit
			// Two-Sided?

		// Internal Sort:
			// Opaque - By Model (no need by texutres if bindless)
			// Translucent - By Dist -> then by model if possible

		// SSBO Data
			// Transform
			// Textures Samplers
			// Texture to use
			// Extra Data(?)l9
		//uint64_t currentShader = 0;
		//GLuint mShader = 0;

		if (isForShadows)
		{
			for (auto& i : shadowRenderCmds)
			{
				const auto& id = i.first;
				auto& batch = i.second;

				if (batch.instances.empty())
					continue;

				auto& mesh = batch.mdl.get()->meshes[id.meshOffset];
				glBindVertexArray(mesh.vao);

				GLint uniformLoc;
				for (auto& i : batch.instances)
				{
					BasicDrawSettings(mShader, i.mdlMtx, batch.isSkin, i.entityID);

					glDrawElements(mesh.drawMode, mesh.drawCnt, GL_UNSIGNED_INT, nullptr);
				}
			}
		}
		else
		{
			for (auto& i : renderCmds)
			{
				const auto& id = i.first;
				auto& batch = i.second;

				if (batch.instances.empty())
					continue;

				// Change Shader // TODO - Currently all Deferred Shader
				//if (currentShader != id.dat[RCK_SHADER])
				//{
				//	currentShader = id.dat[RCK_SHADER];
				//	mShader = Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Shader>((GUID)currentShader).get()->s;
				//	//glUseProgram(mShader);
				//}
				
				auto& mesh = batch.mdl.get()->meshes[id.dat[RCK_MESH_OFFSET]];
				glBindVertexArray(mesh.vao);

				GLint uniformLoc;
				for (auto& i : batch.instances)
				{
					BasicDrawSettings(mShader, i.mdlMtx, batch.isSkin, i.entityID);

					uniformLoc = glGetUniformLocation(mShader, "aGID");
					glUniform1ui(uniformLoc, static_cast<unsigned int>(i.entityID));
					uniformLoc = glGetUniformLocation(mShader, "uRoughness");
					glUniform1f(uniformLoc, i.roughness);
					uniformLoc = glGetUniformLocation(mShader, "uMetallic");
					glUniform1f(uniformLoc, i.metallic);
					uniformLoc = glGetUniformLocation(mShader, "uColor");
					glUniform3f(uniformLoc, i.color.r, i.color.g, i.color.b);

					glBindTextureUnit(0, i.albedoTextureHandle);

					glDrawElements(mesh.drawMode, mesh.drawCnt, GL_UNSIGNED_INT, nullptr);
				}
			}
		}
	}

	void RenderCmdManager::SingleDraw(GLuint mShader, const Entity& entity, bool isForShadow)
	{
		auto core = Core::GetInstance();
		auto& transform = Core::GetInstance()->mFactory.mRegistry.get<Transform>(entity);
		auto& rend = core->GetRegistry().get<Renderer>(entity);
		if (!rend.modelHandle.IsValid()) return;
		auto meshOffset = std::min(rend.meshOffset, static_cast<unsigned char>(rend.modelHandle.get()->meshes.size() - 1));

		auto& mesh = rend.modelHandle.get()->meshes[meshOffset];
		glBindVertexArray(mesh.vao);

		BasicDrawSettings(mShader, transform.transform, (rend.skinned && !rend.modelHandle.get()->is_static), static_cast<unsigned int>(entity));
		glDrawElements(mesh.drawMode, mesh.drawCnt, GL_UNSIGNED_INT, nullptr);
	}

	void RenderCmdManager::BasicDrawSettings(GLuint mShader, glm::mat4& mdlMtx, bool isSkin, unsigned int entityID)
	{
		GLint uniformLoc;
		uniformLoc = glGetUniformLocation(mShader, "M");
		glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, &mdlMtx[0][0]);

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
}