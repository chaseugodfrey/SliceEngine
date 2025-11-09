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
#include <Core/Registry.h>
#include "../EditorCommonTypes.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/euler_angles.hpp"
#include <Graphics/TransformHelper.h>

using namespace std::string_literals;

namespace SliceEditor
{
	#pragma region JPH Helper
	glm::vec3 JPHtoGLM(JPH::Vec3 vec3)
	{
		glm::vec3 newVec;

		newVec.x = vec3.GetX();
		newVec.y = vec3.GetY();
		newVec.z = vec3.GetZ();

		return newVec;
	}

	JPH::Vec3 GLMtoJPH(glm::vec3 vec3)
	{
		JPH::Vec3 newVec;

		newVec.SetX(vec3.x);
		newVec.SetY(vec3.y);
		newVec.SetZ(vec3.z);

		return newVec;
	}
	#pragma endregion



	bool DragFloatInput(Registry& reg, const char* id, float& val, const char* format, float min, float max)
	{
		static float oldVal{};

		bool changed = ImGui::DragFloat(id, &val, 0.1f, min, max, format,ImGuiSliderFlags_AlwaysClamp);

		if (ImGui::IsItemActivated())
			oldVal = val;

		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			if (std::abs(oldVal - val) > FLT_EPSILON)
			{	
				std::unique_ptr<ValueCommand<float>> command = std::make_unique<ValueCommand<float>>(val, oldVal, val);
				reg.GetManager<HistoryManager>("History")->AddCommand(std::move(command));
			}
		}

		return changed;
	}

	bool SliderFloatInput(Registry& reg, const char* id, float& val, const char* format, float min, float max)
	{
		static float oldVal{};

		bool changed = ImGui::SliderFloat(id, &val, min, max, format, ImGuiSliderFlags_AlwaysClamp);

		if (ImGui::IsItemActivated())
			oldVal = val;

		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			if (std::abs(oldVal - val) > FLT_EPSILON)
			{
				std::unique_ptr<ValueCommand<float>> command = std::make_unique<ValueCommand<float>>(val, oldVal, val);
				reg.GetManager<HistoryManager>("History")->AddCommand(std::move(command));
			}
		}

		return changed;
	}

	bool DragIntInput(Registry& reg, const char* id, int& val, const char* format, int min, int max)
	{
		static int oldVal{};

		bool changed = ImGui::DragInt(id, &val, 1.0f, min, max, format, ImGuiSliderFlags_AlwaysClamp);

		if (ImGui::IsItemActivated())
			oldVal = val;

		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			if (oldVal != val)
			{
				std::unique_ptr<ValueCommand<int>> command = std::make_unique<ValueCommand<int>>(val, oldVal, val);
				reg.GetManager<HistoryManager>("History")->AddCommand(std::move(command));
			}
		}

		return changed;
	}

	bool BoolInput(Registry& reg, const char* id, bool& val)
	{
		bool changed = ImGui::Checkbox(id, &val);

		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			if (changed)
			{
				std::unique_ptr<ValueCommand<bool>> command = std::make_unique<ValueCommand<bool>>(val, !val, val);
				reg.GetManager<HistoryManager>("History")->AddCommand(std::move(command));
			}
		}

		return changed;
	}

	bool StringInput(Registry& reg, const char* id, std::string& val)
	{
		static std::string oldVal{};
		bool changed = ImGui::InputText(id, &val);

		if (ImGui::IsItemActivated())
		{
			oldVal = val;
		}

		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			if (changed)
			{
				std::unique_ptr<ValueCommand<std::string>> command = std::make_unique<ValueCommand<std::string>>(val, oldVal, val);
				reg.GetManager<HistoryManager>("History")->AddCommand(std::move(command));
			}
		}

		return changed;
	}

	bool BoolInputHeader(Registry& reg, const char* property_label, const char* id, bool& val)
	{
		bool changed = false;
		ImGui::Text(property_label);
		ImGui::SameLine(150.f);
		changed = BoolInput(reg, id, val) || changed;

		return changed;
	}

	bool DragFloatInputHeader(Registry& reg, const char* property_label, const char* id, float& val, const char* format, float min, float max)
	{
		bool changed = false;
		ImGui::Text(property_label);
		ImGui::SameLine(150.f);
		changed = DragFloatInput(reg, id, val, format, min, max) || changed;

		return changed;
	}

	bool SliderFloatInputHeader(Registry& reg, const char* property_label, const char* id, float& val, const char* format, float min, float max)
	{
		bool changed = false;
		ImGui::Text(property_label);
		ImGui::SameLine(150.f);
		changed = SliderFloatInput(reg, id, val, format, min, max) || changed;

		return changed;
	}

	bool DragIntInputHeader(Registry& reg, const char* property_label, const char* id, int& val, const char* format, float min, float max)
	{
		bool changed = false;
		ImGui::Text(property_label);
		ImGui::SameLine(150.f);
		changed = DragIntInput(reg, id, val, format, min, max) || changed;

		return changed;
	}

	bool StringInputHeader(Registry& reg, const char* property_label, const char* id, std::string& val)
	{
		bool changed = false;
		ImGui::Text(property_label);
		ImGui::SameLine(150.f);
		changed = StringInput(reg, id, val) || changed;

		return changed;
	}

	bool DragFloatInputScriptHeader(Registry& reg, std::function<void(std::string, float)> func, const char* property_label, const char* id, float& val, const char* format, float min, float max)
	{

		ImGui::Text(property_label);
		ImGui::SameLine(150.f);
		static float oldVal{};

		bool changed = ImGui::DragFloat(id, &val, 0.1f, min, max, format, ImGuiSliderFlags_AlwaysClamp);

		if (ImGui::IsItemActivated())
			oldVal = val;

		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			if (std::abs(oldVal - val) > FLT_EPSILON)
			{
				std::unique_ptr<ScriptFieldSetterCommand<float>> command = std::make_unique<ScriptFieldSetterCommand<float>>(func, std::string(property_label), oldVal, val);
				reg.GetManager<HistoryManager>("History")->AddCommand(std::move(command));
			}
		}

		return changed;
	}

	bool DragIntInputScriptHeader(Registry& reg, std::function<void(std::string, int)> func, const char* property_label, const char* id, int& val, const char* format, int min, int max)
	{
		ImGui::Text(property_label);
		ImGui::SameLine(150.f);
		static int oldVal{};

		bool changed = ImGui::DragInt(id, &val, 0.1f, min, max, format, ImGuiSliderFlags_AlwaysClamp);

		if (ImGui::IsItemActivated())
			oldVal = val;

		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			if (std::abs(oldVal - val) > FLT_EPSILON)
			{
				std::unique_ptr<ScriptFieldSetterCommand<int>> command = std::make_unique<ScriptFieldSetterCommand<int>>(func, std::string(property_label), oldVal, val);
				reg.GetManager<HistoryManager>("History")->AddCommand(std::move(command));
			}
		}

		return changed;
	}

	bool DragColorInputHeader(Registry& reg, const char* property_label, const char* id, glm::vec3& val)
	{
		bool changed = false;

		ImGui::Text(property_label);
		ImGui::SameLine(150.0f);
		static glm::vec3 startVal{};

		glm::vec3 tempVal = val;

		bool edited = ImGui::ColorEdit3(id, glm::value_ptr(tempVal), ImGuiColorEditFlags_NoInputs);

		if (ImGui::IsItemActivated()) //Check what the value was onClick
		{
			startVal = val;
		}

		if (edited || ImGui::IsItemEdited()) {
			val = tempVal;
			changed = true;
		}

		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			if (glm::any(glm::epsilonNotEqual(val, startVal, 1e-6f)))
			{
				std::unique_ptr<ValueCommand<glm::vec3>> command = std::make_unique<ValueCommand<glm::vec3>>(val, startVal, val);
				reg.GetManager<HistoryManager>("History")->AddCommand(std::move(command));
			}
		}
		return changed;
	}

	bool DragRotationInputHeader(Registry& reg, const char* property_label, const char* id, glm::quat& quat, glm::vec3& euler)
	{
		static glm::vec3 oldVal{};

		std::function<void(glm::vec3)> func
			= [&](glm::vec3 newEuler)
		{
			euler = newEuler;
			quat = SliceEngine::Vec3ToQuat(euler);
			};

		bool changed = false;
		ImGui::Text(property_label);
		ImGui::SameLine(150.0f);
		ImGui::SetNextItemWidth(50.0f);
		changed = ImGui::DragFloat("##rot_x", &euler.x, 0.1f, 0.0f, 0.0f, "X: %.3f", ImGuiSliderFlags_AlwaysClamp);

		if (ImGui::IsItemActivated())
			oldVal = euler;

		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			std::unique_ptr<FunctionSetsValueCommand<glm::vec3>> command = std::make_unique<FunctionSetsValueCommand<glm::vec3>>(oldVal, euler, func);
			reg.GetManager<HistoryManager>("History")->AddCommand(std::move(command));
		}

		ImGui::SameLine();
		ImGui::SetNextItemWidth(50.0f);
		changed = ImGui::DragFloat("##rot_y", &euler.y, 0.1f, 0.0f, 0.0f, "Y: %.3f", ImGuiSliderFlags_AlwaysClamp) || changed;

		if (ImGui::IsItemActivated())
			oldVal = euler;

		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			std::unique_ptr<FunctionSetsValueCommand<glm::vec3>> command = std::make_unique<FunctionSetsValueCommand<glm::vec3>>(oldVal, euler, func);
			reg.GetManager<HistoryManager>("History")->AddCommand(std::move(command));
		}

		ImGui::SameLine();
		ImGui::SetNextItemWidth(50.0f);
		changed = ImGui::DragFloat("##rot_z", &euler.z, 0.1f, 0.0f, 0.0f, "Z: %.3f", ImGuiSliderFlags_AlwaysClamp) || changed;

		if (ImGui::IsItemActivated())
			oldVal = euler;

		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			std::unique_ptr<FunctionSetsValueCommand<glm::vec3>> command = std::make_unique<FunctionSetsValueCommand<glm::vec3>>(oldVal, euler, func);
			reg.GetManager<HistoryManager>("History")->AddCommand(std::move(command));
		}

		if (changed)
			quat = SliceEngine::Vec3ToQuat(euler);

		return changed;
	}

	bool DragVec3InputHeader(Registry& reg, const char* property_label, const char* id, glm::vec3& vec)
	{
		bool changed = false;
		ImGui::Text(property_label);
		ImGui::SameLine(150.0f);
		ImGui::SetNextItemWidth(50.0f);
		changed = DragFloatInput(reg, (id + "_x"s).c_str(), vec.x, "X: %.3f") || changed;

		ImGui::SameLine();
		ImGui::SetNextItemWidth(50.0f);
		changed = DragFloatInput(reg, (id + "_y"s).c_str(), vec.y, "Y: %.3f") || changed;

		ImGui::SameLine();
		ImGui::SetNextItemWidth(50.0f);
		changed = DragFloatInput(reg, (id + "_z"s).c_str(), vec.z, "Z: %.3f") || changed;

		return changed;
	}

	bool DragFreezeOptionsInputHeader(Registry& reg, const char* property_label, const char* id, SliceEngine::RigidBody::FreezeOptions& options)
	{
		bool changed = false;
		ImGui::Text(property_label);
		ImGui::SameLine(150.0f);
		ImGui::Text("X:");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(50.0f);
		changed = BoolInput(reg, (id + "_x"s).c_str(), options.freezeX) || changed;

		ImGui::SameLine();
		ImGui::Text("Y:");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(50.0f);
		changed = BoolInput(reg, (id + "_y"s).c_str(), options.freezeY) || changed;

		ImGui::SameLine();
		ImGui::Text("Z:");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(50.0f);
		changed = BoolInput(reg, (id + "_z"s).c_str(), options.freezeZ) || changed;

		return changed;
	}

}


