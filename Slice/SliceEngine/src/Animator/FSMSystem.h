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

#include "../Resource/StateMachine.h"
#include <rttr/rttr_enable.h>

namespace SliceEngine
{
	class FSMSystem
	{
	public:

		void InitState(SliceEngineTypes::AnimationPackage);
		void InitState(SliceEngineTypes::Anims);
		void InitState();
		void CheckStates();
		void UpdateState(float& CTime,float dt);

		bool EvalCon(const rttr::variant& paramValue, SliceEngineTypes::ComparisonOp op, const rttr::variant& valueToCompare);

		void OnExit();

		void SetBool(const std::string& name, bool value);
		void SetFloat(const std::string& name, float value);
		void SetInt(const std::string& name, int value);
		void SetLoop(bool loop);
		bool SafeToChange(std::string& name);
		std::string GetCurrAnimName();
		bool IsCurrAnimFin();
		float GetCurrAnimFPS();

		SliceEngineTypes::StateMachine EFSM;
		//float current_time{ 0.0f };
		bool stateChanged{ false };
		//StateMachine EFSM;

		RTTR_ENABLE();
	};
}

#endif