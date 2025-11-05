#include <pch.h>
#include "FSMSystem.h"
#include "Core/Core.h"

namespace SliceEngine
{
	void FSMSystem::OnExit()
	{
		EFSM->currState = nullptr;
	}

	void FSMSystem::InitState()
	{
		EFSM = SliceEngine::Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::StateMachine>(static_cast<GUID>(0));
		if(EFSM.IsValid())
		{
			if (EFSM->stateMap.size() == 0)
			{
				EFSM->currState = nullptr;
			}
			else
			{
				EFSM->currState = &EFSM->stateMap[EFSM->entryState];
			}

			EFSM->stateCon = false;
		}
	}
	void FSMSystem::CheckStates()
	{

		if (!EFSM->currState) return;

		for (const SliceEngineTypes::Transition& transition : EFSM->currState->transitions)
		{
			
			if (EFSM->parameters.find(transition.parameterName) != EFSM->parameters.end())
			{
				const rttr::variant& currentParamValue = EFSM->parameters[transition.parameterName];

				if (EvalCon(currentParamValue, transition.operation, transition.condition))
				{
					EFSM->nextState = transition.targetState;
					EFSM->stateCon = true;
					break;
				}
			}
		}
	}
	void FSMSystem::UpdateState()
	{
		if (!EFSM->stateCon)
		{
			return;
		}

		bool safeToChange = false;

		if(EFSM->currState->hasExitTime)
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

			if (EFSM->stateMap.find(EFSM->nextState) != EFSM->stateMap.end())
			{
				EFSM->prevState = EFSM->currState->stateName;
				EFSM->currState = &EFSM->stateMap[EFSM->nextState];
			}
			else
			{
				std::cout << "wassup error" << std::endl;
			}

			EFSM->stateCon = false;
			EFSM->nextState.clear();
		}
	}


	bool FSMSystem::EvalCon(const rttr::variant& paramValue, SliceEngineTypes::ComparisonOp op, const rttr::variant& valueToCompare)
	{
		switch (op)
		{
		case SliceEngineTypes::ComparisonOp::IsTrue:
			return paramValue.to_bool();
		case SliceEngineTypes::ComparisonOp::IsFalse:
			return !paramValue.to_bool();
		case SliceEngineTypes::ComparisonOp::Equal:
			return paramValue == valueToCompare;
		case SliceEngineTypes::ComparisonOp::NotEqual:
			return paramValue != valueToCompare;
		case SliceEngineTypes::ComparisonOp::GreaterThan:
			return paramValue.to_float() > valueToCompare.to_float();
		case SliceEngineTypes::ComparisonOp::LessThan:
			return paramValue.to_float() < valueToCompare.to_float();
		case SliceEngineTypes::ComparisonOp::GreaterOrEqual:
			return paramValue.to_float() >= valueToCompare.to_float();
		case SliceEngineTypes::ComparisonOp::LessOrEqual:
			return paramValue.to_float() <= valueToCompare.to_float();
		}
		return false;
	}
	
}

