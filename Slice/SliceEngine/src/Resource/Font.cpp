/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			Font.cpp
 author:		Elton leosantosa
 email:			leosantosa@digipen.edu
 brief:			Loads .fnt Font assets

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#include <pch.h>
#include "Font.h"
#include <fstream>

namespace {
	//some consts to help typing
	constexpr uint16_t version_number = 1;	//i think having a vers number could be useful, maybe
}

namespace SliceEngine
{
	namespace SliceEngineTypes
	{

		bool Font_Data::LoadFontResource(std::string const&) {
			return false;
		}
		void Font_Data::DestroyFontResource() {

		}
	}
}