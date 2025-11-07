#include "pch.h"
#include "CoroutineManager.h"
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/mono-config.h> 

namespace SliceEngine
{
	namespace CoroutineManager
	{
		void Update(float dt)
		{
            //MonoDomain* domain = mono_domain_get();
            //MonoAssembly* assembly = mono_domain_assembly_open(domain, "../SliceScript/SliceScript.dll");
            //MonoImage* image = mono_assembly_get_image(assembly);

            //MonoClass* klass = mono_class_from_name(image, "SliceEngine", "CoroutineBridge");
            //MonoMethod* method = mono_class_get_method_from_name(klass, "OnUpdate", 1);

            //if (method)
            //{
            //    void* args[1] = { &dt };
            //    mono_runtime_invoke(method, nullptr, args, nullptr);
            //}
		}
	}
}