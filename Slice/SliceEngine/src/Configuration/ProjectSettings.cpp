/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			ProjectSettings.cpp
 author:		Chase Rodrigues
 email:			roderigues.i@digipen.edu
 brief:			Handles project settings

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#include <pch.h>
#include "ProjectSettings.h"

namespace SliceEngine
{
	void ProjectSettings::CheckDirty()
	{
		if (isDirty)
		{
			ApplySettings();
			SaveSettings();
			isDirty = false;
		}
	}
}