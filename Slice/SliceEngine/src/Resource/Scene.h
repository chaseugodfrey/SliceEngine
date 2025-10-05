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