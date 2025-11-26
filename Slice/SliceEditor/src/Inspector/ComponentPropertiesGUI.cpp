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

		bool changed = ImGui::DragFloat(id, &val, 0.1f, min, max, format);

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

	bool DragUInt64Input(Registry& reg, const char* id, uint64_t& val, const char* format, uint64_t min, uint64_t max)
	{
		static uint64_t oldVal{};

		bool changed = ImGui::DragScalar(id, ImGuiDataType_U64, &val, 1.0f, &min, &max, format, ImGuiSliderFlags_AlwaysClamp);
		if (ImGui::IsItemActivated())
			oldVal = val;

		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			if (oldVal != val)
			{
				std::unique_ptr<ValueCommand<uint64_t>> command = std::make_unique<ValueCommand<uint64_t>>(val, oldVal, val);
				reg.GetManager<HistoryManager>("History")->AddCommand(std::move(command));
			}
		}

		return changed;
	}
	bool DragUInt32Input(Registry& reg, const char* id, uint32_t& val, const char* format, uint32_t min, uint32_t max)
	{
		static uint32_t oldVal{};

		bool changed = ImGui::DragScalar(id, ImGuiDataType_U32, &val, 1.0f, &min, &max, format, ImGuiSliderFlags_AlwaysClamp);

		if (ImGui::IsItemActivated())
			oldVal = val;

		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			if (oldVal != val)
			{
				std::unique_ptr<ValueCommand<uint32_t>> command = std::make_unique<ValueCommand<uint32_t>>(val, oldVal, val);
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

	bool StringInput(Registry& reg, const char* id, std::string& val, float width, std::function<void(std::string)> func)
	{
		static std::string oldVal{};

		if (width == 0.0f)
			width = 150.0f;

		ImGui::SetNextItemWidth(width);

		bool changed = ImGui::InputText(id, &val);

		if (ImGui::IsItemActivated())
		{
			oldVal = val;
		}

		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			if (changed)
			{
				if(func != nullptr)
				{
					func(val);
					std::unique_ptr<FunctionSetsValueCommand<std::string>> command = std::make_unique<FunctionSetsValueCommand<std::string>>(val, oldVal, func);
					reg.GetManager<HistoryManager>("History")->AddCommand(std::move(command));
				}
				else
				{
					std::unique_ptr<ValueCommand<std::string>> command = std::make_unique<ValueCommand<std::string>>(val, oldVal, val);
					reg.GetManager<HistoryManager>("History")->AddCommand(std::move(command));
				}
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

	bool CharBitFlagInputHeader(Registry& reg, const char* property_label, const char* id, char& val)
	{
		return false;
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

	bool DragIntInputHeader(Registry& reg, const char* property_label, const char* id, int& val, const char* format, int min, int max)
	{
		bool changed = false;
		ImGui::Text(property_label);
		ImGui::SameLine(150.f);
		changed = DragIntInput(reg, id, val, format, min, max) || changed;

		return changed;
	}

	bool DragUInt64InputHeader(Registry& reg, const char* property_label, const char* id, uint64_t& val, const char* format, uint64_t min, uint64_t max)
	{
		bool changed = false;
		ImGui::Text(property_label);
		ImGui::SameLine(150.f);
		changed = DragUInt64Input(reg, id, val, format, min, max) || changed;

		return changed;
	}

	bool DragUInt32InputHeader(Registry& reg, const char* property_label, const char* id, uint32_t& val, const char* format, uint32_t min, uint32_t max)
	{
		bool changed = false;
		ImGui::Text(property_label);
		ImGui::SameLine(150.f);
		changed = DragUInt32Input(reg, id, val, format, min, max) || changed;

		return changed;
	}

	bool StringInputHeader(Registry& reg, const char* property_label, const char* id, std::string& val, float width, std::function<void(std::string)> func)
	{
		bool changed = false;
		ImGui::Text(property_label);
		ImGui::SameLine(150.f);
		changed = StringInput(reg, id, val, width, func) || changed;

		return changed;
	}

#pragma region Normal Script Functions

	bool StringInputScriptHeader(Registry& reg, std::function<void(std::string, std::string)> func, const char* property_label, const char* id, std::string& val)
	{
		ImGui::Text(property_label);
		ImGui::SameLine(150.f);
		static std::string oldVal{};

		bool changed = ImGui::InputText(id, &val);

		if (ImGui::IsItemActivated())
			oldVal = val;

		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			if (oldVal != val)
			{
				std::unique_ptr<ScriptFieldSetterCommand<std::string>> command = std::make_unique<ScriptFieldSetterCommand<std::string>>(func, std::string(property_label), oldVal, val);
				reg.GetManager<HistoryManager>("History")->AddCommand(std::move(command));
			}
		}

		return changed;
	}

	bool DragFloatInputScriptHeader(Registry& reg, std::function<void(std::string, float)> func, const char* property_label, const char* id, float& val, const char* format, float min, float max)
	{

		ImGui::Text(property_label);
		ImGui::SameLine(150.f);
		static float oldVal{};

		bool changed = ImGui::DragFloat(id, &val, 0.1f, min, max, format);

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

	bool BoolInputScriptHeader(Registry& reg, std::function<void(std::string, bool)> func, const char* property_label, const char* id, bool& val)
	{
		ImGui::Text(property_label);
		ImGui::SameLine(150.f);
		static bool oldVal{};

		bool changed = ImGui::Checkbox(id, &val);

		if (ImGui::IsItemActivated())
			oldVal = val;

		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			if (oldVal != val)
			{
				std::unique_ptr<ScriptFieldSetterCommand<bool>> command = std::make_unique<ScriptFieldSetterCommand<bool>>(func, std::string(property_label), oldVal, val);
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

		bool changed = ImGui::DragInt(id, &val, 0.1f,min,max,format);

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

	bool DragVec3InputScriptHeader(Registry& reg, std::function<void(std::string, glm::vec3)> func, const char* property_label, const char* id, glm::vec3& val, const char* format, float inc, float min, float max)
	{
		
		static glm::vec3 oldVal{};

		bool changed = false;
		ImGui::Text(property_label);
		ImGui::SameLine(150.0f);
		ImGui::SetNextItemWidth(50.0f);
		changed |= ImGui::DragFloat((id+"_x"s).c_str(), &val.x, inc, min, max, format);
		if (ImGui::IsItemActivated())
			oldVal = val;

		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			if (glm::distance(oldVal, val) > FLT_EPSILON)
			{
				std::unique_ptr<ScriptFieldSetterCommand<glm::vec3>> command = std::make_unique<ScriptFieldSetterCommand<glm::vec3>>(func, std::string(property_label), oldVal, val);
				reg.GetManager<HistoryManager>("History")->AddCommand(std::move(command));
			}
		}

		ImGui::SameLine();
		ImGui::SetNextItemWidth(50.0f);
		changed |= ImGui::DragFloat((id + "_y"s).c_str(), &val.y, inc, min, max, format);
		if (ImGui::IsItemActivated())
			oldVal = val;

		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			if (glm::distance(oldVal, val) > FLT_EPSILON)
			{
				std::unique_ptr<ScriptFieldSetterCommand<glm::vec3>> command = std::make_unique<ScriptFieldSetterCommand<glm::vec3>>(func, std::string(property_label), oldVal, val);
				reg.GetManager<HistoryManager>("History")->AddCommand(std::move(command));
			}
		}

		ImGui::SameLine();
		ImGui::SetNextItemWidth(50.0f);
		changed |= ImGui::DragFloat((id + "_z"s).c_str(), &val.z, inc, min, max, format);

		if (ImGui::IsItemActivated())
			oldVal = val;

		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			if (glm::distance(oldVal, val) > FLT_EPSILON)
			{
				std::unique_ptr<ScriptFieldSetterCommand<glm::vec3>> command = std::make_unique<ScriptFieldSetterCommand<glm::vec3>>(func, std::string(property_label), oldVal, val);
				reg.GetManager<HistoryManager>("History")->AddCommand(std::move(command));
			}
		}

		return changed;
	}

	//bool GameObjectInputScriptHeader(Registry& reg, std::function<void(std::string, SliceEngine::GameObject)> func, const char* property_label, const char* id, SliceEngine::GameObject& val)
	//{
	//	ImGui::Text(property_label);
	//	ImGui::SameLine(150.f);
	//	static SliceEngine::GameObject oldVal{};

	//	val.GetName();

	//	ImGui::BeginDisabled();
	//	bool changed = ImGui::InputText(id, &val.GetName(),ImGuiInputTextFlags_ReadOnly);
	//	ImGui::EndDisabled();

	//	if (ImGui::IsItemActivated())
	//		oldVal = val;

	//	if (ImGui::IsItemDeactivatedAfterEdit())
	//	{
	//		if (oldVal != val)
	//		{
	//			std::unique_ptr<ScriptFieldSetterCommand<std::string>> command = std::make_unique<ScriptFieldSetterCommand<std::string>>(func, std::string(property_label), oldVal, val);
	//			reg.GetManager<HistoryManager>("History")->AddCommand(std::move(command));
	//		}
	//	}

	//	return changed;
	//}

#pragma endregion

#pragma region Array Script Functions

	bool DragFloatArrayScriptHeader(Registry& reg, std::function<void(std::string, std::vector<float>)> func, const char* property_label, const char* id, std::vector<float>& list, const char* format, float min, float max)
	{
		static std::string elementNo_String =  "Element ";
		static std::vector<float> oldList{};
		int i = 0;
		bool changed = false;
		if (ImGui::TreeNodeEx(property_label, ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_Framed))
		{
			for (auto& entry : list)
			{
				std::string elementPropertyLabel = elementNo_String + std::to_string(i);
				std::string newID = std::string(id) + elementNo_String + std::to_string(i);

				ImGui::Text(elementPropertyLabel.c_str());
				ImGui::SameLine(150.f);
				changed |= ImGui::DragFloat(newID.c_str(), &entry,0.1f,min,max,format);

				if (ImGui::IsItemActivated())
					oldList = list;

				if (ImGui::IsItemDeactivatedAfterEdit())
				{
					if (oldList != list)
					{
						std::unique_ptr<ScriptFieldSetterCommand<std::vector<float>>> command = std::make_unique<ScriptFieldSetterCommand<std::vector<float>>>(func, std::string(property_label), oldList, list);
						reg.GetManager<HistoryManager>("History")->AddCommand(std::move(command));
					}
				}
				i++;
			}
			ImGui::TreePop();
		}
		
		return changed;
	}

	bool DragIntArrayScriptHeader(Registry& reg, std::function<void(std::string, std::vector<int>)> func, const char* property_label, const char* id, std::vector<int>& list, const char* format, int min, int max)
	{
		static std::string elementNo_String =  "Element ";
		static std::vector<int> oldList{};
		int i = 0;
		bool changed = false;
		if (ImGui::TreeNodeEx(property_label, ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_Framed))
		{
			for (auto& entry : list)
			{
				std::string elementPropertyLabel = elementNo_String + std::to_string(i);
				std::string newID = std::string(id) + elementNo_String + std::to_string(i);

				ImGui::Text(elementPropertyLabel.c_str());
				ImGui::SameLine(150.f);
				changed |= ImGui::DragInt(newID.c_str(), &entry,1,min,max,format);

				if (ImGui::IsItemActivated())
					oldList = list;

				if (ImGui::IsItemDeactivatedAfterEdit())
				{
					if (oldList != list)
					{
						std::unique_ptr<ScriptFieldSetterCommand<std::vector<int>>> command = std::make_unique<ScriptFieldSetterCommand<std::vector<int>>>(func, std::string(property_label), oldList, list);
						reg.GetManager<HistoryManager>("History")->AddCommand(std::move(command));
					}
				}
				i++;
			}
			ImGui::TreePop();
		}
		
		return changed;
	}

	bool StringArrayScriptHeader(Registry& reg, std::function<void(std::string, std::vector<std::string>)> func, const char* property_label, const char* id, std::vector<std::string>& list)
	{
		static std::string elementNo_String = "Element ";
		static std::vector<std::string > oldList{};
		int i = 0;
		bool changed = false;
		if (ImGui::TreeNodeEx(property_label, ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_Framed))
		{
			for (auto& entry : list)
			{
				std::string elementPropertyLabel = elementNo_String + std::to_string(i);
				std::string newID = std::string(id) + elementNo_String + std::to_string(i);

				ImGui::Text(elementPropertyLabel.c_str());
				ImGui::SameLine(150.f);
				changed |= ImGui::InputText(newID.c_str(), &entry);

				if (ImGui::IsItemActivated())
					oldList = list;

				if (ImGui::IsItemDeactivatedAfterEdit())
				{
					if (oldList != list)
					{
						std::unique_ptr<ScriptFieldSetterCommand<std::vector<std::string>>> command = std::make_unique<ScriptFieldSetterCommand<std::vector<std::string>>>(func, std::string(property_label), oldList, list);
						reg.GetManager<HistoryManager>("History")->AddCommand(std::move(command));
					}
				}
				i++;
			}
			ImGui::TreePop();
		}

		return changed;
	}
	
	bool DragVec3ArrayScriptHeader(Registry& reg, std::function<void(std::string, std::vector<glm::vec3>)> func, const char* property_label, const char* id, std::vector<glm::vec3>& list, const char* format, float inc, float min, float max)
	{
		static std::string elementNo_String = "Element ";
		static std::vector<glm::vec3> oldList{};
		int i = 0;
		bool changed = false;
		if (ImGui::TreeNodeEx(property_label, ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_Framed))
		{
			for (auto& entry : list)
			{
				std::string elementPropertyLabel = elementNo_String + std::to_string(i);
				std::string newID = std::string(id) + elementNo_String + std::to_string(i);

				ImGui::Text(elementPropertyLabel.c_str());
				ImGui::SameLine(150.f);
				ImGui::SetNextItemWidth(50.f);
				changed |= ImGui::DragFloat((newID+"_x"s).c_str(), &entry.x, inc, min, max, format);

				if (ImGui::IsItemActivated())
					oldList = list;

				if (ImGui::IsItemDeactivatedAfterEdit())
				{
					if (oldList != list)
					{
						std::unique_ptr<ScriptFieldSetterCommand<std::vector<glm::vec3>>> command = std::make_unique<ScriptFieldSetterCommand<std::vector<glm::vec3>>>(func, std::string(property_label), oldList, list);
						reg.GetManager<HistoryManager>("History")->AddCommand(std::move(command));
					}
				}
				ImGui::SameLine();
				ImGui::SetNextItemWidth(50.f);
				changed |= ImGui::DragFloat((newID + "_y"s).c_str(), &entry.y, inc, min, max, format);

				if (ImGui::IsItemActivated())
					oldList = list;

				if (ImGui::IsItemDeactivatedAfterEdit())
				{
					if (oldList != list)
					{
						std::unique_ptr<ScriptFieldSetterCommand<std::vector<glm::vec3>>> command = std::make_unique<ScriptFieldSetterCommand<std::vector<glm::vec3>>>(func, std::string(property_label), oldList, list);
						reg.GetManager<HistoryManager>("History")->AddCommand(std::move(command));
					}
				}
				ImGui::SameLine();
				ImGui::SetNextItemWidth(50.f);
				changed |= ImGui::DragFloat((newID + "_z"s).c_str(), &entry.z, inc, min, max, format);

				if (ImGui::IsItemActivated())
					oldList = list;

				if (ImGui::IsItemDeactivatedAfterEdit())
				{
					if (oldList != list)
					{
						std::unique_ptr<ScriptFieldSetterCommand<std::vector<glm::vec3>>> command = std::make_unique<ScriptFieldSetterCommand<std::vector<glm::vec3>>>(func, std::string(property_label), oldList, list);
						reg.GetManager<HistoryManager>("History")->AddCommand(std::move(command));
					}
				}
				i++;
			}
			ImGui::TreePop();
		}

		return changed;
	}

#pragma endregion

#pragma region List Script Functions
	bool StringListScriptHeader(Registry& reg, std::function<void(const char*, std::string, std::vector<std::string>, std::string, int)> editFunc, const char* property_label, const char* id, std::vector< std::string>& list)
	{
		static std::string elementNo_String = "Element ";
		static std::vector<std::string > oldList{};
		int idx = 0;
		bool changed = false;
		if (ImGui::TreeNodeEx(property_label, ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_AllowOverlap))
		{
			for (auto& entry : list)
			{
				std::string elementPropertyLabel = elementNo_String + std::to_string(idx);
				std::string newID = std::string(id) + elementNo_String + std::to_string(idx);
				std::string buttonLabel = "-##" + elementPropertyLabel;

				ImGui::Text(elementPropertyLabel.c_str());
				ImGui::SameLine(150.f);
				ImGui::SetNextItemWidth(200.0f);
				changed |= ImGui::InputText(newID.c_str(), &entry);

				if (ImGui::IsItemActivated())
					oldList = list;

				if (ImGui::IsItemDeactivatedAfterEdit())
				{
					editFunc("Edit", std::string(property_label), list, entry, idx);
				}

				ImGui::SameLine();
				if (ImGui::Button(buttonLabel.c_str(), ImVec2(30, 20)))
				{
					editFunc("Remove", std::string(property_label), list, entry, idx);
				}

				idx++;
			}
			ImGui::Dummy(ImVec2(0, 0));
			ImGui::SameLine(150.f);
			if (ImGui::Button("+", ImVec2(30, 20)))
			{
				// snapshot before change
				//oldList = list;

				// perform change
				editFunc("Add", std::string(property_label), list, "", idx);

				// record in history
				/*if (oldList != list)
				{
					auto command = std::make_unique<ScriptFieldSetterCommand<std::vector<std::string>>>(
						addFunc, std::string(property_label), "", "");
					reg.GetManager<HistoryManager>("History")->AddCommand(std::move(command));
				}*/
				changed = true;
			}

			ImGui::TreePop();
		}

		return changed;
	}
	
	bool FloatListScriptHeader(Registry& reg, std::function<void(const char*, std::string, std::vector<float>, float, int)> editFunc, const char* property_label, const char* id, std::vector<float>& list, const char* format, float inc, float min, float max)
	{
		static std::string elementNo_String = "Element ";
		static std::vector<float > oldList{};
		int idx = 0;
		bool changed = false;
		if (ImGui::TreeNodeEx(property_label, ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_AllowOverlap))
		{
			for (auto& entry : list)
			{
				std::string elementPropertyLabel = elementNo_String + std::to_string(idx);
				std::string newID = std::string(id) + elementNo_String + std::to_string(idx);
				std::string buttonLabel = "-##" + elementPropertyLabel;

				ImGui::Text(elementPropertyLabel.c_str());
				ImGui::SameLine(150.f);
				ImGui::SetNextItemWidth(200.0f);
				changed |= ImGui::DragFloat(newID.c_str(), &entry, inc, min, max, format);

				if (ImGui::IsItemActivated())
					oldList = list;

				if (ImGui::IsItemDeactivatedAfterEdit())
				{
					std::unique_ptr<ScriptListSetterCommand<float>> command = std::make_unique<ScriptListSetterCommand<float>>(editFunc,"Edit", std::string(property_label), oldList, list);
					reg.GetManager<HistoryManager>("History")->AddCommand(std::move(command));
				}

				ImGui::SameLine();
				if (ImGui::Button(buttonLabel.c_str(), ImVec2(30, 20)))
				{
					editFunc("Remove", std::string(property_label),list, entry, idx);
					changed = true;
				}

				idx++;
			}
			ImGui::Dummy(ImVec2(0, 0));
			ImGui::SameLine(150.f);
			if (ImGui::Button("+", ImVec2(30, 20)))
			{
				// snapshot before change
				//oldList = list;

				// perform change
				editFunc("Add", std::string(property_label), list, 0.0f, idx);

				// record in history
				/*std::unique_ptr<ScriptListSetterCommand<float>> command = std::make_unique<ScriptListSetterCommand<float>>(editFunc, "Remove", std::string(property_label), oldList, list,idx);
				reg.GetManager<HistoryManager>("History")->AddCommand(std::move(command));*/
				changed = true;
			}

			ImGui::TreePop();
		}

		return changed;
	}

	bool IntListScriptHeader(Registry& reg, std::function<void(const char*, std::string, std::vector<int>, int, int)> editFunc, const char* property_label, const char* id, std::vector<int>& list, const char* format, int inc, int min, int max)
	{
		static std::string elementNo_String = "Element ";
		static std::vector<int> oldList{};
		int idx = 0;
		bool changed = false;
		if (ImGui::TreeNodeEx(property_label, ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_AllowOverlap))
		{
			for (auto& entry : list)
			{
				std::string elementPropertyLabel = elementNo_String + std::to_string(idx);
				std::string newID = std::string(id) + elementNo_String + std::to_string(idx);
				std::string buttonLabel = "-##" + elementPropertyLabel;

				ImGui::Text(elementPropertyLabel.c_str());
				ImGui::SameLine(150.f);
				ImGui::SetNextItemWidth(200.0f);
				changed |= ImGui::DragInt(newID.c_str(), &entry, inc, min, max, format);

				if (ImGui::IsItemActivated())
					oldList = list;

				if (ImGui::IsItemDeactivatedAfterEdit())
				{
					std::unique_ptr<ScriptListSetterCommand<int>> command = std::make_unique<ScriptListSetterCommand<int>>(editFunc, "Edit", std::string(property_label), oldList, list);
					reg.GetManager<HistoryManager>("History")->AddCommand(std::move(command));
				}

				ImGui::SameLine();
				if (ImGui::Button(buttonLabel.c_str(), ImVec2(30, 20)))
				{
					editFunc("Remove", std::string(property_label), list, entry, idx);
					changed = true;
				}

				idx++;
			}
			ImGui::Dummy(ImVec2(0, 0));
			ImGui::SameLine(150.f);
			if (ImGui::Button("+", ImVec2(30, 20)))
			{
				// snapshot before change
				//oldList = list;

				// perform change
				editFunc("Add", std::string(property_label), list, 0, idx);

				// record in history
				/*std::unique_ptr<ScriptListSetterCommand<float>> command = std::make_unique<ScriptListSetterCommand<float>>(editFunc, "Remove", std::string(property_label), oldList, list,idx);
				reg.GetManager<HistoryManager>("History")->AddCommand(std::move(command));*/
				changed = true;
			}

			ImGui::TreePop();
		}

		return changed;
	}

	bool DragVec3ListScriptHeader(Registry& reg, std::function<void(const char*, std::string, std::vector<glm::vec3>, glm::vec3, int)> editFunc, const char* property_label, const char* id, std::vector<glm::vec3>& list, const char* format, float inc, float min, float max)
	{
		static std::string elementNo_String = "Element ";
		static std::vector<glm::vec3 > oldList{};
		int idx = 0;
		bool changed = false;
		if (ImGui::TreeNodeEx(property_label, ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_AllowOverlap))
		{
			for (auto& entry : list)
			{
				std::string elementPropertyLabel = elementNo_String + std::to_string(idx);
				std::string newID = std::string(id) + elementNo_String + std::to_string(idx);
				std::string buttonLabel = "-##" + elementPropertyLabel;

				ImGui::Text(elementPropertyLabel.c_str());
				ImGui::SameLine(150.f);
				ImGui::SetNextItemWidth(50.f);
				changed |= ImGui::DragFloat((newID + "_x"s).c_str(), &entry.x, inc, min, max, format);

				if (ImGui::IsItemActivated())
					oldList = list;

				if (ImGui::IsItemDeactivatedAfterEdit())
				{
					std::unique_ptr<ScriptListSetterCommand<glm::vec3>> command = std::make_unique<ScriptListSetterCommand<glm::vec3>>(editFunc, "Edit", std::string(property_label), oldList, list);
					reg.GetManager<HistoryManager>("History")->AddCommand(std::move(command));
				}

				ImGui::SameLine();
				ImGui::SetNextItemWidth(50.f);
				changed |= ImGui::DragFloat((newID+"_y"s).c_str(), &entry.y, inc, min, max, format);

				if (ImGui::IsItemActivated())
					oldList = list;

				if (ImGui::IsItemDeactivatedAfterEdit())
				{
					std::unique_ptr<ScriptListSetterCommand<glm::vec3>> command = std::make_unique<ScriptListSetterCommand<glm::vec3>>(editFunc, "Edit", std::string(property_label), oldList, list);
					reg.GetManager<HistoryManager>("History")->AddCommand(std::move(command));
				}

				ImGui::SameLine();
				ImGui::SetNextItemWidth(50.f);
				changed |= ImGui::DragFloat((newID + "_z"s).c_str(), &entry.z, inc, min, max, format);

				if (ImGui::IsItemActivated())
					oldList = list;

				if (ImGui::IsItemDeactivatedAfterEdit())
				{
					std::unique_ptr<ScriptListSetterCommand<glm::vec3>> command = std::make_unique<ScriptListSetterCommand<glm::vec3>>(editFunc, "Edit", std::string(property_label), oldList, list);
					reg.GetManager<HistoryManager>("History")->AddCommand(std::move(command));
				}

				ImGui::SameLine();
				if (ImGui::Button(buttonLabel.c_str(), ImVec2(30, 20)))
				{
					editFunc("Remove", std::string(property_label), list, entry, idx);
					changed = true;
				}

				idx++;
			}
			ImGui::Dummy(ImVec2(0, 0));
			ImGui::SameLine(150.f);
			if (ImGui::Button("+", ImVec2(30, 20)))
			{
				// snapshot before change
				//oldList = list;

				// perform change
				editFunc("Add", std::string(property_label), list, glm::vec3(), idx);

				// record in history
				/*std::unique_ptr<ScriptListSetterCommand<float>> command = std::make_unique<ScriptListSetterCommand<float>>(editFunc, "Remove", std::string(property_label), oldList, list,idx);
				reg.GetManager<HistoryManager>("History")->AddCommand(std::move(command));*/
				changed = true;
			}

			ImGui::TreePop();
		}

		return changed;
	}

#pragma endregion

	bool DragColor3InputHeader(Registry& reg, const char* property_label, const char* id, glm::vec3& val)
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

	bool DragColor4InputHeader(Registry& reg, const char* property_label, const char* id, glm::vec4& val)
	{
		bool changed = false;

		ImGui::Text(property_label);
		ImGui::SameLine(150.0f);
		static glm::vec4 startVal{};

		glm::vec4 tempVal = val;

		bool edited = ImGui::ColorEdit4(id, glm::value_ptr(tempVal), ImGuiColorEditFlags_NoInputs);

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
				std::unique_ptr<ValueCommand<glm::vec4>> command = std::make_unique<ValueCommand<glm::vec4>>(val, startVal, val);
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
		changed = ImGui::DragFloat("##rot_x", &euler.x, 0.1f, 0.0f, 0.0f, "X: %.3f");

		if (ImGui::IsItemActivated())
			oldVal = euler;

		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			std::unique_ptr<FunctionSetsValueCommand<glm::vec3>> command = std::make_unique<FunctionSetsValueCommand<glm::vec3>>(oldVal, euler, func);
			reg.GetManager<HistoryManager>("History")->AddCommand(std::move(command));
		}

		ImGui::SameLine();
		ImGui::SetNextItemWidth(50.0f);
		changed = ImGui::DragFloat("##rot_y", &euler.y, 0.1f, 0.0f, 0.0f, "Y: %.3f") || changed;

		if (ImGui::IsItemActivated())
			oldVal = euler;

		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			std::unique_ptr<FunctionSetsValueCommand<glm::vec3>> command = std::make_unique<FunctionSetsValueCommand<glm::vec3>>(oldVal, euler, func);
			reg.GetManager<HistoryManager>("History")->AddCommand(std::move(command));
		}

		ImGui::SameLine();
		ImGui::SetNextItemWidth(50.0f);
		changed = ImGui::DragFloat("##rot_z", &euler.z, 0.1f, 0.0f, 0.0f, "Z: %.3f") || changed;

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

	bool GUIDDragDropInputHeader(Registry& reg, const char* property_label, const char* id, SliceEngine::GUID& guid, const std::string asset_type, std::function<void(SliceEngine::GUID)> setFunc)
	{
		bool changed = false;
		std::string filename{ "(empty)" };

		ImGui::Text(property_label);
		ImGui::SameLine(150.0f);

		auto& assetManager = reg.GetAssetManager();
		auto file = assetManager.GetFilenameFromGUID(guid);

		if (file.has_value())
		{
			filename = file.value();
		}

		ImGui::BeginDisabled();
		ImGui::InputText(id, &filename, ImGuiInputTextFlags_ReadOnly);
		ImGui::EndDisabled();

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(asset_type.c_str()))
			{
				SliceEngine::GUID newGUID(*(SliceEngine::GUID*)payload->Data);

				// Check if guid is same, if is, then dont execute anything
				changed = (guid != newGUID);
				if (changed)
				{
					if (!setFunc)
					{
						std::unique_ptr<ValueCommand<SliceEngine::GUID>> command = std::make_unique<ValueCommand<SliceEngine::GUID>>(guid, guid, newGUID);
						reg.GetManager<HistoryManager>("History")->AddCommand(std::move(command));
						guid = newGUID;
					}

					else
					{
						std::unique_ptr<FunctionSetsValueCommand<SliceEngine::GUID>> command = std::make_unique<FunctionSetsValueCommand<SliceEngine::GUID>>(guid, newGUID, setFunc);
						reg.GetManager<HistoryManager>("History")->AddCommand(std::move(command));

						setFunc(newGUID);
					}
				}
			}

			ImGui::EndDragDropTarget();
		}

		return changed;

	}

	bool DragVec2InputHeader(Registry& reg, const char* property_label, const char* id, glm::vec2& vec)
	{
		bool changed = false;
		ImGui::Text(property_label);
		ImGui::SameLine(150.0f);
		ImGui::SetNextItemWidth(50.0f);
		changed = DragFloatInput(reg, (id + "_x"s).c_str(), vec.x, "X: %.3f") || changed;

		ImGui::SameLine();
		ImGui::SetNextItemWidth(50.0f);
		changed = DragFloatInput(reg, (id + "_y"s).c_str(), vec.y, "Y: %.3f") || changed;

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

void SetBit(unsigned char& mask, unsigned char bit, bool enabled)
{
	if (enabled)
		mask |= bit;
	else
		mask &= ~bit;
}
