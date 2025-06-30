C:/VulkanSDK/1.3.280.0/Bin/glslc.exe Engine/Shaders/shader.vert -o Engine/Shaders/vert.spv
C:/VulkanSDK/1.3.280.0/Bin/glslc.exe Engine/Shaders/shader.frag -o Engine/Shaders/frag.spv
C:/VulkanSDK/1.3.280.0/Bin/glslc.exe -fshader-stage=compute Engine/Shaders/compute.glsl -g --target-env=vulkan1.3 -o Engine/Shaders/compute.spv
C:/VulkanSDK/1.3.280.0/Bin/glslc.exe -fshader-stage=compute Engine/Shaders/compositing.glsl -o Engine/Shaders/compositing.spv

::C:/VulkanSDK/1.3.280.0/Bin/glslc.exe shaders/raygen.rgen -o shaders/raygen.spv
::C:/VulkanSDK/1.3.280.0/Bin/glslc.exe shaders/miss.rmiss -o shaders/miss.spv
::C:/VulkanSDK/1.3.280.0/Bin/glslc.exe shaders/shadowmiss.rmiss -o shaders/shadowmiss.spv
::C:/VulkanSDK/1.3.280.0/Bin/glslc.exe shaders/closesthit.rchit -o shaders/closesthit.spv