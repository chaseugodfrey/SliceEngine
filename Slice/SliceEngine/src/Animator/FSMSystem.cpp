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
					int pos = anim_pkg.animations[i].name.find("|");
					std::string mapName = anim_pkg.animations[i].name.substr(pos + 1);
					if (EFSM.stateMap.contains(mapName))
					{
						EFSM.stateMap[mapName].curr_anim_idx = i;
						EFSM.stateMap[mapName].animationTime = anim_pkg.animations[i].duration;
						EFSM.stateMap[mapName].fps = anim_pkg.animations[i].fps;
						continue;
					}

					anim_name  = anim_pkg.animations[i].name;
					if (anim_name.empty())
					{
						anim_name = std::to_string(i);
					}

					tmpState.curr_anim_idx = i;
					tmpState.stateName = anim_name;
					tmpState.animationTime = anim_pkg.animations[i].duration;
					tmpState.fps = anim_pkg.animations[i].fps;

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
	void FSMSystem::InitState()
	{
		EFSM.currState = &EFSM.stateMap[EFSM.entryState];
		EFSM.stateCon = false;
	}
	void FSMSystem::CheckStates()
	{
		if (!EFSM.currState) return;

		// only 1 transition
		/*if (EFSM.currState->transitions.size() == 1)
		{
			if (EFSM.parameters.find(EFSM.currState->transitions[0].parameterName) != EFSM.parameters.end())
			{
				const rttr::variant& currentParamValue = EFSM.parameters[EFSM.currState->transitions[0].parameterName];
				EFSM.nextState = EFSM.currState->transitions[0].targetState;
				EFSM.stateCon = true;
				return;
			}
		}*/

		for (const SliceEngineTypes::Transition& transition : EFSM.currState->transitions)
		{
			for(const SliceEngineTypes::Condition& condition : transition.conditions)
			{
				if (EFSM.parameters.find(condition.paramName) != EFSM.parameters.end())
				{
					const rttr::variant& currentParamValue = EFSM.parameters[condition.paramName];

					//bool check = currentParamValue.to_bool();

					if (EvalCon(currentParamValue, condition.op, condition.value))
					{
						EFSM.nextState = transition.targetState;
						EFSM.stateCon = true;
						EFSM.currState->transitionUsed = &transition;
						break;
					}
				}
			}
		}

		for (const SliceEngineTypes::Transition& transition : EFSM.anyState->transitions)
		{
			for (const SliceEngineTypes::Condition& condition : transition.conditions)
			{
				if (EFSM.parameters.find(condition.paramName) != EFSM.parameters.end())
				{
					const rttr::variant& currentParamValue = EFSM.parameters[condition.paramName];

					//bool check = currentParamValue.to_bool();

					if (EvalCon(currentParamValue, condition.op, condition.value))
					{
						EFSM.nextState = transition.targetState;
						EFSM.stateCon = true;
						EFSM.anyState->transitionUsed = &transition;
						break;
					}
				}
			}
		}
	}
	void FSMSystem::UpdateState(float &CTime,float dt)
	{
		// update ctime dt somewhere here
		// not here cos only update when is playin and is bone
		//CTime += dt;

		if (!EFSM.currState) return;

		if (!EFSM.stateCon)
		{
			return;
		}
		EFSM.stateMap[EFSM.prevState].isFinish = false;

		bool safeToChange = false;

		if(EFSM.currState->transitionUsed != nullptr)
		{
			if (EFSM.currState->transitionUsed->hasExitTime)
			{
				// check exit time
				if (EFSM.currState->transitionUsed->exitTime * EFSM.currState->animationTime <= CTime)
				{
					EFSM.currState->isFinish = true;
					safeToChange = true;
				}
			}
			else
			{
				safeToChange = true;
			}
		}

		if(EFSM.anyState->transitionUsed != nullptr)
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
			stateChanged = true;
			EFSM.currState->transitionUsed = nullptr;
			EFSM.anyState->transitionUsed = nullptr;
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
	std::string FSMSystem::GetCurrAnimName()
	{
		if (!EFSM.currState) 
			return std::string{};

		return EFSM.currState->stateName;
	}
	bool FSMSystem::IsCurrAnimFin()
	{
		if (!EFSM.currState) 
			return false;

		return EFSM.currState->isFinish;
	}
	float FSMSystem::GetCurrAnimFPS()
	{
		if (!EFSM.currState)
			return 0.0f;

		return (float)EFSM.currState->fps;
	}

	// change this, its supposed to be either condiiton change or param idk which
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

