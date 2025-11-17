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
			auto material = rend.materialHandle;

			uint64_t shaderID = 9461939409271178249;
			uint64_t modelID = model.getGUID().GetGUID();
			uint64_t meshOffset = rend.meshOffset;
			uint64_t isSkinNMeshStatic = 0;
			if (rend.skinned && !model.get()->is_static)
				isSkinNMeshStatic = 1;

			RenderCmdID key(RCK_MAXBITS, shaderID, modelID, meshOffset, isSkinNMeshStatic);

			RenderBatch& batch = renderCmds[key];

			if (batch.instances.empty())
				batch.mdl = model;
			
			InstanceData data;
			data.albedoTextureHandle = material.get()->albedo.get()->texture_id; // -TODO- Get ARB HAndle
			data.color = glm::vec4(material.get()->color, 1.f);
			data.mdlMtx = Core::GetInstance()->mFactory.mRegistry.get<Transform>(entity).transform;
			data.metallic = material.get()->metallic;
			data.roughness = material.get()->roughness;
			data.entityID = static_cast<unsigned int>(entity);

			batch.instances.push_back(data);

			// -TODO- IF cast Shadows
			ShadowRenderCmdID sKey(modelID, meshOffset, isSkinNMeshStatic);
			ShadowRenderBatch& sBatch = shadowRenderCmds[sKey];
			if (sBatch.instances.empty())
				sBatch.mdl = model;
			ShadowInstanceData sData;
			sData.mdlMtx = data.mdlMtx;
			sData.entityID = data.entityID;
			sBatch.instances.push_back(sData);
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
		auto core = Core::GetInstance();
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
					uniformLoc = glGetUniformLocation(mShader, "M");
					glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, &i.mdlMtx[0][0]);

					uniformLoc = glGetUniformLocation(mShader, "skinned");
					if (id.isSkinAndStaticMesh) {
						glUniform1ui(uniformLoc, 1);
						auto const& bone = core->GetRegistry().get<Bone>(static_cast<Entity>(i.entityID));
						Entity root_entity = bone.skeleton_root;
						if (core->GetRegistry().any_of<Animator>(root_entity)) {

							auto const& animator = core->GetRegistry().get<Animator>(root_entity);

							uniformLoc = glGetUniformLocation(mShader, "final_bones_matrices");
							glUniformMatrix4fv(uniformLoc, MAX_BONES, false, glm::value_ptr(animator.GetFinalTform().data()[0]));

							glm::mat4 inverse_root = animator.inverse_map.at(bone.frame_idx);
							uniformLoc = glGetUniformLocation(mShader, "inverse_root");
							glUniformMatrix4fv(uniformLoc, 1, false, glm::value_ptr(inverse_root[0]));
						}
						else {
							//	SLICE_LOG_ERROR("Invalid root entity for bone component when rendering");
						}
					}
					else {
						glUniform1ui(uniformLoc, 0);
					}


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
					uniformLoc = glGetUniformLocation(mShader, "M");
					glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, &i.mdlMtx[0][0]);
					uniformLoc = glGetUniformLocation(mShader, "aGID");
					glUniform1ui(uniformLoc, static_cast<unsigned int>(i.entityID));
					uniformLoc = glGetUniformLocation(mShader, "uRoughness");
					glUniform1f(uniformLoc, i.roughness);
					uniformLoc = glGetUniformLocation(mShader, "uMetallic");
					glUniform1f(uniformLoc, i.metallic);
					uniformLoc = glGetUniformLocation(mShader, "uColor");
					glUniform3f(uniformLoc, i.color.r, i.color.g, i.color.b);

					glBindTextureUnit(0, i.albedoTextureHandle);


					uniformLoc = glGetUniformLocation(mShader, "skinned");
					if (id.dat[RCK_IS_SKIN_AND_MESH_STATIC]) {
						glUniform1ui(uniformLoc, 1);
						auto const& bone = core->GetRegistry().get<Bone>(static_cast<Entity>(i.entityID));
						Entity root_entity = bone.skeleton_root;
						if (core->GetRegistry().any_of<Animator>(root_entity)) {

							auto const& animator = core->GetRegistry().get<Animator>(root_entity);

							uniformLoc = glGetUniformLocation(mShader, "final_bones_matrices");
							glUniformMatrix4fv(uniformLoc, MAX_BONES, false, glm::value_ptr(animator.GetFinalTform().data()[0]));

							glm::mat4 inverse_root = animator.inverse_map.at(bone.frame_idx);
							uniformLoc = glGetUniformLocation(mShader, "inverse_root");
							glUniformMatrix4fv(uniformLoc, 1, false, glm::value_ptr(inverse_root[0]));
						}
						else {
							//	SLICE_LOG_ERROR("Invalid root entity for bone component when rendering");
						}
					}
					else {
						glUniform1ui(uniformLoc, 0);
					}
					glDrawElements(mesh.drawMode, mesh.drawCnt, GL_UNSIGNED_INT, nullptr);
				}
			}
		}
	}
}