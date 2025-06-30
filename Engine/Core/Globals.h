#pragma once
#include "Game/GameManager.h"

bool firstFrame = true;

float computeTime = 0;
float rasterTime = 0;

uint32_t maxNumberOfObjects = 1000;
uint32_t currentFrame = 0;
float globalDeltaTime = 0.0f;
float globalDeltaTimeSum = 0.0f;
inline Engine::GameManager gameManager;

const unsigned int numThreads = std::thread::hardware_concurrency();

VkDeviceSize largeBufferSize = 100 * 1024 * 1024; // 100 MB
uint32_t bufferDefaultValue = 25 * 1024 * 1024;  // 25 MB
VkDeviceSize normalBufferSize = bufferDefaultValue;

inline bool usingGpgpuRaytracing = true;

inline bool showOnlyRaytracing = true;