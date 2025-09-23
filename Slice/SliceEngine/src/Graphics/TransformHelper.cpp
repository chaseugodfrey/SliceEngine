#include "TransformHelper.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "gtx/euler_angles.hpp"
#define TOANGLEF 57.2957795131f
#include <pch.h>

namespace SliceEngine
{
	glm::quat Vec3ToQuat(const glm::vec3& in)
	{
		return glm::quat(in);
	}

	glm::vec3 QuatToVec3(const glm::quat& in)
	{
		return glm::eulerAngles(in) * TOANGLEF;
	}
}