#include <iostream>
#include "Engine/engineMain.h"

using namespace Engine;

bool shouldRecompile(const std::string& glslPath, const std::string& spvPath)
{
    if (!fs::exists(spvPath))
    {
        return true;
    }
    return fs::last_write_time(glslPath) > fs::last_write_time(spvPath);
}

void tryCompileShader(const std::string& command, const std::string& glslPath, const std::string& spvPath)
{
    if (shouldRecompile(glslPath, spvPath))
    {
        std::cout << "Compiling: " << glslPath << std::endl;
        int result = std::system(command.c_str());
        if (result != 0)
        {
            std::cerr << "Shader compilation failed for: " << glslPath << std::endl;
        }
    }
    else
    {
        std::cout << "Up to date: " << spvPath << std::endl;
    }
}

void compileShadersIfNeeded()
{
    tryCompileShader(
        "glslc Engine/Shaders/shader.vert -o Engine/Shaders/vert.spv",
        "Engine/Shaders/shader.vert",
        "Engine/Shaders/vert.spv"
    );

    tryCompileShader(
        "glslc Engine/Shaders/shader.frag -o Engine/Shaders/frag.spv",
        "Engine/Shaders/shader.frag",
        "Engine/Shaders/frag.spv"
    );

    tryCompileShader(
        "glslc -fshader-stage=compute Engine/Shaders/compute.glsl -O0 -g --target-env=vulkan1.3 -o Engine/Shaders/compute.spv",
        "Engine/Shaders/compute.glsl",
        "Engine/Shaders/compute.spv"
    );

    tryCompileShader(
        "glslc -fshader-stage=compute Engine/Shaders/compositing.glsl -o Engine/Shaders/compositing.spv",
        "Engine/Shaders/compositing.glsl",
        "Engine/Shaders/compositing.spv"
    );
}

int main()
{
    try
    {
        // Compile shaders into SPIR-V
		printf("Compiling shaders:\n");
        auto start = std::chrono::high_resolution_clock::now();

        compileShadersIfNeeded();
        //system("compile.bat");

        auto end1 = std::chrono::high_resolution_clock::now();
        auto duration1 = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start).count();
        printf("\nTime taken: %lld ms\n\nStarting engine\n", duration1);

        // Run the engine with the default scene
        EngineMain gameEngine;
        gameEngine.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}