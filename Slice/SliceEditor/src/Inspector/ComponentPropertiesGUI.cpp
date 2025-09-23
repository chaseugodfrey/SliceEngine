/******************************************************************************/
/*!
\group  	CtrlAlt
\file		EditorProperties.cpp
\author 	Chase Rodrigues
\par    	rodrigues.i@digipen.edu
\date   	14th February 2025
\brief

Editor class that wraps ImGui functions and editor commands to create displayable properties to allow for
editing, undoing and redoing.

 */
 /******************************************************************************/

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

		DragFloatInput(("x##"s + id).c_str(), val.x, "X: %.3f");

		ImGui::SameLine();
		ImGui::SetNextItemWidth(50.0f);

		DragFloatInput(("y##"s + id).c_str(), val.y, "Y: %.3f");

		ImGui::SameLine();
		ImGui::SetNextItemWidth(50.0f);

		DragFloatInput(("z##"s + id).c_str(), val.z, "Z: %.3f");
	}

}


