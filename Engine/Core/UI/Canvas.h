#pragma once
#include <functional>
#include "../../../_externals/imgui/imgui-master/backends/imgui.h"
#include "UIElement.h"
#include <string>
#include <map>

namespace Engine
{
	class Canvas
	{
	private:
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoScrollbar |
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoBackground;

	public:
		std::map<std::string, UiElement*> uiElements = { };

		Canvas() {}

		void render(int windowWidth, int windowHeight)
		{
			ImVec2 windowSize = ImVec2{ (float)windowWidth, (float)windowHeight };
			ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
			ImGui::SetNextWindowSize(windowSize);

			if (ImGui::Begin("TransparentWindow", nullptr, window_flags))
			{
				for (auto pair : uiElements)
				{
					pair.second->render();
				}
			}
			ImGui::End();
		}
	};
};