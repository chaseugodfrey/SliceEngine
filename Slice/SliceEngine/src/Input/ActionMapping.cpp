/* File Documentation -----------------------------------------------------------------------------
file:           ActionMapping.cpp

\author			Micah Lim (100%)

email:          micahshengyao.lim@digipen.edu

brief:

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
--------------------------------------------------------------------------------------------------*/
#include <pch.h>
#include "ActionMapping.h"
#include <cassert> // for assert
#include <iostream> // for read and write to text file
#include <fstream> // for file stream
#include <string>
#include "../nlohmann/include/json.hpp"
using nlohmann::json;

namespace SliceEngine
{
	// gloval instance of action mapping system
	// use a pinter to be set to singleton instance
	static ActionMappingSystem gActionMappingSystemInstance{ nullptr };

	// global accessor for map of action maps
	ActionMappingSystem& GetActionMappingSystem()
	{
		return gActionMappingSystemInstance;
	}

#pragma region Enable/Disable Action Maps & Finding Action
	void ActionMappingSystem::enableMap(const std::string& mapName, bool enable)
	{
		auto* map = findMap(maps, mapName);
		// if map exists, enable/disable it
		if (map)
		{
			map->enabled = enable;
		}
	}

	// finding maps and actions
	ActionMap* ActionMappingSystem::findMap(std::unordered_map<std::string, ActionMap>& maps, const std::string& mapName)
	{
		auto it = maps.find(mapName);
		if (it != maps.end())
		{
			return &(it->second);
		}
		return nullptr;
	}

	size_t ActionMappingSystem::findAction(const ActionMap& actionMap, const std::string& actionName)
	{
		// loop through action definitions to find action by name
		for (size_t i{}; i < actionMap.definitions.size(); ++i)
		{
			if (actionMap.definitions[i].name == actionName)
			{
				return i;
			}
		}
		return static_cast<size_t>(-1); // gpt says returning false is bad practice, so return max size_t value instead
	}

	size_t ActionMappingSystem::findBinding(const ActionDefinition& actionDef, int keyCode)
	{
		// loop through bindings to find keycode
		for (size_t i{}; i < actionDef.bindings.size(); ++i)
		{
			if (actionDef.bindings[i].keyCode == keyCode)
			{
				return i;
			}
		}
		return static_cast<size_t>(-1); // not found
	}

	// action map creation
	ActionMap& ActionMappingSystem::CreateMap(const std::string& mapName)
	{
		// if map already exists, return it
		if (auto* existingMap = findMap(maps, mapName))
		{
			return *existingMap;
		}


		// else create new map and return it
		auto& newMap = maps[mapName];
		newMap.name = mapName;
		// call the savetofile function here later to persist the new map
		return newMap;

	}

#pragma endregion

#pragma region Action Adding and Binding
	// adding actions
	size_t ActionMappingSystem::AddButton(const std::string& mapName, const std::string& actionName)
	{
		auto& map = maps[mapName];
		map.name = mapName;
		map.definitions.push_back({ actionName, ActionType::Button, {} }); // give name, type, empty bindings for now
		map.states.push_back({ ActionPhase::Disabled, false, 0.0f, 0.0f }); // default state
		return map.definitions.size() - 1; // return index of newly added action
	}

	size_t ActionMappingSystem::AddValue1D(const std::string& mapName, const std::string& actionName)
	{
		auto& map = maps[mapName];
		map.name = mapName;
		map.definitions.push_back({ actionName, ActionType::Value1D, {} }); // give name, type, empty bindings for now
		map.states.push_back({ ActionPhase::Disabled, false, 0.0f, 0.0f }); // default state
		return map.definitions.size() - 1; // return index of newly added action
	}

	size_t ActionMappingSystem::AddValue2D(const std::string& mapName, const std::string& actionName)
	{
		auto& map = maps[mapName];
		map.name = mapName;
		map.definitions.push_back({ actionName, ActionType::Value2D, {} }); // give name, type, empty bindings for now
		map.states.push_back({ ActionPhase::Disabled, false, 0.0f, 0.0f }); // default state
		return map.definitions.size() - 1; // return index of newly added action
	}

	// binding actions
	void ActionMappingSystem::BindButton(const std::string& mapName, const std::string& actionName, int keyCode)
	{
		auto& map = maps[mapName];
		// find action index through its name then bind the keycode to it
		size_t actionIndex = findAction(map, actionName);
		// if action found, bind keycode
		if (actionIndex != static_cast<size_t>(-1))
		{
			map.definitions[actionIndex].bindings.push_back({ keyCode, 1.0f, 0.0f, 0.0f }); // for button, scaleX=1.0f, x/y=0.0f
			//std::cout << "Bound keycode " << keyCode << " to action '" << actionName << "' in map '" << mapName << "'\n";
		}
	}

	void ActionMappingSystem::Bind1D(const std::string& mapName, const std::string& actionName, int keyCode, float scale)
	{
		auto& map = maps[mapName];
		// find action index through its name then bind the keycode to it
		size_t actionIndex = findAction(map, actionName);
		// if action found, bind keycode
		if (actionIndex != static_cast<size_t>(-1))
		{
			map.definitions[actionIndex].bindings.push_back({ keyCode, scale, 0.0f, 0.0f }); // for value1D, x/y=0.0f
			//std::cout << "Bound keycode " << keyCode << " to action '" << actionName << "' in map '" << mapName << "' with scale " << scale << "\n";
		}
	}

	void ActionMappingSystem::Bind2D(const std::string& mapName, const std::string& actionName, int keyCode, float x, float y)
	{
		auto& map = maps[mapName];
		// find action index through its name then bind the keycode to it
		size_t actionIndex = findAction(map, actionName);
		// if action found, bind keycode
		if (actionIndex != static_cast<size_t>(-1))
		{
			map.definitions[actionIndex].bindings.push_back({ keyCode, 1.0f, x, y }); // for value2D, scaleX=1.0f
			//std::cout << "Bound keycode " << keyCode << " to action '" << actionName << "' in map '" << mapName << "' with value2D(" << x << ", " << y << ")\n";
		}
	}

#pragma endregion

#pragma region Clearing Functions
	/*
		- functions to clear maps, bindings, actions
		so clear binding will read in actionname and mapname, find map, find action, clear all its binding vector and then remove
		it from the json file when we call savetofile.
		- then clear action will read in actionname and mapname, find map, find action, remove both definition and state at that index
		- clear map just removes the entire map from the maps unordered map
	 */
	void ActionMappingSystem::ClearBindings(const std::string& mapName, const std::string& actionName)
	{
		// this function will clear all bindings for a particular action in a particular map
		auto* map = findMap(maps, mapName);
		if (!map)
		{
			return;
		}

		size_t actionIndex = findAction(*map, actionName);
		// check if action exists
		if (actionIndex == static_cast<size_t>(-1))
		{
			return;
		}

		// wipe all binds
		map->definitions[actionIndex].bindings.clear();
		map->states[actionIndex].valueX = map->states[actionIndex].valueY = 0.0f; // set values to zero
		map->states[actionIndex].performedThisFrame = false; // reset performed flag
		map->states[actionIndex].phase = ActionPhase::Waiting; // neutral state
	}

	void ActionMappingSystem::ClearAction(const std::string& mapName, const std::string& actionName)
	{
		auto* map = findMap(maps, mapName);
		if (!map)
		{
			return;
		}

		size_t idx = findAction(*map, actionName);
		if (idx == static_cast<size_t>(-1))
		{
			return;
		}

		// erase matching definition plus its parallel state
		// my previous method wasn't efficient apparently, gpt says to cast to ptrdiff_t to avoid warnings
		map->definitions.erase(map->definitions.begin() + static_cast<std::ptrdiff_t>(idx));
		map->states.erase(map->states.begin() + static_cast<std::ptrdiff_t>(idx));
	}

	void ActionMappingSystem::ClearMap(const std::string& mapName)
	{
		auto it = maps.find(mapName);
		if (it == maps.end())
		{
			return;
		}

		maps.erase(it);
	}

#pragma endregion

#pragma region Input Processing
	// processing, this is the one area where i used gpt for help because idk how to use the queue with it
	void ActionMappingSystem::processInput(const std::string& mapName)
	{
		// designers can query PerformedThisFrame(map, "Jump")

		// check inputsystem existence or enabled or whether its in game mode
		if (!inputSys || !inputSys->IsEnabled() || inputSys->GetMode() != InputMode::Game)
		{
			return; // input system not ready
		}

		// find the action map
		auto* map = findMap(maps, mapName);
		if (!map || !map->enabled)
		{
			return; // map not found or not enabled
		}

		// clear previous frame's action states
		for (auto& actionState : map->states)
		{
			actionState.performedThisFrame = false;
			if (actionState.phase == ActionPhase::Performed || actionState.phase == ActionPhase::Canceled)
			{
				actionState.phase = ActionPhase::Waiting; // reset to waiting if it was performed or started
			}
			actionState.valueX = 0.0f; // reset values
			actionState.valueY = 0.0f;
		}

		// loop through each action in the map, handling button actions for now
		for (size_t i{}; i < map->definitions.size(); ++i)
		{
			auto& actionDef = map->definitions[i];
			auto& actionState = map->states[i];
			//bool actionPerformed = false;

			if (actionDef.type != ActionType::Button)
			{
				continue; // currently only handling button actions
			}

			bool pressed = false;
			bool released = false;

			// check each binding for the action
			for (const auto& binding : actionDef.bindings)
			{
				// accumulate pressed/released states
				pressed = pressed || inputSys->IsKeyPressed(binding.keyCode);
				released = released || inputSys->IsKeyReleased(binding.keyCode);
			}
			if (pressed)
			{
				actionState.phase = ActionPhase::Performed;
				actionState.performedThisFrame = true;
			}
			else if (released)
			{
				actionState.phase = ActionPhase::Canceled;
			}
		}
		//handle value1d actions
		for (size_t i{}; i < map->definitions.size(); ++i)
		{
			// reset accumulated values
			auto& actionDef = map->definitions[i];
			auto& actionState = map->states[i];

			if (actionDef.type != ActionType::Value1D)
			{
				continue; // skip non-value1d actions
			}

			float value = 0.0f;
			for (const auto& bind : actionDef.bindings)
			{
				if (inputSys->IsKeyDown(bind.keyCode))
				{
					value += bind.scaleX; // scalex -> (e.g. A = -1.0f, D = +1.0f)
				}
			}

			actionState.valueX = std::clamp(value, -1.0f, 1.0f); // clamp to -1.0f to 1.0f range

			// turn the phase to performed if valueX is non-zero
			if (actionState.valueX != 0.f)
			{
				actionState.phase = ActionPhase::Performed;
			}
		}

		// handle value2d actions
		for (size_t i{}; i < map->definitions.size(); ++i)
		{
			// reset accumulated values
			auto& actionDef = map->definitions[i];
			auto& actionState = map->states[i];

			if (actionDef.type != ActionType::Value2D)
			{
				continue; // skip non-Value2D actions
			}

			float x = 0.0f;
			float y = 0.0f;

			// check each binding for the action so we can accumulate values
			for (const auto& binding : actionDef.bindings)
			{
				if (inputSys->IsKeyDown(binding.keyCode))
				{
					x += binding.x;
					y += binding.y;
				}
			}

			// clamp to -1.0f to 1.0f range to avoid >1 when multiple keys are pressed
			actionState.valueX = std::clamp(x, -1.0f, 1.0f);
			actionState.valueY = std::clamp(y, -1.0f, 1.0f);

			if (actionState.valueX != 0.f || actionState.valueY != 0.f)
			{
				actionState.phase = ActionPhase::Performed;
			}
		}
	}

	// process all action maps
	void ActionMappingSystem::processAllInput()
	{
		// process using key-value pairs too
		for (auto& keyValue : maps)
		{
			processInput(keyValue.first);
		}
	}

#pragma endregion

#pragma region Queries
	bool ActionMappingSystem::PerformedThisFrame(const std::string& mapName, const std::string& actionName)
	{
		auto* map = findMap(maps, mapName);

		// check if map exists and is enabled
		if (!map || !map->enabled)
		{
			return false; // map not found or not enabled
		}

		// find action index
		size_t actionIndex = findAction(*map, actionName);

		// check if action exists
		if (actionIndex == static_cast<size_t>(-1))
		{
			return false; // action not found
		}

		// return whether action was performed this frame
		return map->states[actionIndex].performedThisFrame;
	}

	std::pair<float, float> ActionMappingSystem::GetValue2D(const std::string& mapName, const std::string& actionName)
	{
		auto* map = findMap(maps, mapName);
		// check if map exists and is enabled
		if (!map || !map->enabled)
		{
			return { 0.0f, 0.0f }; // map not found or not enabled
		}

		// find action index
		size_t actionIndex = findAction(*map, actionName);

		// check if action exists
		if (actionIndex == static_cast<size_t>(-1))
		{
			return { 0.0f, 0.0f }; // action not found
		}
		// return the accumulated value2D
		auto& state = map->states[actionIndex];
		return { state.valueX, state.valueY };
	}

	float ActionMappingSystem::GetValue1D(const std::string& mapName, const std::string& actionName)
	{
		auto* map = findMap(maps, mapName);

		// check if map exists and is enabled
		if (!map || !map->enabled)
		{
			return 0.0f; // map not found or not enabled
		}

		// find action index
		size_t actionIndex = findAction(*map, actionName);

		// check if action exists
		if (actionIndex == static_cast<size_t>(-1))
		{
			return 0.0f; // action not found
		}

		// return the accumulated value1D
		auto& state = map->states[actionIndex];
		return state.valueX;
	}

#pragma endregion

#pragma region JSON Serialization
	// file I/O
	// do save to file first so we know how to load from file later
	bool ActionMappingSystem::SaveToJson(const std::string& path) const
	{
		try
		{
			json root;
			root["version"] = 1;
			root["maps"] = json::array();

			for (const auto& kv : maps)
			{
				const auto& mapName = kv.first;
				const auto& map = kv.second;

				json jMap;
				jMap["name"] = mapName;
				jMap["enabled"] = map.enabled;
				jMap["actions"] = json::array();

				for (size_t i = 0; i < map.definitions.size(); ++i)
				{
					const auto& def = map.definitions[i];

					json jAct;
					jAct["name"] = def.name;
					// type should be button, value1d, value2d
					jAct["type"] = (def.type == ActionType::Button) ? "button" : (def.type == ActionType::Value1D) ? "value1d" : "value2d";

					json jBinds = json::array();
					for (const auto& b : def.bindings)
					{
						json jBind;
						jBind["key"] = b.keyCode;
						// value2d or value1d
						if (def.type == ActionType::Value2D)
						{
							jBind["x"] = b.x;
							jBind["y"] = b.y;
						}
						if (def.type == ActionType::Value1D)
						{
							jBind["scale"] = b.scaleX; // only scaleX used for 1D
						}
						jBinds.push_back(jBind);
					}
					jAct["bindings"] = std::move(jBinds);
					jMap["actions"].push_back(std::move(jAct));
				}

				root["maps"].push_back(std::move(jMap));
			}

			// write to file
			std::ofstream out(path, std::ios::binary | std::ios::trunc);
			if (!out) return false;
			out << root.dump(2); // pretty print, write json to file with indentation of 2 spaces
			return true;
		}
		catch (...) // catch all exceptions
		{
			return false;
		}
	}

	bool ActionMappingSystem::LoadFromJson(const std::string& path)
	{
		try
		{
			std::ifstream in(path, std::ios::binary);
			if (!in) return false;

			json root; in >> root;
			if (!root.is_object() || !root.contains("maps"))
			{
				return false;
			}

			// clear and rebuild
			maps.clear();

			auto jMaps = root["maps"];
			if (!jMaps.is_array())
			{
				return false;
			}

			// loop through maps
			for (const auto& jMap : jMaps)
			{
				// validate map object
				if (!jMap.contains("name") || !jMap.contains("actions")) continue;
				std::string mapName = jMap.value("name", "");
				bool enabled = jMap.value("enabled", true);

				CreateMap(mapName);

				const auto& jActions = jMap["actions"];
				if (!jActions.is_array()) continue;

				for (const auto& jAct : jActions)
				{
					std::string actName = jAct.value("name", "");
					std::string typeStr = jAct.value("type", "button");
					ActionType type = (typeStr == "button") ? ActionType::Button :
						(typeStr == "value1d") ? ActionType::Value1D :
						(typeStr == "value2d") ? ActionType::Value2D :
						ActionType::Button; // default to button if unknown

					if (type == ActionType::Button)
					{
						AddButton(mapName, actName);
					}
					else if (type == ActionType::Value1D)
					{
						AddValue1D(mapName, actName);
					}
					else if (type == ActionType::Value2D)
					{
						AddValue2D(mapName, actName);
					}

					const auto& jBinds = jAct["bindings"];
					if (jBinds.is_array()) {
						for (const auto& jBind : jBinds)
						{
							int   key = jBind.value("key", 0);
							if (type == ActionType::Button)
							{
								BindButton(mapName, actName, key);
							}
							else if (type == ActionType::Value1D)
							{
								float scale = jBind.value("scale", 1.0f);
								Bind1D(mapName, actName, key, scale);
							}
							else if (type == ActionType::Value2D)
							{
								float x = jBind.value("x", 0.0f);
								float y = jBind.value("y", 0.0f);
								Bind2D(mapName, actName, key, x, y);
							}
						}
					}
				}

				enableMap(mapName, enabled);
			}

			return true;
		}
		catch (...)
		{
			return false;
		}
	}
#pragma endregion 

}