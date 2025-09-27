/******************************************************************************/
/*!
\group  	CtrlAlt
\file		EditorProperties.h
\author 	Chase Rodrigues
\par    	rodrigues.i@digipen.edu
\date   	14th February 2025
\brief

Editor class that wraps ImGui functions and editor commands to create displayable properties to allow for
editing, undoing and redoing.

 */
 /******************************************************************************/

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