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
#include <Selection/SelectionManager.h>
#include "ComponentMultipleSelection.h"

namespace SliceEditor
{
	class Registry;
	#pragma region JPH Helper

	glm::vec3 JPHtoGLM(JPH::Vec3 vec3);

	JPH::Vec3 GLMtoJPH(glm::vec3 vec3);

	#pragma endregion

	bool DragFloatInput(Registry& reg, const char* id, float& val, const char* format, float min = 0.f, float max = 0.f, float speed = 0.1f, bool selectionDifferent = false);

	bool SliderFloatInput(Registry& reg, const char* id, float& val, const char* format, float min, float max);
	

	bool DragIntInput(Registry& reg, const char* id, int& val, const char* format, int min = 0, int max = 0);
	
	bool DragUInt64Input(Registry& reg, const char* id, uint64_t& val, const char* format, uint64_t min = 0, uint64_t max = 0, float speed = 1.0f);
	bool DragUInt32Input(Registry& reg, const char* id, uint32_t& val, const char* format, uint32_t min = 0, uint32_t max = 0);

	bool BoolInput(Registry& reg, const char* id, bool& val);
	
	bool DragVec2InputHeader(Registry& reg, const char* property_label, const char* id, glm::vec2& vec);

	bool DragVec3InputHeader(Registry& reg, const char* property_label, const char* id, glm::vec3& vec, float min = 0.0f, float max = 0.0f, std::array<bool, 3> selectionDifferent = std::array<bool, 3>{false,false,false}, std::array<bool, 3>* changedAxis = nullptr);

	bool DragFreezeOptionsInputHeader(Registry& reg, const char* property_label, const char* id, SliceEngine::RigidBody::FreezeOptions& options);

	bool DragFloatInputHeader(Registry& reg, const char* property_label, const char* id, float& val, const char* format = "%.3f", float min = 0.f, float max = 0.f, float speed = 0.1f);
	
	bool SliderFloatInputHeader(Registry& reg, const char* property_label, const char* id, float& val, const char* format = "%.3f", float min = 0.f, float max = 0.f);

	bool DragIntInputHeader(Registry& reg, const char* property_label, const char* id, int& val, const char* format = "%d", int min = 0, int max = 0);
	
	bool DragUInt64InputHeader(Registry& reg, const char* property_label, const char* id, uint64_t& val, const char* format = "X: %llu", uint64_t min = 0, uint64_t max = 0, float speed = 1.0f);
	bool DragUInt32InputHeader(Registry& reg, const char* property_label, const char* id, uint32_t& val, const char* format = "X: %u", uint32_t min = 0, uint32_t max = 0);
	
	bool BoolInputHeader(Registry& reg, const char* property_label, const char* id, bool& val);

	bool CharBitFlagInputHeader(Registry& reg, const char* property_label, const char* id, char& val);

	bool StringInputScriptHeader(Registry& reg, std::function<void(std::string, std::string)> func, const char* property_label, const char* id, std::string& val, bool selectionDifferent);
	
	bool StringInput(Registry& reg, const char* id, std::string& val, float width, bool enterReturnsTrue = false, std::function<void(std::string)> func = nullptr, bool selectionDifferent = false);

	bool StringInputHeader(Registry& reg, const char* property_label, const char* id, std::string& val, float width = 0.0f, bool enterReturnsTrue = false, std::function<void(std::string)> func = nullptr, bool selectionDifferent = false);

	bool DragFloatInputScriptHeader(Registry& reg, std::function<void(std::string, float)> func, const char* property_label, const char* id, float& val, const char* format = "%.3f", float min = 0.f, float max = 0.f, bool selectionDifferent = false);
	
	bool BoolInputScriptHeader(Registry& reg, std::function<void(std::string, bool)> func, const char* property_label, const char* id, bool& val, bool selectionDifferent);

	bool DragIntInputScriptHeader(Registry& reg, std::function<void(std::string, int)> func, const char* property_label, const char* id, int& val, const char* format = "%d", int min = 0, int max = 0, bool selectionDifferent = false);

	bool DragVec3InputScriptHeader(Registry& reg, std::function<void(std::string, glm::vec3)> func, const char* property_label, const char* id, glm::vec3& val, const char* format, float inc, float min, float max, std::array<bool, 3> selectionDifferent, std::array<bool, 3>* changedAxis);

	bool GameObjectInputScriptHeader(Registry& reg, std::function<void(std::string, SliceEngine::GameObject)> func, const char* property_label, const char* id, SliceEngine::GameObject& val, bool selectionDifferent);

	bool PrefabInputScriptHeader(Registry& reg, std::function<void(std::string, SliceEngine::PrefabVar)> func, const char* property_label, const char* id, SliceEngine::PrefabVar& val);

	bool DragFloatArrayScriptHeader(Registry& reg, std::function<void(std::string, std::vector<float>)> func, const char* property_label, const char* id, std::vector<float>& list, const char* format = "%.3f", float min = 0.f, float max = 0.f);

	bool DragIntArrayScriptHeader(Registry& reg, std::function<void(std::string, std::vector<int>)> func, const char* property_label, const char* id, std::vector<int>& list, const char* format = "%d", int min = 0, int max = 0);

	bool StringArrayScriptHeader(Registry& reg, std::function<void(std::string, std::vector<std::string>)> func, const char* property_label, const char* id, std::vector<std::string>& list);

	bool DragVec3ArrayScriptHeader(Registry& reg, std::function<void(std::string, std::vector<glm::vec3>)> func, const char* property_label, const char* id, std::vector<glm::vec3>& list, const char* format = "%.3f", float inc =0.1f, float min = 0.f, float max = 0.f);

	bool StringListScriptHeader(Registry& reg, std::function<void(const char*, std::string, std::vector<std::string>, std::string, int)> editFunc, const char* property_label, const char* id, std::vector<std::string>& list);

	bool GameObjectListScriptHeader(Registry& reg, std::function<void(const char*, std::string, std::vector<SliceEngine::GameObject>, SliceEngine::GameObject, int)> editFunc, const char* property_label, const char* id, std::vector<SliceEngine::GameObject>& list, std::vector<bool> elementDiffs, std::vector<MultiSelect>& changedVars);

	bool FloatListScriptHeader(Registry& reg, std::function<void(const char*, std::string, std::vector<float>, float, int)> editFunc, const char* property_label, const char* id, std::vector<float>& list, const char* format, float inc, float min, float max, std::vector<bool> elementDiffs,std::vector<MultiSelect>& changedVars);

	bool IntListScriptHeader(Registry& reg, std::function<void(const char*, std::string, std::vector<int>, int, int)> editFunc, const char* property_label, const char* id, std::vector<int>& list, const char* format = "%d", float inc = 1.f, int min = 0, int max = 0);

	bool DragVec3ListScriptHeader(Registry& reg, std::function<void(const char*, std::string, std::vector<glm::vec3>, glm::vec3, int)> editFunc, const char* property_label, const char* id, std::vector<glm::vec3>& list, const char* format ="%.3f", float inc =.1f, float min =0.f, float max =0.f);

	bool DragColor3InputHeader(Registry& reg, const char* property_label, const char* id, glm::vec3& color);

	bool DragColor4InputHeader(Registry& reg, const char* property_label, const char* id, glm::vec4& color);

	bool DragRotationInputHeader(Registry& reg, const char* property_label, const char* id, glm::quat& quat, glm::vec3& euler, std::array<bool, 3> selectionDifferent, std::array<bool, 3>* changedAxis);
	//void IntInput(const char* id, int& val, std::function<void(int)> setFunc = nullptr);
	//void DragIntInput(const char* id, int& val, int min, int max, std::function<void(int)> setFunc = nullptr);
	//void DragDoubleInput(const char* id, double& val, const char* format, std::function<void(double)> setFunc = nullptr);
	//void IntInputHeader(const char* property_label, const char* id, int& val, std::function<void(int)> setFunc = nullptr);
	//void DragIntInputHeader(const char* property_label, const char* id, int& val, int min, int max, std::function<void(int)> setFunc = nullptr);
	//void DragDoubleInputHeader(const char* property_label, const char* id, double& val, const char* format = "%.3f", std::function<void(double)> setFunc = nullptr);
	//void DragVec2InputHeader(const char* property_label, const char* id, MathLib::vec2& val);

	bool GUIDDragDropInputHeader(Registry& reg, const char* property_label, const char* id, SliceEngine::GUID& val, const std::string asset_type, std::function<void(SliceEngine::GUID)> setFunc = nullptr);

	bool EntityInputHeader(Registry& reg, const char* property_label, const char* id, Entity& val);

	bool LayerHeader(Registry& reg, std::string property_label, const char* id, uint32_t& selected, std::vector<std::string>& container, bool searchBar = false, bool selectionDifferent = false);

	// if need to pass in lambda
	// example code:
	//
	// std::function<void(SliceEngine::GUID)> func = [&](SliceEngine::GUID)
	// {
	//		// get resource manager here
	//		// or whatever functions that need to be done here
	//		// set the handle/guid here
	// }
	//

	template<typename Enum>
	bool ComboInput(Registry& reg, const char* id, Enum& selected, std::vector<std::string>& container, bool searchBar = false, bool selectionDifferent = false)
	{
		static char buffer[256];
		static std::string searchPrompt;
		bool changed = false;
		int idx = static_cast<int>(selected);

		if (selectionDifferent)
		{
			idx = container.size() - 1;
		}
		ImGui::SetNextItemWidth(-FLT_MIN);
		if (ImGui::BeginCombo(id, container[idx].c_str()))
		{
			
			if (searchBar)
			{
				std::string newID = std::string(id) + "searchBar";

				if (ImGui::IsWindowAppearing())
				{
					ImGui::SetKeyboardFocusHere();
					buffer[0] = '\0';
					searchPrompt.clear();
				}
				if (ImGui::InputText(newID.c_str(), buffer, IM_ARRAYSIZE(buffer)))
				{
					searchPrompt = buffer;
				}
				ImGui::Separator();
			}
			for (int i = 0; i < container.size(); ++i)
			{
				if (!searchPrompt.empty() && container[i].find(searchPrompt) == std::string::npos)
				{
					continue;
				}

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

	template <typename Enum>
	bool ComboHeader(Registry& reg, std::string property_label, const char* id, Enum& selected, std::vector<std::string>& container, bool searchBar = false, bool selectionDifferent = false)
	{
		bool changed = false;
		std::vector<std::string> containerCopy = container;
		if (!property_label.empty())
		{
			ImGui::Text(property_label.c_str());
			ImGui::SameLine(150.f);
		}
		if (selectionDifferent)
		{
			containerCopy.push_back("---");
		}

		ImGui::SetNextItemWidth(150.0f);

		changed = ComboInput(reg , id, selected, containerCopy, searchBar, selectionDifferent);

		return changed;
	}

	template <typename T>
	bool HandleDragDropInputHeader(Registry& reg, const char* property_label, const char* id, SliceEngine::Handle<T>& handle, const std::string asset_type, std::function<void(SliceEngine::GUID)> setFunc = nullptr, bool multiSelection = false, std::function<SliceEngine::GUID(Entity)> multiSelectFunc = nullptr)
	{
		bool changed = false;
		auto& assetManager = reg.GetAssetManager();
		auto mapPtr = assetManager.GetMapFromAssetType(asset_type);
		auto selectionManager = reg.GetManager<SelectionManager>("Selection");

		if (mapPtr != nullptr)
		{
			std::vector<std::string> mapNames;
			mapNames.reserve(assetManager.GetMapFromAssetType(asset_type)->size()); //Not sure if i need this but just to be sure.

			int currentIndex = -1;
			SliceEngine::GUID currentGUID = handle.getGUID();

			for (const auto& guid : *mapPtr)
			{
				if (assetManager.mGUIDtoFilename.find(guid) == assetManager.mGUIDtoFilename.end())
				{
					SLICE_LOG_ERROR("This is not supposed to happen, some map de-sync!");
					continue;
				}

				if (guid == currentGUID)
				{
					currentIndex = (int)mapNames.size();
				}
				//Gotta manipulate the names somehow, make it the stem?
				std::filesystem::path relativePath = assetManager.mGUIDtoFilename[guid];
				std::string fileNameString = relativePath.filename().string();

				mapNames.push_back(fileNameString);
			}

			//Push a blank at the end for fallback
			//mapNames.push_back(" ");

			/*int selectedIndex = (currentIndex < 0) ? 0 : currentIndex;*/
			int selectedIndex = currentIndex;

			std::string guidString = currentGUID.toString();
			std::string errorText;
			if (assetManager.mGUIDtoFilename.find(currentGUID) == assetManager.mGUIDtoFilename.end())
			{
				//?????? wtf is this
				//SLICE_LOG_ERROR("Cant find GUID of " + guidString);
				errorText = "GUID not found in AssetManager";
				mapNames.push_back(guidString);
				//Should be the last added unknown GUID
				selectedIndex = static_cast<int>(mapNames.size()) - 1;
				ImGui::Text("%s GUID:", property_label);
				ImGui::SameLine(150.f);
			}
			//Make sure its in the respective assetMap too
			else if (auto it = std::find(mapPtr->begin(), mapPtr->end(), currentGUID); it == mapPtr->end())
			{
				SLICE_LOG_WARNING("Found GUID in guidToFilename but its not in the assetMap. Remember to update asset maps!");
				std::filesystem::path relativePath = assetManager.mGUIDtoFilename[currentGUID];
				std::string fileNameString = relativePath.filename().string();
				mapNames.push_back(fileNameString);
				//Should be the last added unknown GUID
				selectedIndex = static_cast<int>(mapNames.size()) - 1;
				ImGui::Text("%s GUID:", property_label);
				ImGui::SameLine(150.f);
			}
			else
			{
				ImGui::Text(property_label);
				ImGui::SameLine(150.0f);
			}

			if (ComboHeader<int>(reg, "", id, selectedIndex, mapNames, true, GUIDMultipleSelection(selectionManager, currentGUID, multiSelection,multiSelectFunc)))
			{
				//const std::string& selectedName = mapNames[selectedIndex];
				SliceEngine::GUID newGUID = (*mapPtr)[selectedIndex];
				changed = (handle.getGUID() != newGUID) || multiSelection;
				if (changed)
				{
					if (!setFunc)
					{
						auto rm = SliceEngine::Core::GetInstance()->GetResourceManager();
						auto newHandle = rm->get<T>(newGUID);

						std::unique_ptr<ValueCommand<SliceEngine::Handle<T>>> command = std::make_unique<ValueCommand<SliceEngine::Handle<T>>>(handle, handle, newHandle);
						reg.GetManager<HistoryManager>("History")->AddCommand(std::move(command));

						handle = newHandle;
					}

					else
					{
						setFunc(newGUID);
					}
				}
			}
		}
		
		//No Drag-Drop for some reason
		else
		{
				std::string filename{ "(empty)" };

				ImGui::Text(property_label);
				ImGui::SameLine(150.0f);

				auto file = assetManager.GetFilenameFromGUID(handle.getGUID());

				if (file.has_value())
				{
					filename = file.value();
				}


				ImGui::BeginDisabled();
				ImGui::InputText(id, &filename, ImGuiInputTextFlags_ReadOnly);
				ImGui::EndDisabled();
		}
		
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(asset_type.c_str()))
			{
				SliceEngine::GUID newGUID(*(SliceEngine::GUID*)payload->Data);

				// Check if guid is same, if is, then dont execute anything
				changed = (handle.getGUID() != newGUID);
				if (changed)
				{
					if (!setFunc)
					{
						auto rm = SliceEngine::Core::GetInstance()->GetResourceManager();
						auto newHandle = rm->get<T>(newGUID);

						std::unique_ptr<ValueCommand<SliceEngine::Handle<T>>> command = std::make_unique<ValueCommand<SliceEngine::Handle<T>>>(handle, handle, newHandle);
						reg.GetManager<HistoryManager>("History")->AddCommand(std::move(command));

						handle = newHandle;
					}

					else
					{
						/*auto rm = SliceEngine::Core::GetInstance()->GetResourceManager();
						auto newHandle = rm->get<T>(newGUID);
						std::unique_ptr<FunctionSetsValueCommand<SliceEngine::Handle<T>>> command = std::make_unique<FunctionSetsValueCommand<SliceEngine::Handle<T>>>(handle, newHandle, setFunc);
						reg.GetManager<HistoryManager>("History")->AddCommand(std::move(command));*/

						setFunc(newGUID);
					}
				}
			}

			ImGui::EndDragDropTarget();
		}

		return changed;
	}
	
	/*template <>
	bool HandleDragDropInputHeader(Registry& reg, const char* property_label, const char* id, SliceEngine::Handle<SliceEngine::SliceEngineTypes::Material>& handle, const std::string asset_type, std::function<void(SliceEngine::GUID)> setFunc);*/

	
}

void SetBit(unsigned char& mask, unsigned char bit, bool enabled);

// using char for now
//bool ComboMultiSelectHeader(Registry& reg, std::string property_label, const char* id, char& value, std::vector<std::string>& container);



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
#endif