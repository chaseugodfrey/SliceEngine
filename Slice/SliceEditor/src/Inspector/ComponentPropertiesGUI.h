/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        ComponentPropertiesGUI.h

 author:	  Chase Rodrigues

 email:       rodrigues.i@digipen.edu

 brief:		  Declares the ComponentPropertiesGUI class, which wraps ImGui functions to create displayable
		  properties to allow for editing, undoing and redoing.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

#ifndef COMPONENT_PROPERTIES_H
#define COMPONENT_PROPERTIES_H
#include <History/HistoryManager.h>

namespace SliceEditor
{
	class Registry;
	#pragma region JPH Helper

	glm::vec3 JPHtoGLM(JPH::Vec3 vec3);

	JPH::Vec3 GLMtoJPH(glm::vec3 vec3);

	#pragma endregion

	bool DragFloatInput(Registry& reg, const char* id, float& val, const char* format, float min = 0.f, float max = 0.f);

	bool SliderFloatInput(Registry& reg, const char* id, float& val, const char* format, float min, float max);
	
	bool DragIntInput(Registry& reg, const char* id, int& val, const char* format, int min = 0, int max = 0);
	
	bool DragUInt64Input(Registry& reg, const char* id, uint64_t& val, const char* format, uint64_t min = 0, uint64_t max = 0);

	bool BoolInput(Registry& reg, const char* id, bool& valselecte);
	
	bool DragVec3InputHeader(Registry& reg, const char* property_label, const char* id, glm::vec3& vec);

	bool DragFreezeOptionsInputHeader(Registry& reg, const char* property_label, const char* id, SliceEngine::RigidBody::FreezeOptions& options);
	
	bool DragFloatInputHeader(Registry& reg, const char* property_label, const char* id, float& val, const char* format = "%.3f", float min = 0.f, float max = 0.f);
	
	bool SliderFloatInputHeader(Registry& reg, const char* property_label, const char* id, float& val, const char* format = "%.3f", float min = 0.f, float max = 0.f);

	bool DragIntInputHeader(Registry& reg, const char* property_label, const char* id, int& val, const char* format = "%.3f", int min = 0, int max = 0);
	
	bool DragUInt64InputHeader(Registry& reg, const char* property_label, const char* id, uint64_t& val, const char* format = "%.3f", int min = 0, int max = 0);
	
	bool BoolInputHeader(Registry& reg, const char* property_label, const char* id, bool& val);
	
	bool StringInput(Registry& reg, const char* id, std::string& val, float width);

	bool StringInputHeader(Registry& reg, const char* property_label, const char* id, std::string& val, float width = 0.0f);

	bool DragFloatInputScriptHeader(Registry& reg, std::function<void(std::string, float)> func, const char* property_label, const char* id, float& val, const char* format = "%.3f", float min = 0.f, float max = 0.f);
	
	bool DragIntInputScriptHeader(Registry& reg, std::function<void(std::string, int)> func, const char* property_label, const char* id, int& val, const char* format = "%.3f", int min = 0, int max = 0);

	bool DragColor3InputHeader(Registry& reg, const char* property_label, const char* id, glm::vec3& color);

	bool DragColor4InputHeader(Registry& reg, const char* property_label, const char* id, glm::vec4& color);

	bool DragRotationInputHeader(Registry& reg, const char* property_label, const char* id, glm::quat& quat, glm::vec3& euler);
	/*void IntInput(const char* id, int& val, std::function<void(int)> setFunc = nullptr);
	//void DragIntInput(const char* id, int& val, int min, int max, std::function<void(int)> setFunc = nullptr);
	//void DragDoubleInput(const char* id, double& val, const char* format, std::function<void(double)> setFunc = nullptr);
	//void IntInputHeader(const char* property_label, const char* id, int& val, std::function<void(int)> setFunc = nullptr);
	//void DragIntInputHeader(const char* property_label, const char* id, int& val, int min, int max, std::function<void(int)> setFunc = nullptr);
	//void DragDoubleInputHeader(const char* property_label, const char* id, double& val, const char* format = "%.3f", std::function<void(double)> setFunc = nullptr);
	//void DragVec2InputHeader(const char* property_label, const char* id, MathLib::vec2& val);
	//void AssetDragDropInputHeader(const char* property_label, const char* id, std::string& val, std::function<void(std::string)> setFunc, const char* asset_type);
	*/

	/*template <typename T, typename Container>
	void ComboHeader(const char* property_label,const char* id, std::string& selected, Container& container, std::function<std::string(T&)>& iterateStringFunc)
	{
		ImGui::Text(property_label);
		ImGui::SameLine(150.f);
		if (ImGui::BeginCombo(id, selected.c_str()))
		{
			for (T& value : container)
				if (ImGui::Selectable(container[value].c_str()))
				{
					selected = (Enum)value;
				}
		}
	}*/

	template <typename Enum>
	bool ComboHeader(Registry& reg, std::string property_label, const char* id, Enum& selected, std::vector<std::string>& container)
	{
		bool changed = false;
		
		if (!property_label.empty())
		{
			ImGui::Text(property_label.c_str());
			ImGui::SameLine(150.f);
		}

		ImGui::SetNextItemWidth(150.0f);

		int idx = static_cast<int>(selected);
		if (ImGui::BeginCombo(id, container[(int)selected].c_str()))
		{
			for (int i = 0; i < container.size(); ++i)
			{
				if (ImGui::Selectable(container[i].c_str()))
				{
					if (i != idx)
					{
						/*Enum newVal = static_cast<Enum>(i);
						Enum oldVal = static_cast<Enum>(idx);

						std::unique_ptr<ValueCommand<Enum>> command = std::make_unique<ValueCommand<Enum>>(newVal, oldVal, newVal);
						reg.GetManager<HistoryManager>("History")->AddCommand(std::move(command));*/
						
						idx = i;
						selected = static_cast<Enum>(i);
						changed = true;

					}
				}
			}
			ImGui::EndCombo();
		}
		return changed;
	}
}

#endif