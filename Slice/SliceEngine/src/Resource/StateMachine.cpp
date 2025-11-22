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
        void from_json(const nlohmann::json& j, Transition& t)
        {
            j.at("targetState").get_to(t.targetState);
            //j.at("condition").get_to(t.condition);
            j.at("parameterName").get_to(t.parameterName);
            j.at("comparisonOP").get_to(t.operation);
            j.at("hasExitTime").get_to(t.hasExitTime);
            j.at("exitTime").get_to(t.exitTime);
            j.at("entryTime").get_to(t.entryTime);

            const nlohmann::json& conditionJson = j.at("condition");

            // 2. Explicitly call your from_json function for rttr::variant
            from_json(conditionJson, t.condition);
        }

        // --- from_json for State ---
        void from_json(const nlohmann::json& j, State& s)
        {
            j.at("stateName").get_to(s.stateName);
            j.at("currAnimIdx").get_to(s.curr_anim_idx);
            j.at("isLoop").get_to(s.isLoop);
            j.at("mNodePos").get_to(s.mNodePos);

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
