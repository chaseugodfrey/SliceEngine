#include <pch.h>
#include "FSMSystem.h"
#include "Core/Core.h"

namespace SliceEngine
{
	void FSMSystem::OnExit()
	{
		EFSM.currState = nullptr;
	}

	void FSMSystem::InitState(SliceEngineTypes::AnimationPackage anim_pkg)
	{
		//EFSM = SliceEngine::Core::GetInstance()->GetResourceManager()->get<SliceEngineTypes::StateMachine>(static_cast<GUID>(9857886709116471337));
		{
			if (anim_pkg.animations.size() != 0)
			{
				EFSM.stateMap.reserve(anim_pkg.animations.size());

				std::string anim_name;
				SliceEngineTypes::State tmpState;

				for (unsigned int i = 0; i < anim_pkg.animations.size(); i++)
				{
					anim_name  = anim_pkg.animations[i].name;
					if (anim_name.empty())
					{
						anim_name = std::to_string(i);
					}

					tmpState.curr_anim_idx = i;
					tmpState.stateName = anim_name;
					tmpState.animationTime = anim_pkg.animations[i].duration;

					EFSM.stateMap[anim_name] = tmpState;
				}
			}

			if (EFSM.stateMap.size() == 0)
			{
				EFSM.currState = nullptr;
			}
			else
			{
				EFSM.currState = &EFSM.stateMap[EFSM.entryState];
			}

			EFSM.stateCon = false;
		}
	}
	void FSMSystem::CheckStates()
	{
		if (!EFSM.currState) return;

		for (const SliceEngineTypes::Transition& transition : EFSM.currState->transitions)
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
	void FSMSystem::UpdateState(float &CTime)
	{
		if (!EFSM.currState) return;

		if (!EFSM.stateCon)
		{
			return;
		}

		bool safeToChange = false;

		if(EFSM.currState->hasExitTime)
		{
			// check exit time
			if(EFSM.currState->exitTime * EFSM.currState->animationTime <= current_time)
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
			}
			else
			{
				std::cout << "wassup error" << std::endl;
			}

			EFSM.stateCon = false;
			EFSM.nextState.clear();

			CTime = 0.0f;
		}
	}

	void FSMSystem::UpdateCurrentTime(float cTime)
	{
		current_time = cTime;
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
	
	void FSMSystem::SetFloat(const std::string& name, float value)
	{
		if (!EFSM.currState) return;

		EFSM.parameters[name] = value;
	}
	void FSMSystem::SetInt(const std::string& name, int value)
	{
		if (!EFSM.currState) return;

		EFSM.parameters[name] = value;
	}
	void FSMSystem::SetLoop(bool loop)
	{
		if (!EFSM.currState) return;

		EFSM.currState->isLoop = loop;
	}
	void FSMSystem::SetBool(const std::string& name, bool value)
	{
		if (!EFSM.currState) return;

		EFSM.parameters[name] = value;
		for (auto& [key, var] : EFSM.parameters)
		{
			if(value)
			{
				if (var.is_type<bool>())
				{
					if (std::strcmp(key.c_str(), name.c_str()) != 0)
					{
						var = false;
					}
				}
			}
		}
	}
}

