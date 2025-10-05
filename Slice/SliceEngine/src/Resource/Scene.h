/*-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 file:			Scene.h
 author:		Gideon Francis
 email:			g.francis@digipen.edu
 brief:			Loads scene data

Copyright (C) 2024 DigiPen Institute of Technology.
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
			static Scene* Load(std::string const& path)
			{
				return new Scene(path);
			}
			inline std::string GetFilePath() const { return filePath; }
		private:
			Scene() = default;
			Scene(std::string name) : filePath(name) {}
			~Scene() = default;
			std::string filePath;
		};
	}
}


#endif