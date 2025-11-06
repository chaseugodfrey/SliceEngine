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

		struct Transition
		{
			std::string targetState;
			rttr::variant condition;
			ComparisonOp operation;
			std::string parameterName;

			bool operator==(const Transition& other) const
			{
				return targetState == other.targetState && condition == other.condition;
			}
		};

		struct State
		{
			std::string stateName;
			unsigned int curr_anim_idx{};

			bool hasExitTime;
			float exitTime;
			float entryTime;

			std::vector<Transition> transitions;

			bool operator==(const State& other) const
			{
				return (other.stateName == this->stateName);
			}
		};

		class StateMachine
		{
		public:

			std::unordered_map<std::string, State> stateMap;
			std::string entryState;
			State* currState = nullptr;
			std::string nextState;
			std::string prevState;

			bool stateCon = false;
			std::map<std::string, rttr::variant> parameters;

			void SetBool(const std::string& name, bool value) { parameters[name] = value; }
			void SetFloat(const std::string& name, float value) { parameters[name] = value; }
			void SetInt(const std::string& name, int value) { parameters[name] = value; }

			static StateMachine LoadStateMachineResource(std::string const&);
		};


	}
}

#endif
