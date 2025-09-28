/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:        ScriptFunctions.h

 author:   Gideon Francis

 email:       g.francis@digipen.edu

 brief:       Contains the static functions that are meant to be linked to mono c# so that c# scripts can call. Function Names have to match
				the C# equivalent so that it can bind

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

#ifndef SCRIPT_FUNCTIONS_H
#define SCRIPT_FUNCTIONS_H

namespace SliceEngine
{
	class ScriptFunctions
	{
	public:

		/// <summary>
		/// Registers components to mono c#
		/// </summary>
		static void RegisterComponents();

		/// <summary>
		/// Register functions to mono c#
		/// </summary>
		static void RegisterFunctions();
	};
}

#endif