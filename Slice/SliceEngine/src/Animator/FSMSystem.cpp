#include <pch.h>
#include "FSMSystem.h"

namespace SliceEngine
{
	void FSMSystem::OnExit()
	{
		EFSM.currState = nullptr;
	}

	void FSMSystem::InitState()
	{
		/*if (entryState.empty())
		{
			entryState = stateMap.begin()->second;
		}

		nextState = entryState;*/


		EFSM.currState = &EFSM.stateMap[EFSM.entryState];
		EFSM.stateCon = false;
	}
	void FSMSystem::CheckStates()
	{

		if (!EFSM.currState) return;

		for (const Transition& transition : EFSM.currState->transitions)
		{
			
			if (EFSM.parameters.find(transition.parameterName) != EFSM.parameters.end())
			{
				const rttr::variant& currentParamValue = EFSM.parameters[transition.parameterName];

				if (EvalCon(currentParamValue, transition.operation, transition.condition))
				{
					EFSM.nextState = transition.targetState;
					EFSM.stateCon = true;
					break;
				}
			}
		}
	}
	void FSMSystem::UpdateState()
	{
		if (!EFSM.stateCon)
		{
			return;
		}

		bool safeToChange = false;

		if(EFSM.currState->hasExitTime)
		{
			//if(stateMachine.currState.exitTime >= (current anim time))
			{
				safeToChange = true;
			}
		}
		else
		{
			safeToChange = true;
		}

		if(safeToChange)
		{

			if (EFSM.stateMap.find(EFSM.nextState) != EFSM.stateMap.end())
			{
				EFSM.prevState = EFSM.currState->stateName;
				EFSM.currState = &EFSM.stateMap[EFSM.nextState];
				//stateMachine.animTimer = 0.0f;
			}
			else
			{
				std::cout << "wassup error" << std::endl;
			}

			EFSM.stateCon = false;
			EFSM.nextState.clear();
		}
	}


	bool FSMSystem::EvalCon(const rttr::variant& paramValue, ComparisonOp op, const rttr::variant& valueToCompare)
	{
		switch (op)
		{
		case ComparisonOp::IsTrue:
			return paramValue.to_bool();
		case ComparisonOp::IsFalse:
			return !paramValue.to_bool();
		case ComparisonOp::Equal:
			return paramValue == valueToCompare;
		case ComparisonOp::NotEqual:
			return paramValue != valueToCompare;
		case ComparisonOp::GreaterThan:
			return paramValue.to_float() > valueToCompare.to_float();
		case ComparisonOp::LessThan:
			return paramValue.to_float() < valueToCompare.to_float();
		case ComparisonOp::GreaterOrEqual:
			return paramValue.to_float() >= valueToCompare.to_float();
		case ComparisonOp::LessOrEqual:
			return paramValue.to_float() <= valueToCompare.to_float();
		}
		return false;
	}
}

