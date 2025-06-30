#pragma once
#include "UIElement.h"
#include <string>
#include <glm/glm.hpp>
#include "../../../_externals/imgui/imgui-master/backends/imgui.h"

namespace Engine
{
	class UiText : public UiElement
	{
	public:
		std::string text = "text";
		float posX = 50;
		float posY = 50;
		glm::vec3 textColor = glm::vec3(1, 1, 1);

		UiText() {}

		void render()
		{
			ImGui::SetCursorScreenPos(ImVec2(posX, posY));

			// Customize text color
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f - textColor.r, 1.0f - textColor.g, 1.0f - textColor.b, 1.0f));

			// Render the text
			ImGui::Text("%s", text.c_str());

			ImGui::PopStyleColor(1); // Restore previous color settings
		}
	};
};