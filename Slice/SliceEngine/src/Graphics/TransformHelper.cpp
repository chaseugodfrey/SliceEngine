#include <pch.h>
#include "TransformHelper.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/euler_angles.hpp"
#define M_PI 3.14159265358979323846264338327950288


namespace SliceEngine
{
	glm::quat Vec3ToQuat(const glm::vec3& in)
	{
        glm::mat4x4 rotMatrix = glm::eulerAngleXYZ(glm::radians(in.x), glm::radians(in.y), glm::radians(in.z));
        return glm::quat_cast(rotMatrix);
	}

	glm::vec3 QuatToVec3(glm::quat q)
	{
        q = glm::quat{ q.w, -q.x, -q.y, -q.z };
		glm::vec3 result;

        // roll (x-axis rotation)
        double sinr_cosp = 2.f * (q.w * q.x + q.y * q.z);
        double cosr_cosp = 1.f - 2.f * (q.x * q.x + q.y * q.y);
        result.x = std::atan2(sinr_cosp, cosr_cosp);

        // pitch (y-axis rotation)
        double sinp = std::sqrt(1.f + 2.f * (q.w * q.y - q.x * q.z));
        double cosp = std::sqrt(1.f - 2.f * (q.w * q.y - q.x * q.z));
        result.y = 2.f * std::atan2(sinp, cosp) - M_PI / 2.f;

        // yaw (z-axis rotation)
        double siny_cosp = 2.f * (q.w * q.z + q.x * q.y);
        double cosy_cosp = 1.f - 2.f * (q.y * q.y + q.z * q.z);
        result.z = std::atan2(siny_cosp, cosy_cosp);

        return glm::degrees(-result);
    }
}