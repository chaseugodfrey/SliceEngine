/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			WorldSpaceGraphicsSystem.cpp
 author:		Won Yu Xuan Rainne
 email:			won.m@digipen.edu
 brief:			Handles draw calls, and maybe spatial partioning of all renderEntities

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#include <pch.h>

#include "Resource/ResourceManager.h"
#include "Resource/Shader.h"
#include "Resource/Material.h"
#include "Resource/Model.h"
#include "Resource/Texture.h"

#include "WorldSpaceGraphicsSystem.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/euler_angles.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "../Core/Core.h"

namespace SliceEngine
{
	// void WorldSpaceGraphicsSystem::Render(GLuint shader, bool withTex)
	// {
	// 	SetShaderAndWTexSettings(shader, withTex);
	//
	// 	//ResetVisibleEntities();
	//
	// 	auto view = Core::GetInstance()->GetRegistry().view<renderEntity>(entt::exclude<PrefabEditingEntity>); // renderEntity // visibleEntity
	// 	for (auto entity : view)
	// 	{
	// 		if (!Core::GetInstance()->GetRegistry().get<Renderer>(entity).componentEnabled) continue;
	//
	// 		EntityDraw(entity);
	// 	}
	// }

	void WorldSpaceGraphicsSystem::SetShaderAndWTexSettings(GLuint shader, bool withTex)
	{
		mShader = shader;
		mHasRenderTexture = withTex;
	}

	void WorldSpaceGraphicsSystem::EntityOnEnter(entt::registry& reg, Entity entity)
	{
		auto rm = Core::GetInstance()->GetResourceManager();
		auto& renderer = reg.get<Renderer>(entity);
		if (!renderer.modelHandle.IsValid())
		{
			renderer.modelHandle = rm->get<SliceEngineTypes::Model>(renderer.modelHandle.mGUID);
			// get model handle
		}

		if (!renderer.materialHandle.IsValid())
		{
			// get material handle
			renderer.materialHandle = rm->get<SliceEngineTypes::Material>(renderer.materialHandle.mGUID);
		}
	}

	void WorldSpaceGraphicsSystem::EntityOnExit(entt::registry& reg, Entity entity)
	{

	}

	void WorldSpaceGraphicsSystem::EntityOnUpdate(entt::registry& reg, Entity entity, float dt)
	{
		//auto& transform = reg.get<Transform>(entity);
		//// -------------------------------------------------------------
		//// Calc the Transformation Matrix
		//// -------------------------------------------------------------
		//glm::mat4x4 M(1.f);
		//M = glm::translate(M, transform.position);
		//M *= glm::eulerAngleXYZ(glm::radians(transform.rotation.x), glm::radians(transform.rotation.y), glm::radians(transform.rotation.z));
		//M = glm::scale(M, transform.scale);
		//
		//transform.transform = M;
		// Transformation code for child - continuing from parent
		//M = glm::translate(M, glm::vec3(2.f, -2.f, 2.f));
		//glm::mat4x4 M2 = glm::eulerAngleXYZ(glm::radians(45.f), glm::radians(0.f), glm::radians(0.f));
		//M2 = glm::scale(M2, glm::vec3(0.5f, 0.5f, 0.5f));
		//M = M2 * M;
		//
		// -------------------------------------------------------------
		// Add entity into spatial partition
		// -------------------------------------------------------------
		// Case 1:
		// [?,?] -> [0,10] (GridNum 11)
		// Since: 100 ~ 109.999/10 = 10
		// And 0 ~ 9.999 /10 = 0
		// Therefore, [0,110) --> [-55,55) + 55 offset -> [0,110) Div Grid Size -> [0,10]
		// 
		// Case 2:
		// [?, ?] -> [0,1] (GridNum 2)
		// Since: 10 ~ 19.999 / 10 = 1
		// And 0 ~ 9.999 /10 = 0
		// Therefore, [0, 20) --> [-10,10) + 10 offset -> [0,20) Div Grid Size --> [0,1]
		//const float gridExtreme = (static_cast<float>(gridNum / 2) + (gridNum % 2) * 0.5f) * gridSize;
		//
		//int xPos{ static_cast<int>(transform.position.x + gridExtreme) / gridNum },
		//	zPos{ static_cast<int>(transform.position.z + gridExtreme) / gridNum };
		//if (xPos < gridNum && xPos > -1 && zPos < gridNum && zPos > -1)
		//	spatialData[xPos * gridNum + zPos].push_back(entity);
		//else
		//	outerSpatial.push_back(entity);
	}
	
	// void WorldSpaceGraphicsSystem::EntityDraw(const Entity& entity)
	// {
	// 	auto core = Core::GetInstance();
	// 	auto& rc = core->GetRegistry().get<Renderer>(entity);
	//
	// 	auto model = rc.modelHandle;		
	// 	if (!model.IsValid()) return;
	//
	// 	// --TODO-- Cursed model Error Checking loading
	// 	auto& mesh = model.get()->meshes[std::min(rc.meshOffset, static_cast<unsigned char>(model.get()->meshes.size() - 1))];
	//
	// 	/*model.meshes[rc.meshOffset];*/
	// 	glBindVertexArray(mesh.vao);
	//
	// 	auto& transform = Core::GetInstance()->mFactory.mRegistry.get<Transform>(entity);
	//
	// 	GLint uniformLoc;
	// 	uniformLoc = glGetUniformLocation(mShader, "M");
	// 	glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, &transform.transform[0][0]);
	// 	if (mHasRenderTexture)
	// 	{
	// 		auto material = rc.materialHandle.get();//rm->get<SliceEngineTypes::Material>(rc.material).get();
	//
	// 		uniformLoc = glGetUniformLocation(mShader, "aGID");
	// 		glUniform1ui(uniformLoc, static_cast<unsigned int>(entity));
	// 		uniformLoc = glGetUniformLocation(mShader, "uRoughness");
	// 		glUniform1f(uniformLoc, material->roughness);
	// 		uniformLoc = glGetUniformLocation(mShader, "uMetallic");
	// 		glUniform1f(uniformLoc, material->metallic);
	// 		uniformLoc = glGetUniformLocation(mShader, "uColor");
	// 		glUniform3f(uniformLoc, material->color.r, material->color.g, material->color.b);
	//
	// 		//auto rm = Core::GetInstance()->GetResourceManager();
	// 		//auto albedoTex = rm->get<SliceEngineTypes::Texture>(material->albedo);
	// 		auto albedoTex = material->albedo;
	//
	// 		//auto roughTex = rm->get<SliceEngineTypes::Texture>(matHandle->roughness);
	//
	// 		// --TODO-- Cursed Texture exist check, Fix Resource Manager
	// 		if (reinterpret_cast<void*>(albedoTex.get()) != (void*)0xdddddddddddddddd)
	// 			glBindTextureUnit(0, albedoTex.get()->texture_id);
	// 		else
	// 			glBindTextureUnit(0, Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::Texture>((GUID)DefaultResourceIDs::COLOR_DEADED_DEFAULT)->texture_id);
	// 	}
	//
	// 	//glDrawElements(handle.get()->drawMode, handle.get()->drawCnt, GL_UNSIGNED_INT, nullptr);
	//
	// 	//glDrawArrays(handle.get()->drawMode, 0, handle.get()->drawCnt);
	//
	// 	/*
	// 	* mesh skinning
	// 	* NOTE: THIS IS TEMPORARY CODE TO SHOW FOR SUBMISSION,
	// 	* NEED TO FIGURE OUT A BETTER WAY TO DO THIS SPLIT
	// 	* 
	// 	* also only gona do this for deferred first just to test
	// 	* 
	// 	* ISSUES:
	// 	* BOTH SKIN AND STATIC MESH USE THE SAME SHADER(UNIFORM BRANCH IN SHADER CODE)
	// 	* EACH MESH SENDS THE ENTIRE SKELETON TRANSFORM TO THE GPU, WHICH MEANS UP TO 100 MAT4 PER MESH TO DRAW(ITS ALOT)
	// 	*/
	// 	uniformLoc = glGetUniformLocation(mShader, "skinned");
	// 	if (rc.skinned && !model.get()->is_static) {
	// 		glUniform1ui(uniformLoc, 1);
	// 		auto const& bone = core->GetRegistry().get<Bone>(entity);
	// 		Entity root_entity = bone.skeleton_root;
	// 		if (core->GetRegistry().any_of<Animator>(root_entity)) {
	//
	// 			auto const& animator = core->GetRegistry().get<Animator>(root_entity);
	//
	// 			// only update if theres a anim pkg and skeleton
	// 			if(animator.Handle_curr_anim_pkg.IsValid() && animator.Handle_skeleton.IsValid())
	// 			{
	// 				uniformLoc = glGetUniformLocation(mShader, "final_bones_matrices");
	// 				glUniformMatrix4fv(uniformLoc, MAX_BONES, false, glm::value_ptr(animator.GetFinalTform().data()[0]));
	//
	// 				glm::mat4 inverse_root = animator.inverse_map.at(bone.frame_idx);
	// 				uniformLoc = glGetUniformLocation(mShader, "inverse_root");
	// 				glUniformMatrix4fv(uniformLoc, 1, false, glm::value_ptr(inverse_root[0]));
	// 			}
	// 		}
	// 		else {
	// 		//	SLICE_LOG_ERROR("Invalid root entity for bone component when rendering");
	// 		}
	// 	}
	// 	else {
	// 		glUniform1ui(uniformLoc, 0);
	// 	}
	//
	//
	//
	// 	glDrawElements(mesh.drawMode, mesh.drawCnt, GL_UNSIGNED_INT, nullptr);
	// }

	void WorldSpaceGraphicsSystem::Update(float dt)
	{
		//for (auto& i : spatialData)
		//	i.clear();
		//outerSpatial.clear();

		BaseSystem::Update(dt);
	}

	//void WorldSpaceGraphicsSystem::FetchFrustrumCull(Entity camObj)
	//{
	//	std::unordered_set<Entity> ret;
	//	// Add the objects that are outside first lo.
	//	AddGridEntities(ret, gridNum, gridNum);
	//
	//	// -------------------------------------------------------------
	//	// Start
	//	// -------------------------------------------------------------
	//	auto& transform = mRegistry->get<Transform>(camObj);
	//	auto& camera = mRegistry->get<Camera>(camObj);
	//
	//	auto& m = transform.transform;
	//
	//	glm::vec3 p0 = m[3];
	//
	//	glm::mat4 rot = m;
	//	rot[3] = glm::vec4();
	//
	//	glm::vec3 dirFacing = rot * glm::vec4(0.f, 0.f, 1.f, 1.f);
	//	dirFacing = glm::normalize(dirFacing);
	//
	//	float tanT = tanf(glm::radians(camera.pov) * 0.5f);
	//	// Frustrum edges values
	//	float nn = camera.near;
	//	float nh = nn * tanT;
	//	float nw = nh / camera.height * camera.width;
	//	float ff = camera.far;
	//	float fh = ff * tanT;
	//	float fw = fh / camera.height * camera.width;
	//	// -------------------------------------------------------------
	//	// The 4 Edges of the Frustrum from Far -> Near Projected on the floor
	//	// -------------------------------------------------------------
	//	glm::vec3 fVtxs[8];
	//	fVtxs[0] = m * glm::vec4(fw, fh, ff, 1.f);// nw, nh, nn
	//	fVtxs[1] = m * glm::vec4(nw, nh, nn, 1.f);// nw, nh, nn
	//	fVtxs[2] = m * glm::vec4(-fw, -fh, ff, 1.f);// -nw, -nh, nn
	//	fVtxs[3] = m * glm::vec4(-nw, -nh, nn, 1.f);// -nw, -nh, nn
	//	fVtxs[4] = m * glm::vec4(fw, -fh, ff, 1.f);// nw, -nh, nn
	//	fVtxs[5] = m * glm::vec4(nw, -nh, nn, 1.f);// nw, -nh, nn
	//	fVtxs[6] = m * glm::vec4(-fw, fh, ff, 1.f);// -nw, nh, nn
	//	fVtxs[7] = m * glm::vec4(-nw, nh, nn, 1.f);// -nw, nh, nn
	//	for (int i{}; i < 8; ++i) // After rotating, remove y component
	//		fVtxs[i].y = 0.f;
	//
	//	// Direction found, 4 Possible candidates found
	//	float lF{ FLT_MAX }, rF{ FLT_MAX };
	//	glm::vec3 p1{}, p2{};
	//
	//	// Getting the 2 extreme lines
	//	for (int i{}; i < 4; ++i)
	//	{
	//		glm::vec3 temp = fVtxs[i * 2] - fVtxs[i * 2 + 1];
	//		temp.y = 0.f;
	//		temp = glm::normalize(temp) * ff * 2.f;
	//		float dot = temp.x * dirFacing.x + temp.z * dirFacing.z;
	//		// Right
	//		if (temp.x * -dirFacing.z + temp.z * dirFacing.x > 0.f)
	//		{
	//			if (dot < rF)
	//			{
	//				rF = dot;
	//				p2 = temp;
	//			}
	//		}
	//		else
	//		{
	//			if (dot < lF)
	//			{
	//				lF = dot;
	//				p1 = temp;
	//			}
	//		}
	//	}
	//	p0 /= gridSize;
	//	p1 /= gridSize;
	//	p2 /= gridSize;
	//	// Offset so that the correct grid is selected
	//	p0 += glm::vec3{ 0.5f,0.f,0.5f };
	//	p1 = p0 + p1;
	//	p2 = p0 + p2;
	//
	//	int tempX = static_cast<int>(p0.x), tempZ = static_cast<int>(p0.z);
	//
	//	glm::ivec2 basicOffset{ 1 * static_cast<int>(dirFacing.x < 0) * -1, 1 * static_cast<int>(dirFacing.z < 0) * -1 };
	//
	//	AddGridEntities(ret, tempX, tempZ);
	//	AddGridEntities(ret, tempX + basicOffset.x, tempZ);
	//	AddGridEntities(ret, tempX, tempZ + basicOffset.y);
	//	AddGridEntities(ret, tempX + basicOffset.x, tempZ + basicOffset.y);
	//
	//	// -------------------------------------------------------------
	//	// Edge Walking
	//	// -------------------------------------------------------------
	//	// Convert it to a vec2 for easier use
	//	p0.y = p0.z;
	//	p1.y = p1.z;
	//	p2.y = p2.z;
	//
	//	// l0 ~ l2 stands for the direction of the 3 lines
	//	glm::vec3 l2{ p1.x - p0.x, p1.y - p0.y, 0 };// p0 -> p1
	//	glm::vec3 l1{ p0.x - p2.x, p0.y - p2.y, 0 };// p2 -> p0
	//	glm::vec3 l0{ p2.x - p1.x, p2.y - p1.y, 0 };// p1 -> p2
	//	if (l2.y * l1.x + l2.x * -l1.y < 0) // if Total Area is -ve // L1 X L2
	//	{
	//		std::swap(p1, p2);
	//		l2 = glm::vec3{ p1.x - p0.x, p1.y - p0.y, 0 };
	//		l1 = glm::vec3{ p0.x - p2.x, p0.y - p2.y, 0 };
	//		l0 = -l0;
	//		if (l2.y * l1.x + l2.x * -l1.y < 0)
	//			return;
	//	}
	//
	//	// Convert the 3 lines to edge equation
	//	l0 = glm::vec3{ -l0.y, l0.x, l0.y * p1.x - l0.x * p1.y };
	//	l1 = glm::vec3{ -l1.y, l1.x, l1.y * p2.x - l1.x * p2.y };
	//	l2 = glm::vec3{ -l2.y, l2.x, l2.y * p0.x - l2.x * p0.y };
	//
	//	GLint xMin{ static_cast<GLint>(fminf(fminf(p0.x, p1.x), p2.x)) },
	//		yMin{ static_cast<GLint>(fminf(fminf(p0.y, p1.y), p2.y)) },
	//		xMax{ static_cast<GLint>(fmaxf(fmaxf(p0.x, p1.x), p2.x)) },
	//		yMax{ static_cast<GLint>(fmaxf(fmaxf(p0.y, p1.y), p2.y)) };
	//
	//	// Just writing down the starting positions, so the code is less cluttered
	//	GLfloat xStart{ static_cast<GLfloat>(xMin) },
	//		yStart{ (static_cast<GLfloat>(yMin)) };
	//
	//	// Dot product of the edge equation vs the starting point to check
	//	GLfloat eval0{ l0.x * xStart + l0.y * yStart + l0.z },
	//		eval1{ l1.x * xStart + l1.y * yStart + l1.z },
	//		eval2{ l2.x * xStart + l2.y * yStart + l2.z };
	//
	//	for (GLint y{ yMin }; y <= yMax + 1; ++y)
	//	{
	//		GLfloat heval0{ eval0 },
	//			heval1{ eval1 },
	//			heval2{ eval2 };
	//		// MAYDO: erm.. not fully sure my edge walking is correct esp, since i need be more linent compared to drawing pixels 
	//		bool xPlusOne{ false };
	//		for (GLint x{ xMin }; x <= xMax + 1; ++x)
	//		{
	//			if ((heval0 >= 0 && heval1 >= 0 && heval2 >= 0) || xPlusOne)
	//				AddGridEntities(ret, x, y);
	//
	//			heval0 += l0.x;
	//			heval1 += l1.x;
	//			heval2 += l2.x;
	//
	//			// if eval value passed -ve and any addition is also negative, skip the line
	//			if ((heval0 < 0 && l0.x < 0) || (heval1 < 0 && l1.x < 0) || (heval2 < 0 && l2.x < 0))
	//			{
	//				if (xPlusOne)
	//					break;
	//				xPlusOne = true;
	//			}
	//		}
	//		eval0 += l0.y;
	//		eval1 += l1.y;
	//		eval2 += l2.y;
	//	}
	//
	//	//return ret;
	//}
	//
	//// Can be negative, Basically, you pass in [-5,5] inclusive
	//void WorldSpaceGraphicsSystem::AddGridEntities(std::unordered_set<Entity>& in, const int& x, const int& z)
	//{
	//	int halfGridNum = gridNum / 2 + 1;
	//
	//	if (x > -halfGridNum && x < halfGridNum && z > -halfGridNum && z < halfGridNum)
	//	{
	//		for (auto& i : spatialData[(x + gridNum / 2) * gridNum + (z + gridNum / 2)])
	//		{
	//			if (FactoryInstance.mRegistry.any_of<visibleEntity>(i))
	//				continue;
	//			Core::GetInstance()->mFactory.mRegistry.emplace<visibleEntity>(i);
	//		}
	//			//in.insert(i);
	//	}
	//	else // Add out of bounds objs
	//	{
	//		for (auto& i : outerSpatial)
	//		{
	//			if (FactoryInstance.mRegistry.any_of<visibleEntity>(i))
	//				continue;
	//			Core::GetInstance()->mFactory.mRegistry.emplace<visibleEntity>(i);
	//		}
	//	}
	//}
	//
	//void WorldSpaceGraphicsSystem::ResetVisibleEntities()
	//{
	//	auto view = mRegistry->view<visibleEntity>();
	//	for (auto entity : view)
	//	{
	//		if (mRegistry->any_of<visibleEntity>(entity))
	//			mRegistry->remove<visibleEntity>(entity);
	//	}
	//}

}