#pragma once
#include "UIElement.h"
#include <string>
#include <glm/glm.hpp>
#include "../../../_externals/imgui/imgui-master/backends/imgui.h"

namespace Engine
{
	class UiButton : public UiElement
	{
	public:
		std::string text = "text";
		float posX = 50;
		float posY = 50;
		glm::vec3 buttonColor = glm::vec3(1, 1, 1);
		glm::vec3 textColor = glm::vec3(1, 1, 1);

		UiButton() {}

		void render()
		{
			ImGui::SetCursorScreenPos(ImVec2(posX, posY));

			// Customize button color
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(buttonColor.r, buttonColor.g, buttonColor.b, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(buttonColor.r * 1.2f, buttonColor.g * 1.2f, buttonColor.b * 1.2f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(buttonColor.r * 0.8f, buttonColor.g * 0.8f, buttonColor.b * 0.8f, 1.0f));

			// Customize text color
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f - textColor.r, 1.0f - textColor.g, 1.0f - textColor.b, 1.0f));

			if (ImGui::Button(text.c_str()))
			{
				if (onClick) onClick();
			}

			ImGui::PopStyleColor(4); // Restore previous color settings
		}
	};
};