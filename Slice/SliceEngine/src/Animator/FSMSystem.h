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

namespace SliceEngine
{
	class FSMSystem
	{
	public:

		void InitState();
		void CheckStates();
		void UpdateState();

		bool EvalCon(const rttr::variant& paramValue, SliceEngineTypes::ComparisonOp op, const rttr::variant& valueToCompare);

		void OnExit();

		Handle<SliceEngineTypes::StateMachine> EFSM;
		//StateMachine EFSM;
	};
}

#endif