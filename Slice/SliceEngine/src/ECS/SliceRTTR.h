/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			SliceRTTR.cpp
 author:		Gideon Francis
 email:			g.francis@digipen.edu
 brief:			RTTR

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#ifndef SLICE_RTTR_H
#define SLICE_RTTR_H

#include "../Graphics/WorldSpaceGraphicsSystem.h"
#include "../Systems/TransformSystem.h"
#include "../Systems/SoundSystem.h"
#include "Resource/ResourceManager.h"
#include "Resource/Resource.h"
#include "Resource/Model.h"
#include "Resource/Material.h"
#include "Animator/FSMSystem.h"
#include <rttr/registration.h>

void InitRTTR();

#endif