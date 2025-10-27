#include <pch.h>
#include "FSMSystem.h"

namespace SliceEngine
{
	void FSMSystem::EntityOnEnter(entt::registry& reg, entt::entity entity)
	{
		CStateMachine& entitySM = reg.get<CStateMachine>(entity);

		entitySM.currState = &entitySM.stateMap[entitySM.entryState];
		entitySM.stateCon = false;
	}

	void FSMSystem::EntityOnExit(entt::registry& reg, entt::entity entity)
	{
		CStateMachine& entitySM = reg.get<CStateMachine>(entity);
		entitySM.currState = nullptr;
	}

	void FSMSystem::EntityOnUpdate(entt::registry& reg, entt::entity entity, float dt)
	{
		CStateMachine& entitySM = reg.get<CStateMachine>(entity);

		CheckStates(entitySM);

		UpdateState(entitySM);
	}


	void FSMSystem::InitState()
	{
		/*if (entryState.empty())
		{
			entryState = stateMap.begin()->second;
		}

		nextState = entryState;*/
	}
	void FSMSystem::CheckStates(CStateMachine& stateMachine)
	{

		if (!stateMachine.currState) return;

		for (const Transition& transition : stateMachine.currState->transitions)
		{
			
			if (stateMachine.parameters.find(transition.parameterName) != stateMachine.parameters.end())
			{
				const rttr::variant& currentParamValue = stateMachine.parameters[transition.parameterName];

				if (EvalCon(currentParamValue, transition.operation, transition.condition))
				{
					stateMachine.nextState = transition.targetState;
					stateMachine.stateCon = true;
					break;
				}
			}
		}
	}
	void FSMSystem::UpdateState(CStateMachine& stateMachine)
	{
		if (!stateMachine.stateCon)
		{
			return;
		}

		bool safeToChange = false;

		if(stateMachine.currState->hasExitTime)
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

			if (stateMachine.stateMap.find(stateMachine.nextState) != stateMachine.stateMap.end())
			{
				stateMachine.prevState = stateMachine.currState->stateName;
				stateMachine.currState = &stateMachine.stateMap[stateMachine.nextState];
				//stateMachine.animTimer = 0.0f;
			}
			else
			{
				std::cout << "wassup error" << std::endl;
			}

			stateMachine.stateCon = false;
			stateMachine.nextState.clear();
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

