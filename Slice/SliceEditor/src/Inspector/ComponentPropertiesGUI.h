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

#ifndef COMPONENT_PROPERPTIES_H
#define COMPONENT_PROPERPTIES_H


namespace SliceEditor
{
	//void IntInput(const char* id, int& val, std::function<void(int)> setFunc = nullptr);
	//void DragIntInput(const char* id, int& val, int min, int max, std::function<void(int)> setFunc = nullptr);
	void DragFloatInput(const char* id, float& val, const char* format, std::function<void(float)> setFunc = nullptr, float min = 0.f, float max = 0.f);
	//void DragDoubleInput(const char* id, double& val, const char* format, std::function<void(double)> setFunc = nullptr);
	//void StringInput(const char* id, std::string& val, std::function<void(std::string)> setFunc = nullptr);
	//void IntInputHeader(const char* property_label, const char* id, int& val, std::function<void(int)> setFunc = nullptr);
	//void DragIntInputHeader(const char* property_label, const char* id, int& val, int min, int max, std::function<void(int)> setFunc = nullptr);
	//void DragFloatInputHeader(const char* property_label, const char* id, float& val, const char* format = "%.3f", std::function<void(float)> setFunc = nullptr, float min = 0.f, float max = 0.f);
	//void DragDoubleInputHeader(const char* property_label, const char* id, double& val, const char* format = "%.3f", std::function<void(double)> setFunc = nullptr);
	//void DragVec2InputHeader(const char* property_label, const char* id, MathLib::vec2& val);
	void DragVec3InputHeader(const char* property_label, const char* id, glm::vec3& val);
	//void DragColorInputHeader(const char* property_label, const char* id, glm::vec4& color);
	//void StringInputHeader(const char* property_label, const char* id, std::string& val);
	//void AssetDragDropInputHeader(const char* property_label, const char* id, std::string& val, std::function<void(std::string)> setFunc, const char* asset_type);

	template <typename T, typename Container>
	void ComboHeader(const char* property_label, std::string& selected, Container& container, std::function<std::string(T&)>& iterateStringFunc)
	{
		if (ImGui::BeginCombo(property_label, selected.c_str()))
		{
			for (T& value : container)
			{
				std::string str = iterateStringFunc(value);
				if (ImGui::Selectable(str.c_str()))
				{
					selected = str;
				}
			}
		}
	}
}

#endif