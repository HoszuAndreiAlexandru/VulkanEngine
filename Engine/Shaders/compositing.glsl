#version 450

layout(set = 0, binding = 0, rgba32f) uniform image2D raytracedImage;

layout(set = 0, binding = 1, rgba8) uniform image2D rasterizedImage;

//layout(set = 0, binding = 2, rgba32f) uniform image2D outputImage;

void main() {
    ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);
    
    // Pick a solid color, e.g., Red
    uvec4 color = uvec4(255, 0, 0, 255); // Red in 8-bit unsigned
    
    imageStore(rasterizedImage, pixelCoords, color);
}