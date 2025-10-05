/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			TransformHelper.h
 author:		Won Yu Xuan Rainne
 email:			won.m@digipen.edu
 brief:			Helper functions to convert vec3 to quaternions and vice versa

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#ifndef TRANSFORM_HELPER_H
#define TRANSFORM_HELPER_H

#include "../ECS/ECSTypes.h"

namespace SliceEngine
{
	glm::quat Vec3ToQuat(const glm::vec3& in);
	glm::vec3 QuatToVec3(glm::quat q);
}

#endif