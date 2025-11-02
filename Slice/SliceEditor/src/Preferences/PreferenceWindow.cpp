#include <pch.h>
#include "PreferenceWindow.h"
#include "Core/Registry.h"
#include "Session/SessionManager.h"

namespace SliceEditor
{
	ImVec4 operator* (const ImVec4 v, const float f)
	{
		return ImVec4(std::clamp(v.x * f, 0.f, 255.f), (std::clamp(v.y * f, 0.f, 255.f)), (std::clamp(v.z * f, 0.f, 255.f)), (std::clamp(v.w * f, 0.f, 255.f)));
	}

	float* ImVec4ToFloatArray(ImVec4 v)
	{
		float arr[4]
		{
			v.x,
			v.y,
			v.z,
			v.w
		};

		return arr;
	}

	ImVec4 FloatArrayToImVec4(float* arr)
	{
		return ImVec4(arr[0], arr[1], arr[2], arr[3]);
	}

	void PreferenceWindow::SetThemeColor(ImVec4 col)
	{
		ImGuiStyle& style = ImGui::GetStyle();

		//style.Colors[ImGuiCol_Header] = col;
		//style.Colors[ImGuiCol_Tab] = col;
		//style.Colors[ImGuiCol_TabActive] = col * 0.95f;
		//style.Colors[ImGuiCol_TabHovered] = col * 1.05f;
		//style.Colors[ImGuiCol_TabSelected] = style.Colors[ImGuiCol_TabHovered];
		//style.Colors[ImGuiCol_TabSelected].w = style.Colors[ImGuiCol_TabHovered].w * 0.95f;
		////style.Colors[ImGuiCol_TabDimmed].w = style.Colors[ImGuiCol_TabHovered].w * 0.95f;
		//style.Colors[ImGuiCol_TabDimmedSelected]= style.Colors[ImGuiCol_Tab] * 0.35f;
	}

	void PreferenceWindow::Init()
	{
		preferences = &mRegistry.GetManager<SessionManager>("Session")->GetPreferences();
	}

	void PreferenceWindow::Draw()
	{
		bool isOpen;
		if (ImGui::Begin("Preferences", &isOpen, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::SeparatorText("Themes");

			if (ImGui::BeginCombo("Theme", EditorThemes[preferences->Theme]))
			{
				for (int i = 0; i < EditorThemes.size(); i++)
				{
					if (ImGui::Selectable(EditorThemes[i]))
					{
						preferences->Theme = EditorThemeType(i);
					}
				}

				ImGui::EndCombo();
			}

			if (ImGui::Button("Save"))
			{
				mRegistry.GetManager<SessionManager>("Session")->SavePreferences();
			}

			ImGui::End();
		}

		if (!isOpen)
		{
			markForRemoval = true;
		}
	}
}