/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			StateMachine.h
 author:		Muhammad Rayan
 email:			muhammadrayan.b@digipen.edu
 brief:			

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#ifndef STATEMACHINE_H
#define STATEMACHINE_H

namespace SliceEngine
{
	namespace SliceEngineTypes
	{

		enum class ComparisonOp
		{
			Equal,
			NotEqual,
			GreaterThan,
			LessThan,
			GreaterOrEqual,
			LessOrEqual,
			IsTrue,
			IsFalse
		};

		NLOHMANN_JSON_SERIALIZE_ENUM(ComparisonOp, {
			{ComparisonOp::Equal, "Equal"},
			{ComparisonOp::NotEqual, "NotEqual"},
			{ComparisonOp::GreaterThan, "GreaterThan"},
			{ComparisonOp::LessThan, "LessThan"},
			{ComparisonOp::GreaterOrEqual, "GreaterOrEqual"},
			{ComparisonOp::LessOrEqual, "LessOrEqual"},
			{ComparisonOp::IsTrue, "IsTrue"},
			{ComparisonOp::IsFalse, "IsFalse"}
			})

		struct Condition
		{
			std::string paramName;
			rttr::variant value;
			ComparisonOp op;
		};

		struct Transition
		{
			int id;
			std::string sourceState;
			std::string targetState;

			std::vector<Condition> conditions;
			/*rttr::variant condition;
			ComparisonOp operation;
			std::string parameterName;*/

			bool hasExitTime{};
			float exitTime = 1.0f;
			float entryTime = 0.0f;

			bool operator==(const Transition& other) const
			{
				return std::strcmp(sourceState.c_str(), other.sourceState.c_str()) == 0 && std::strcmp(targetState.c_str(), other.targetState.c_str());
			}
		};

		struct State
		{
			std::string stateName;
			unsigned int curr_anim_idx{};
			int fps{};
			float animationTime{};
			float animationSpeed{1};

			bool isLoop { false };
			bool isFinish{ false };

			std::vector<Transition> transitions;
			Transition const* transitionUsed{ nullptr };

			// node editor stuff

			glm::vec2 mNodePos{};

			bool operator==(const State& other) const
			{
				return (other.stateName == this->stateName);
			}
		};

		class StateMachine
		{
		public:

			std::unordered_map<std::string, State> stateMap;
			glm::vec2 entryPosition{};
			glm::vec2 exitPosition{};
			std::string entryState;
			State* currState = nullptr;
			State* anyState = nullptr;
			std::string nextState;
			std::string prevState;

			bool stateCon = false;
			std::unordered_map<std::string, rttr::variant> parameters;
			

			static StateMachine LoadStateMachineResource(std::string const&);
		};


	}
}

#endif
