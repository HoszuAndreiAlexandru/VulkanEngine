#pragma once
#include "../UI/Canvas.h"

namespace Engine
{
	class GameScene
	{
	private:

	public:
		GameObject root;
		std::vector<GameObject> lightSources = {};
		std::vector<GameObject> gameObjects = {};
		Canvas canvas;

		GameScene():
			root()
		{
			root.isActive = true;
			root.isStatic = true;
			root.isVisible = false;
			root.isTerrain = true;
		}

		void addGameObject(GameObject gameObject)
		{
			root.addChild(&gameObject);
			if (gameObject.isLight)
			{
				lightSources.push_back(gameObject);
			}

			gameObjects.push_back(gameObject);
		};

		void addGameObjectAsChild(GameObject gameObject, GameObject parent)
		{
			parent.addChild(&gameObject);
		};
	};
};