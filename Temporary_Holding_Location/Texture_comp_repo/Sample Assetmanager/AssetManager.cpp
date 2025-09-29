#include "AssetManager.h"
#include <iostream>

#include <TextureDescriptor.h>

#include "windows.h"
namespace Asset {
	//assume that there are no issues creating directories first
	void AssetManager::SetupDirectories() {
		if (!std::filesystem::exists(compiler_dir)) {
			std::cerr << "Need compiler folder and exe" << std::endl;
			std::filesystem::create_directories(compiler_dir);
			std::exit(EXIT_FAILURE);
		}

		std::filesystem::create_directories(resource_dir);
		std::filesystem::create_directories(descriptor_dir);
		std::filesystem::create_directories(asset_dir);
	}

	std::optional<std::filesystem::path> AssetManager::CreateDescriptor(std::filesystem::path const& asset_path) {
		//Check that path exists
		Descriptor_Base* descriptor{};
		if (!std::filesystem::exists(asset_path)) {
			std::cout << "Asset does not exist!" << asset_path << std::endl;
			return std::nullopt;
		}

		//Determione which descriptor to create
		std::string ext = asset_path.extension().string();
		auto type_guid = xresource::type_guid{};// Texture_Asset::resource_type_guid_v;
		std::string desc_folder{};
		if (ext == ".png") {
			type_guid = Texture_Asset::resource_type_guid_v;
			descriptor = new Texture_Asset::Texture_Descriptor{};
			desc_folder = Texture_Asset::folder_name;
		}

		if (descriptor == nullptr) {
			std::cout << "asset dosent match any descriptor" << std::endl;
			return std::nullopt;
		}
		//directory of where descriptor will be saved
		std::filesystem::path descriptor_folder = descriptor_dir / desc_folder;
		//relative path from program to asset
		descriptor->file_path = asset_path;
		
		xresource::full_guid new_guid = { xresource::guid_generator::Instance64(), type_guid};
		std::cout << std::hex << "new guid generated: " << new_guid.m_Instance.m_Value << " " << new_guid.m_Type.m_Value << std::endl;

		//Save the descriptor
		
		//Use the 2nd and 3rd byte to create another subfolder <- probably not needed for now
		 
		std::filesystem::create_directories(descriptor_folder);	//ensure directory is created

		std::ostringstream oss{};
		oss << new_guid.m_Instance.m_Value << ".desc";
		descriptor_folder /= oss.str();
		descriptor->Serialize(descriptor_folder);
		//sanity check
		Texture_Asset::Texture_Descriptor test_desc{};
		test_desc.Deserialize(descriptor_folder);

		return descriptor_folder;
	}

	void AssetManager::CompileAsset(std::filesystem::path const& desc_file) {
		STARTUPINFO si;
		PROCESS_INFORMATION pi;

		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		ZeroMemory(&pi, sizeof(pi));
	
		//std::filesystem::path rel_Path = std::filesystem::relative(desc_file, compiler_dir);

		std::wstring cmd = desc_file.wstring();
		std::filesystem::path compiler = compiler_dir / "TextureCompile.exe";
		// Start the child process. 
		if (!CreateProcess(compiler.wstring().c_str(),   // No module name (use command line)
			cmd.data(),        // Command line
			NULL,           // Process handle not inheritable
			NULL,           // Thread handle not inheritable
			FALSE,          // Set handle inheritance to FALSE
			0,              // No creation flags
			NULL,           // Use parent's environment block
			NULL,           // Use parent's starting directory 
			&si,            // Pointer to STARTUPINFO structure
			&pi)           // Pointer to PROCESS_INFORMATION structure
			)
		{
			printf("CreateProcess failed (%d).\n", GetLastError());
			return;
		}

		// Wait until child process exits.
		WaitForSingleObject(pi.hProcess, INFINITE);

		// Close process and thread handles. 
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
}