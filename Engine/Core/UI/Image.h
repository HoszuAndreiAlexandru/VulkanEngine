#pragma once
#include "UIElement.h"
#include <string>
#include <glm/glm.hpp>
#include "../../../_externals/imgui/imgui-master/backends/imgui.h"
#include "vulkan/vulkan.h"
#include "../Globals.h"

namespace Engine
{
	class UiImage : public UiElement
	{
	public:
		float posX = 50;
		float posY = 50;
		ImTextureID textureID = reinterpret_cast<ImTextureID>(gameManager.textures["default"].id);
		ImVec2 size = gameManager.textures["default"].size;

		UiImage() {}

		void setImage(std::string image)
		{
			textureID = reinterpret_cast<ImTextureID>(gameManager.textures[image].id);
			size = gameManager.textures[image].size;
		}

		void render()
		{
			ImGui::SetCursorScreenPos(ImVec2(posX, posY));

			if (textureID)
			{
				ImGui::Image(textureID, size);
			}
		}
	};
};