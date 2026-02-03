/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			Scene.h
 author:		Gideon Francis
 email:			g.francis@digipen.edu
 brief:			Loads scene data

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#ifndef SCENE_H
#define SCENE_H
#include <string>

namespace SliceEngine
{
	namespace SliceEngineTypes
	{
		class Scene
		{
		public:
			//static Scene* Load(std::string const& path)
			//{
			//	return new Scene(path);
			//}
			Scene() = default;
			Scene(std::filesystem::path name) : filePath(name) {}
			~Scene() = default;
			inline std::filesystem::path GetFilePath() const { return filePath; }
			GUID GetNavMeshGUID();
		private:
			std::string sceneName;
			std::filesystem::path filePath;
		};
	}
}


#endif