/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        ComponentPropertiesGUI.cpp

 author:	  Chase Rodrigues

 email:       rodrigues.i@digipen.edu

 brief:		  Defines the ComponentPropertiesGUI class, which wraps ImGui functions to create displayable
		  properties to allow for editing, undoing and redoing.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

#include <pch.h>
#include "ComponentPropertiesGUI.h"

using namespace std::string_literals;

namespace SliceEditor
{
	void DragFloatInput(const char* id, float& val, const char* format, std::function<void(float)> setFunc, float min, float max)
	{
		static float oldVal{};

		ImGui::DragFloat(id, &val, 0.1f, min, max, format);

		//if (ImGui::IsItemActivated())
		//	oldVal = val;

		//if (ImGui::IsItemDeactivatedAfterEdit())
		//{
		//	if (std::abs(oldVal - val) > FLT_EPSILON)
		//	{
		//		if (setFunc)
		//		{
		//			setFunc(val);
		//			service.AddCommand(std::make_unique<FunctionSetsValueCommand<float>>(setFunc, oldVal, val));
		//		}

		//		else
		//			service.AddCommand(std::make_unique<ValueCommand<float>>(val, oldVal, val));
		//	}
		//}
	}

	void DragVec3InputHeader(const char* property_label, const char* id, glm::vec3& val)
	{
		ImGui::Text(property_label);
		ImGui::SameLine(100.0f);
		ImGui::SetNextItemWidth(50.0f);

		DragFloatInput((id + "_x"s).c_str(), val.x, "X: %.3f");

		ImGui::SameLine();
		ImGui::SetNextItemWidth(50.0f);

		DragFloatInput((id + "_y"s).c_str(), val.y, "Y: %.3f");

		ImGui::SameLine();
		ImGui::SetNextItemWidth(50.0f);

		DragFloatInput((id + "_z"s).c_str(), val.z, "Z: %.3f");
	}

}


