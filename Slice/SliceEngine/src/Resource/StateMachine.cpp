#include "pch.h"
#include "StateMachine.h"
#include "../../SliceEditor/src/AssetManager/AssetTypes.h"
namespace SliceEngine
{
	namespace SliceEngineTypes
	{
        void from_json(const nlohmann::json& j, rttr::variant& var)
        {
            if (j.is_boolean()) {
                var = j.get<bool>();
            }
            else if (j.is_number_integer()) {
                var = j.get<int>();
            }
            else if (j.is_number_float()) {
                var = j.get<double>();
            }
            else if (j.is_string()) {
                var = j.get<std::string>();
            }
            else {
                var.clear(); // Set to invalid/empty
            }
        }
        void from_json(const nlohmann::json& j, Condition& c)
        {
            j.at("paramName").get_to(c.paramName);
            j.at("comparisonOP").get_to(c.op);
            if (j.contains("value")) {
                from_json(j.at("value"), c.value);
            }
            else {
                c.value = rttr::variant();
            }
        }
        void from_json(const nlohmann::json& j, Transition& t)
        {
            j.at("sourceState").get_to(t.sourceState);
            j.at("targetState").get_to(t.targetState);
            j.at("hasExitTime").get_to(t.hasExitTime);
            j.at("exitTime").get_to(t.exitTime);
            j.at("entryTime").get_to(t.entryTime);

            const nlohmann::json& conditionsArray = j.at("conditions");

            for (const auto& conditionJson : conditionsArray)
            {
                // 4. Create a temporary object
                Condition tmpCon;

                // 5. Explicitly call your from_json for Transition
                from_json(conditionJson, tmpCon);

                // 6. Add the deserialized object to your vector
                t.conditions.push_back(tmpCon);
            }
        }

        // --- from_json for State ---
        void from_json(const nlohmann::json& j, State& s)
        {
            j.at("stateName").get_to(s.stateName);
            j.at("currAnimIdx").get_to(s.curr_anim_idx);
            j.at("isLoop").get_to(s.isLoop);
            j.at("mNodePos").get_to(s.mNodePos);
            j.at("fps").get_to(s.fps);

            s.animationSpeed = j.value("speed", 1.0f);

            const nlohmann::json& transitionsArray = j.at("transitions");

            // 2. Make sure the vector is empty before filling it
            s.transitions.clear();

            // 3. Loop over the array
            for (const auto& transitionJson : transitionsArray)
            {
                // 4. Create a temporary object
                Transition tempTransition;

                // 5. Explicitly call your from_json for Transition
                from_json(transitionJson, tempTransition);

                // 6. Add the deserialized object to your vector
                s.transitions.push_back(tempTransition);
            }
        }

		StateMachine StateMachine::LoadStateMachineResource(std::string const& filename)
		{
			StateMachine temp;

			std::ifstream file(filename);
			if (!file.is_open())
			{
				return temp;
			}

			nlohmann::json ctrlJson;
			try
			{
				ctrlJson = nlohmann::json::parse(file);
			}
			catch (nlohmann::json::parse_error& e)
			{
				SLICE_LOG_ERROR("Invalid controller JSON file" + std::string(e.what()));

				return temp;
			}

			temp.entryState = ctrlJson["entryState"];
            auto entry_pos = ctrlJson.find("entryNodePosition");
            auto exit_pos = ctrlJson.find("exitNodePosition");
            temp.entryPosition = entry_pos != ctrlJson.end() ? entry_pos->get<glm::vec2>() : glm::vec2(0.0f);
            temp.exitPosition = exit_pos != ctrlJson.end() ? exit_pos->get<glm::vec2>() : glm::vec2(0.0f);

            if (ctrlJson.contains("parameters"))
            {
                const nlohmann::json& parametersArray = ctrlJson.at("parameters");

                temp.parameters.clear();

                for (auto& [key, value] : parametersArray.items())
                {
                    rttr::variant tempVariant;
                    from_json(value, tempVariant);
                    temp.parameters[key] = tempVariant;
                }
            }

            if (ctrlJson.contains("stateMap"))
            {

                const nlohmann::json& stateArray = ctrlJson.at("stateMap");

                temp.stateMap.clear();

                for (auto& [key, value] : stateArray.items())
                {
                    State tempState;
                    from_json(value, tempState);
                    temp.stateMap[key] = tempState;
                }
            }

            return temp;
		}
	}
}
