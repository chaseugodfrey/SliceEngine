#include <pch.h>
#include "ECSTypes.h"


namespace SliceEngine
{
	void TempTransform::operator=(const Transform& other)
	{
		position = other.position;
		rotation = other.rotation;
		scale = other.scale;
		transform_local = other.transform_local;
		transform = other.transform;
		eulerAnglesHint = other.eulerAnglesHint;
	}
}