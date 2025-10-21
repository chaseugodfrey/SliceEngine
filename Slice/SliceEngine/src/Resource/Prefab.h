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
		public:
			std::string filePath;
			Prefab() = default;
			Prefab(GUID prefabID, const std::string& file) : prefabFile(prefabID), filePath(file) {}
			~Prefab() = default;
		};
	}
}

#endif