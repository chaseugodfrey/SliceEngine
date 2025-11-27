/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			Scene.cpp
 author:		Gideon Francis
 email:			g.francis@digipen.edu
 brief:			Loads scene data

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#include <pch.h>
#include "Scene.h"
#include "../Core/Core.h"

namespace SliceEngine
{
	namespace SliceEngineTypes
	{

		GUID Scene::GetNavMeshGUID()
		{
			auto path = Core::GetInstance()->GetResourceManager()->GetResourcePath(filePath.filename().stem().string());

			std::filesystem::path sceneMeta{};
			GUID navmeshGUID;

			if (path.has_value())
			{
				sceneMeta = path.value();
				sceneMeta.replace_extension(".meta");

				std::ifstream metaFile(sceneMeta);

				nlohmann::json metaData;

				metaFile >> metaData;

				metaFile.close();


			}

			

			return navmeshGUID;
		}
	}
}