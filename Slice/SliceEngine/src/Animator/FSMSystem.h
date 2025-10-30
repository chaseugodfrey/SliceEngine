/*!
\file		FSMSystem.h
\author		Muhammad Rayan (muhammadrayan.b@digipen.edu)
\co-author
\brief
	Functions that deal with the State Machine for Animator

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
*/

#ifndef FSMSYSTEM_H
#define	FSMSYSTEM_H

#include "../ECS/BaseSystem.h"
#include "../ECS/ECSTypes.h"

namespace SliceEngine
{
	struct Animations {};

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
		// change to guid
		std::string stateName;
		Animations* currAnim;
		bool isLoop;
		//rttr::variant stateCon;

		bool hasExitTime;
		float exitTime;
		float entryTime;

		std::vector<Transition> transitions;

		bool operator==(const State& other) const
		{
			return (other.stateName == this->stateName);
		}
	};

	class CStateMachine
	{
	public:

		std::unordered_map<std::string, State> stateMap;
		std::string entryState ;
		State* currState = nullptr;
		std::string nextState;
		std::string prevState;

		bool stateCon = false;
		std::map<std::string, rttr::variant> parameters;


		void SetBool(const std::string& name, bool value) { parameters[name] = value; }
		void SetFloat(const std::string& name, float value) { parameters[name] = value; }
		void SetInt(const std::string& name, int value) { parameters[name] = value; }
	};


	class FSMSystem
	{
	public:

		void InitState();
		void CheckStates();
		void UpdateState();

		bool EvalCon(const rttr::variant& paramValue, ComparisonOp op, const rttr::variant& valueToCompare);

		void OnExit();

	private:
		CStateMachine EFSM;
	};
}

#endif