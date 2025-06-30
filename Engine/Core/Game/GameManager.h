#pragma once
#include "GameModel.h"
#include "GameTexture.h"
#include "GameObject.h"
#include "GameCamera.h"
#include "GameScene.h"
#include "../UI/Canvas.h"

namespace Engine
{
	class GameManager
	{
	public:
		std::unordered_map<std::string, GameModel> models;
		std::map<std::string, GameTexture> textures;
		std::map<std::string, GameObject> gameObjects;
		std::map<std::string, GameCamera> gameCameras;
		std::map<std::string, GameScene> gameScenes;
		std::map<std::string, Canvas> canvases;

		std::string currentCamera = "defaultCamera";
		std::string currentScene = "defaultScene";
		std::string currentCanvas = "defaultCanvas";

		GameManager()
		{
			gameCameras["defaultCamera"] = GameCamera();
			gameScenes["defaultScene"] = GameScene();
			canvases["defaultCanvas"] = Canvas();
		};

		GameManager(std::string projectFilePath)
		{
			// TODO 
			// Load project file and initialize game manager's assets
		};

		GameCamera getCurrentCamera()
		{
			if (gameCameras.find(currentCamera) != gameCameras.end())
			{
				return gameCameras[currentCamera];
			}
			else
			{
				std::cerr << "Current camera not found: " << currentCamera << std::endl;
				return GameCamera();
			}
		};

		GameScene getCurrentScene()
		{
			if (gameScenes.find(currentScene) != gameScenes.end())
			{
				return gameScenes[currentScene];
			}
			else
			{
				std::cerr << "Current scene not found: " << currentScene << std::endl;
				return GameScene();
			}
		}

		void callEveryOnUpdate()
		{
			for (auto& [name, gameObject] : gameObjects)
			{
				gameObject.onUpdateInternal();
			}
		}
	};
};