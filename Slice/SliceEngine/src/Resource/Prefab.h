#ifndef PREFAB_H
#define PREFAB_H

namespace SliceEngine
{
	namespace SliceEngineTypes
	{
		class Prefab
		{
		private:
			GUID prefabFile;
			std::string filePath;
		public:
			Prefab() = default;
			Prefab(GUID prefabID, const std::string& file) : prefabFile(prefabID), filePath(file) {}
			~Prefab() = default;
		};
	}
}

#endif