#ifndef TRANSFORM_HELPER_H
#define TRANSFORM_HELPER_H

#include "../ECS/ECSTypes.h"

namespace SliceEngine
{
	glm::quat Vec3ToQuat(const glm::vec3& in);
	glm::vec3 QuatToVec3(glm::quat q);
}

#endif