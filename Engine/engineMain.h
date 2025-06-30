#include "Core/Systems/Window.h"
#include "Core/Systems/InputManager.h"
#include <sstream>
#include <string>
#include "Core/Systems/Physics.h"
#include "Core/VulkanRenderer.h"

namespace Engine
{
	class EngineMain
	{
	private: 
		bool gpuTimingInitialized = false;

		void mainLoop()
		{
			while (!window.shouldClose())
			{
				calculatePerformanceMetrics();
				window.pollEvents();
				InputManager::ProcessKeyboardInput();
				physics.stepSimulation(deltaTime);
				gameManager.callEveryOnUpdate();
				vulkanRenderer.renderFrame(deltaTime);
			}
		};

		void calculatePerformanceMetrics()
		{
			if (!gpuTimingInitialized)
			{
				gpuTimingInitialized = true;
				return; // Skip metrics on first frame
			}

			double currentFrameTime = glfwGetTime();
			const double timeSinceLastSecond = currentFrameTime - lastSecond;
			deltaTime = currentFrameTime - lastFrameTime;
			globalDeltaTimeSum += deltaTime;
			globalDeltaTime = deltaTime;
			lastFrameTime = currentFrameTime;
			currentFrameCounter++;

			// Accumulate GPU timings
			auto timestampPeriod = deviceProperties.limits.timestampPeriod;
			computeTime += float(timestamps[4 * currentFrame + 1] - timestamps[4 * currentFrame + 0]) * timestampPeriod / 1'000'000.0;
			rasterTime += float(timestamps[4 * currentFrame + 3] - timestamps[4 * currentFrame + 2]) * timestampPeriod / 1'000'000.0;

			// Over the last second
			if (timeSinceLastSecond >= 1.0)
			{
				// Calculate FPS
				FPS = double(currentFrameCounter) / timeSinceLastSecond;

				// Calculate CPU frame time
				double cpuFrameTimeMs = globalDeltaTimeSum / double(currentFrameCounter);

				// Calculate GPU frame time values
				double computeRayTraceMs = computeTime / double(currentFrameCounter);
				double rasterizationMs = rasterTime / double(currentFrameCounter);

				double totalFrameAverageMs = cpuFrameTimeMs + computeRayTraceMs + rasterizationMs;

				ostringstream s;
				s << "FPS: " << FPS << ", " << totalFrameAverageMs << " ms"
					<< "\nCPU: " << cpuFrameTimeMs << " ms"
					<< "\nCompute ray trace: " << computeRayTraceMs << " ms"
					<< "\nRasterization: " << rasterizationMs << " ms";
				string str = s.str();
				char* cstr = str.data();

				//window.renameWindow(cstr);

				UiElement* element = gameManager.canvases[gameManager.currentCanvas].uiElements["fpsText"];
				UiText* fpsText = dynamic_cast<UiText*>(element);
				if (fpsText)
				{
					fpsText->text = str;
				}

				computeTime = 0;
				rasterTime = 0;

				globalDeltaTimeSum = 0;
				currentFrameCounter = 0;
				lastSecond = currentFrameTime;
				//deltaTime = lastSecond;
			}
		}

	public:
		Window window;
		Physics physics;
		VulkanRenderer vulkanRenderer;

		double deltaTime = 0;
		double lastSecond = 0;
		double lastFrameTime = 0;
		double currentFrameCounter = 0;
		double FPS = 0;

		EngineMain()
			:vulkanRenderer(&window)
		{
			vulkanRenderer.loadAssets("Resources/");
		};

		void run()
		{
			mainLoop();
		};
	};
}