#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec3 cameraPos;
    vec3 cameraLookAt;
} ubo;

layout(binding = 1) uniform sampler2D texSampler;

layout(binding = 2, rgba32f) uniform image2D raytracedImage;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragPos;
layout(location = 3) in vec3 fragNormal;

layout(location = 0) out vec4 outColor;

float checkerAnisoAA(vec2 uv, float gridSize) {
    // Scale UV space
    vec2 uvScaled = uv * gridSize;

    // Compute derivatives
    vec2 dx = dFdx(uvScaled);
    vec2 dy = dFdy(uvScaled);

    // Create anisotropic filter footprint
    // The major axis of the pixel footprint in UV space
    float maxLen = max(length(dx), length(dy));

    // Anti-aliasing width
    float aaWidth = maxLen * 0.5;

    // Compute fractional coordinates inside a square
    vec2 f = fract(uvScaled);

    // Anti-aliased edge fading
    float fx = smoothstep(0.0, aaWidth, f.x) * smoothstep(1.0, 1.0 - aaWidth, f.x);
    float fy = smoothstep(0.0, aaWidth, f.y) * smoothstep(1.0, 1.0 - aaWidth, f.y);

    float blend = fx * fy;

    // Basic checker logic
    float checker = mod(floor(uvScaled.x) + floor(uvScaled.y), 2.0);

    // If the footprint is larger than a square, fade out the pattern
    float visibility = clamp(1.0 - maxLen, 0.0, 1.0);

    // Blend toward gray when too fine to avoid moiré
    float color = mix(0.5, checker, visibility);

    return mix(1.0 - checker, checker, blend);
}

void main()
{
    // Rasterized value (using grayscale checker pattern)
    //float value = checkerAnisoAA(fragTexCoord, 100.0);
    float value = 1;

    // Fetch raytraced pixel color
    ivec2 pixelCoords = ivec2(gl_FragCoord.xy);
    vec4 rayColor = imageLoad(raytracedImage, pixelCoords);

    // Combine rasterized value and raytraced pixel
    vec3 combinedColor = mix(fragColor, rayColor.rgb, 0.8);

    outColor = vec4(combinedColor, 1.0);
    //outColor = vec4(rayColor.rgb, 1.0);
    //outColor = rayColor;
}
