/* File Documentation -----------------------------------------------------------------------------
file:           ActionMapping.cpp

\author			Micah Lim (100%)

email:          micahshengyao.lim@digipen.edu

brief:

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
--------------------------------------------------------------------------------------------------*/

#pragma once
#include "pch.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <utility>
#include <algorithm>
#include <cmath>
#include <../src/Input/InputSystem.h>

namespace SliceEngine
{
	// DEFINITIONS
	// ActionDefintion		-> the particular action i wanna perform (jump, shoot, move, etc)
		// contains string name/id, actiontype, 
		// container of actionbinding so that a single action can have multiple bindings
	// devicetype		-> type of controller/device -> keyboard, mouse, gamepad, etc
	// actionbinding	-> the particular key i wanna bind an action to
	// actiontype		-> button, 1d value, 2d value
	// actionphase		-> runtime states similar to queue sys
	// actionstate -> current state of the action (pressed, released, hold, none)
	// actionmap	-> map that contains all actions and states
	// 
	// how this system works: 
	// i create an action, 
	// bind it to a key/button, 
	// then during update i check the key/button state,
	// then update the action state accordingly [hopefully it works this way]

	enum class ActionType
	{
		Button,
		Value1D, // not sure i'll ever use this but just in case
		Value2D
	};

	enum class ActionPhase
	{
		Disabled,
		Waiting,
		Started,
		Performed,
		Canceled
	};

	// binding fo a physical key, for 2dvalues, use x/y vvalues
	struct ActionBinding
	{
		int keyCode{}; // glfw keycode or mouse button code
		float scaleX = 0.0f; // for value1D/Horizontal
		float scaleY = 0.0f; // for value1D/Vertical
		float x = 0.0f; // for value2D
		float y = 0.0f; // for value2D
	};

	struct ActionDefinition
	{
		std::string name;
		ActionType type = ActionType::Button; // default to button for now
		std::vector<ActionBinding> bindings; // for multiple bindings per action
	};

	struct ActionState
	{
		ActionPhase phase = ActionPhase::Disabled; // default to disabled
		bool performedThisFrame = false; // to track if action was performed this frame
		float valueX = 0.0f; // for value1D/2D
		float valueY = 0.0f; // for value2D
	};

	struct ActionMap
	{
		std::string name; // name of the action map "map for gameplay", "map for menu", etc
		bool enabled = false; // is this action map enabled
		std::vector<ActionDefinition> definitions; // action definitions
		std::vector<ActionState> states; // states of each action, should be samze size as def
	};

	// this class contains functions that will add actions, bind them to keys and work with queue system to update action states
	class ActionMappingSystem
	{
	public:
		// pointer to input system to query key/button states
		// this explicit default constructor is to ensure input system pointer is provided
		explicit ActionMappingSystem(InputSystem* input) : inputSys(input) {}
		void SetInputSystem(InputSystem* input) { inputSys = input; } // setter for input system pointer

		// function to read and write action mappings to file so that they persist between sessions
		bool SaveToJson(const std::string& path) const;
		bool LoadFromJson(const std::string& path);

		// functions to add action maps, actions, and bindings
		ActionMap& CreateMap(const std::string& mapName);
		// add action to map
		size_t AddButton(const std::string& mapName, const std::string& actionName);
		size_t AddValue1D(const std::string& mapName, const std::string& actionName);
		size_t AddValue2D(const std::string& mapName, const std::string& actionName);
		// bind keys to action
		void BindButton(const std::string& mapName, const std::string& actionName, int keyCode);
		void Bind1D(const std::string& mapName, const std::string& actionName, int keyCode, float scale);
		void Bind2D(const std::string& mapName, const std::string& actionName, int keyCode, float x, float y);
		// enable/disble action map
		void enableMap(const std::string& mapName, bool enable);

		// UGGGGHHHHH i forgot to create functions to clear bindings and actions and entire maps
		// clear a binding for an action, clear action from map, clear entire map
		void ClearBinding(const std::string& mapName, const std::string& actionName);
		void ClearAction(const std::string& mapName, const std::string& actionName);
		void ClearMap(const std::string& mapName);
		
		// process input events and update action states
		void processInput(const std::string& mapName); 
		// process all maps
		void processAllInput(); 

		// queries
		bool PerformedThisFrame(const std::string& mapName, const std::string& actionName); // for buttons
		std::pair<float, float> GetValue2D(const std::string& mapName, const std::string& actionName); // for value2D

		// function to pull map of actionmaps
		std::unordered_map<std::string, ActionMap>& GetActionMaps() { return maps; }

	private:
		InputSystem* inputSys; // 
		std::unordered_map<std::string, ActionMap> maps; // class contains a container of all the different action maps there are
		static size_t findAction(const ActionMap& actionMap, const std::string& actionName); // finds a particular action in an aciton map
		static ActionMap* findMap(std::unordered_map<std::string, ActionMap>& maps, const std::string& mapName); // finds a specific action map in the container of action maps

	};

	// globbal accessor for singleton class actionmappingsystem instance
	ActionMappingSystem& GetActionMappingSystem();
}