#version 450
#extension GL_EXT_debug_printf : enable

layout(local_size_x = 16, local_size_y = 16) in;

#define PI 3.1415926538

#define EPSILON 0.0001f
#define SMALL_EPSILON 0.00000001f

// ========== BUFFER SIZES ==========
layout(push_constant, std430) uniform PushConstants
{
    int nodeCount;
    int triangleCount;
    int instanceCount;
    int lightCount;
};

// ========== OUTPUT IMAGE ==========
layout(set = 0, binding = 0, rgba32f) uniform image2D outputImage;

// ========== CAMERA ==========
struct Camera
{
    vec4 position;
    vec4 direction;
    vec4 up;
    vec4 right;
    vec4 screenCenter;
    vec4 screenDimensions;

    float verticalFovRadians;
    float tanFov;
    float aspect;

    mat4 projectionMatrix;
    mat4 viewMatrix;
};

layout(set = 0, binding = 1) uniform CameraBuffer
{
    Camera camera;
};

// ========== BVH NODES ==========
struct BVHNode
{
    vec3 boundMin;
    float pad0;

    vec3 boundMax;
    float pad1;

    int left;
    int right;
    int firstTriangle;
    int triangleCount;
};

layout(std430, set = 0, binding = 2) buffer BVHBuffer
{
    BVHNode nodes[];
};

// ========== TRIANGLES ==========
struct Triangle
{
    vec4 v0;
    vec4 v1;
    vec4 v2;
    vec4 c;
};

layout(std430, set = 0, binding = 3) buffer TriangleBuffer
{
    Triangle triangles[];
};

// ========== INSTANCES ==========
struct BVHInstance
{
    mat4 modelMatrix;
    mat4 inverseModelMatrix;

    int bvhRootNodeIndex;
    int triangleOffset;
    int triangleCount;
    int pad0;
};

layout(std430, set = 0, binding = 4) buffer InstanceBuffer
{
    BVHInstance instances[];
};

// ========== LIGHTS ==========
struct LightInstance
{
    vec3 position;
    float pad0;

    vec3 color;
    float pad1;

    float intensity;
    float radius;
    float pad2;
    float pad3;
};

layout(std430, set = 0, binding = 5) buffer LightBuffer
{
    LightInstance lights[];
};

// ========== UTILITY STRUCTS ==========
// Simple ray structure
struct Ray {
    vec3 origin;
    vec3 direction;
    vec3 inverseDirection;
    float t;
    float d;
};

// Output basic hit info
struct HitInfo {
    float t;
    vec3 position;
    vec3 normal;
    vec3 color;
    bool hit;
};

// ========== UTILITY FUNCTIONS ==========
// AABB ray-box intersection
bool intersectAABB(Ray ray, BVHNode node, out vec2 intersect)
{
    vec3 t0 = (node.boundMin - ray.origin) * ray.inverseDirection;
    vec3 t1 = (node.boundMax - ray.origin) * ray.inverseDirection;

    vec3 temp = t0;
    t0 = min(t0, t1);  // tNear
    t1 = max(temp, t1);// tFar

    intersect.x = max(max(t0.x, t0.y), t0.z);
    intersect.y = min(min(t1.x, t1.y), t1.z);

    // Check if the ray intersects the AABB
    return intersect.x <= intersect.y + EPSILON && intersect.y >= 0.0;
}

// Möller–Trumbore ray-triangle intersection
bool intersectRayTriangle(Ray ray, Triangle tri, out float t, out vec3 normal)
{
    vec3 edge1 = (tri.v1 - tri.v0).xyz;
    vec3 edge2 = (tri.v2 - tri.v0).xyz;
    vec3 h = cross(ray.direction, edge2);
    float a = dot(edge1, h);
    if (abs(a) < SMALL_EPSILON) return false;

    float f = 1.0 / a;
    vec3 s = (ray.origin - tri.v0.xyz).xyz;
    a = f * dot(s, h);
    if (a < 0.0 || a > 1.0) return false;

    s = cross(s, edge1);
    float v = f * dot(ray.direction, s);
    if (v < 0.0 || a + v > 1.0) return false;

    t = f * dot(edge2, s);
    if (t > SMALL_EPSILON)
    {
        normal = normalize(cross(edge1, edge2));
        return true;
    }
    return false;
}

void IntersectTri(Ray ray, const Triangle tri)
{
    const vec3 edge1 = (tri.v1 - tri.v0).xyz;
    const vec3 edge2 = (tri.v2 - tri.v0).xyz;
    const vec3 h = cross(ray.direction, edge2);
    const float a = dot(edge1, h);
    if (a > -0.0001f && a < 0.0001f) return; // ray parallel to triangle
    const float f = 1 / a;
    const vec3 s = ray.origin.xyz - tri.v0.xyz;
    const float u = f * dot(s, h);
    if (u < 0 || u > 1) return;
    const vec3 q = cross(s, edge1);
    const float v = f * dot(ray.direction.xyz, q);
    if (v < 0 || u + v > 1) return;
    const float t = f * dot(edge2, q);
    if (t > 0.0001f) ray.t = min(ray.t, t);
}

float radicalInverse_VdC(uint bits)
{
    bits = (bits << 16) | (bits >> 16);
    bits = ((bits & 0x55555555u) << 1) | ((bits & 0xAAAAAAAAu) >> 1);
    bits = ((bits & 0x33333333u) << 2) | ((bits & 0xCCCCCCCCu) >> 2);
    bits = ((bits & 0x0F0F0F0Fu) << 4) | ((bits & 0xF0F0F0F0u) >> 4);
    bits = ((bits & 0x00FF00FFu) << 8) | ((bits & 0xFF00FF00u) >> 8);
    return float(bits) * 2.3283064365386963e-10; // 1.0 / 2^32
}

vec2 hammersley2d(int i, int N)
{
    return vec2(float(i) / float(N), radicalInverse_VdC(uint(i)));
}

vec2 hammersley2dOriginal(int i, int N)
{
    float rdi = 0.0;
    float f = 0.5;
    int id = i;
    while (id > 0)
    {
        rdi += float(id & 1) * f;
        id >>= 1;
        f *= 0.5;
    }
    return vec2(float(i) / float(N), rdi);
}

vec2 sampleConcentricDisk(vec2 u)
{
    float a = 2.0 * u.x - 1.0;
    float b = 2.0 * u.y - 1.0;

    float r, phi;

    // Determine radius and angle using a branchless approach
    float absA = abs(a);
    float absB = abs(b);

    bool condition = absA > absB;

    float r1 = absA;
    float phi1 = (PI / 4.0) * (b / a);

    float r2 = absB;
    float phi2 = (PI / 2.0) - (PI / 4.0) * (a / b);

    r = mix(r2, r1, float(condition));
    phi = mix(phi2, phi1, float(condition));

    return r * vec2(cos(phi), sin(phi));
}

vec2 sampleConcentricDiskOriginal(vec2 u)
{
    float a = 2.0 * u.x - 1.0;
    float b = 2.0 * u.y - 1.0;

    if (a == 0.0 && b == 0.0) return vec2(0.0);

    float r, theta;
    if (abs(a) > abs(b))
    {
        r = a;
        theta = (PI / 4.0) * (b / a);
    }
    else
    {
        r = b;
        theta = (PI / 2.0) - (PI / 4.0) * (a / b);
    }

    return r * vec2(cos(theta), sin(theta));
}

// ========== BVH TRAVERSAL ==========

HitInfo traceRay(Ray ray)
{
    HitInfo closestHit;
    closestHit.t = 1e20;
    closestHit.hit = false;

    const int maxStackDepth = 256;
    //instanceCount
    for (int instID = 0; instID < 1; ++instID)
    {
        instID = 0;
        BVHInstance instance = instances[instID];
        mat3 instanceNormalMatrix = transpose(inverse(mat3(instance.modelMatrix)));

        int stack[maxStackDepth];
        int stackIndex = 0;
        stack[stackIndex++] = instances[instID].bvhRootNodeIndex;

        // Transform ray origin and direction into instance local space
        Ray localRay;
        localRay.origin = (instance.inverseModelMatrix * vec4(ray.origin, 1.0)).xyz;
        localRay.direction = (instance.inverseModelMatrix * vec4(ray.direction, 0.0)).xyz;

        while (stackIndex > 0 && stackIndex < maxStackDepth)
        {
            int nodeIndex = stack[--stackIndex];
            if (nodeIndex < 0)
            {
                continue;
            }
            BVHNode currentNode = nodes[nodeIndex];

            vec2 intersect;
            bool rayIntersectsAABB = intersectAABB(ray, currentNode, intersect);
            if (!rayIntersectsAABB)
            {
                continue;
            }

            if (currentNode.triangleCount <= 0)
            {
                stack[stackIndex++] = currentNode.left;
                stack[stackIndex++] = currentNode.right;
                continue;
            }

            for (int i = 0; i < currentNode.triangleCount; ++i)
            {
                Triangle currentTriangle = triangles[instance.triangleOffset + currentNode.firstTriangle + i];
                float t;
                vec3 n;
                if (intersectRayTriangle(ray, currentTriangle, t, n))
                {
                    if (t < closestHit.t)
                    {
                        closestHit.t = t;

                        closestHit.position = (instance.modelMatrix * vec4(localRay.origin + t * localRay.direction, 1.0)).xyz;
                        closestHit.normal = normalize(instanceNormalMatrix * n);

                        closestHit.hit = true;
                    }
                }
            }
        }
    }

    return closestHit;
}

HitInfo traceRayNoAS(Ray ray)
{
    HitInfo closestHit;
    closestHit.t = 1e20;
    closestHit.hit = false;

    //for (int i = 0; i < 144046; i++) // bunny
    //for (int i = 144046; i < 149742; i++) // model
    //for (int i = 149742; i < 149744; i++) // quad
    for (int i = 149744; i < 150768; ++i) // teapot
    //for (int i = 150768; i < 154596; i++) // viking room
    {
        float t;
        vec3 n;
        bool intersected = intersectRayTriangle(ray, triangles[i], t, n);
        if (intersected && t < closestHit.t)
        {
            closestHit.t = t;
            closestHit.position = ray.origin + t * ray.direction;
            closestHit.normal = n;
            closestHit.hit = true;
        }
    }
    return closestHit;
}

HitInfo visualizeBVHRoot(Ray ray)
{
    HitInfo closestHit;
    closestHit.t = 1e20;
    closestHit.hit = false;

    BVHInstance instance = instances[0];

    mat4 modelMatrix = instance.modelMatrix;
    mat4 inverseModelMatrix = inverse(modelMatrix);

    Ray localRay;
    localRay.origin = (inverseModelMatrix * vec4(ray.origin, 1.0)).xyz;
    localRay.direction = (inverseModelMatrix * vec4(ray.direction, 0.0)).xyz;
    localRay.inverseDirection = 1.0 / max(abs(localRay.direction), vec3(1e-8)) * sign(localRay.direction);

    vec2 intersect;
    bool rayIntersectsAABB = intersectAABB(localRay, nodes[instance.bvhRootNodeIndex], intersect);

    if (rayIntersectsAABB)
    {
        closestHit.hit = true;
    }

    return closestHit;
}

HitInfo visualizeBVHLeaves(Ray ray)
{
    HitInfo closestHit;
    closestHit.t = 1e20;
    closestHit.hit = false;

    const int stackSize = 256;

    for(int i = 0; i < instanceCount; ++i)
    {
        if(i == 3)
        {
            continue;
        }

        BVHInstance instance = instances[i];

        mat4 modelMatrix = instance.modelMatrix;
        mat4 inverseModelMatrix = inverse(modelMatrix);

        int stack[stackSize];
        int stackIndex = 0;
        stack[stackIndex++] = instance.bvhRootNodeIndex;

        Ray localRay;
        localRay.origin = (inverseModelMatrix * vec4(ray.origin, 1.0)).xyz;
        localRay.direction = (inverseModelMatrix * vec4(ray.direction, 0.0)).xyz;
        localRay.inverseDirection = 1.0 / max(abs(localRay.direction), vec3(1e-8)) * sign(localRay.direction);

        while (stackIndex > 0)
        {
            int nodeIndex = stack[--stackIndex];
            if (nodeIndex < 0) continue;

            BVHNode currentNode = nodes[nodeIndex];
            vec2 intersect;
            bool rayIntersectsAABB = intersectAABB(localRay, currentNode, intersect);

            if (!rayIntersectsAABB || intersect.x > closestHit.t)
            {
                continue;
            }

            if (currentNode.triangleCount <= 0)
            {
                // Internal node: push children
                if (stackIndex < stackSize) stack[stackIndex++] = currentNode.left;
                if (stackIndex < stackSize) stack[stackIndex++] = currentNode.right;
                continue;
            }

            // Leaf node
            if (rayIntersectsAABB)
            {
                //closestHit.color = vec3();
                closestHit.hit = true;
            }
        }
    }

    return closestHit;
}

HitInfo traceRay2(Ray ray)
{
    HitInfo closestHit;
    closestHit.t = 1e20;
    closestHit.hit = false;

	const int stackSize = 64;

    for(int i = 0; i < instanceCount; ++i)
    {
        BVHInstance instance = instances[i];

        mat4 modelMatrix = instance.modelMatrix;
        mat4 inverseModelMatrix = inverse(modelMatrix);

        int stack[stackSize];
        int stackIndex = 0;
        stack[stackIndex++] = instance.bvhRootNodeIndex;

        Ray localRay;
        localRay.origin = (inverseModelMatrix * vec4(ray.origin, 1.0)).xyz;
        localRay.direction = (inverseModelMatrix * vec4(ray.direction, 0.0)).xyz;
        localRay.inverseDirection = 1.0 / max(abs(localRay.direction), vec3(1e-8)) * sign(localRay.direction);

        while (stackIndex > 0)
        {
            int nodeIndex = stack[--stackIndex];
            if (nodeIndex < 0) continue;

            BVHNode currentNode = nodes[nodeIndex];
            vec2 intersect;
            bool rayIntersectsAABB = intersectAABB(localRay, currentNode, intersect);

            if (!rayIntersectsAABB || intersect.x > closestHit.t)
            {
                continue;
            }

            if (currentNode.triangleCount <= 0)
            {
                // Internal node: push children
                if (stackIndex < stackSize) stack[stackIndex++] = currentNode.left;
                if (stackIndex < stackSize) stack[stackIndex++] = currentNode.right;
                continue;
            }

            // Leaf node: check triangles
			int triangleOffset = instance.triangleOffset + currentNode.firstTriangle;
			int triangleCount = currentNode.triangleCount;

            for (int i = triangleOffset; i < triangleOffset + triangleCount; ++i)
            {
                float t;
                vec3 n;
                bool intersected = intersectRayTriangle(localRay, triangles[i], t, n);
                if (intersected && t < closestHit.t)
                {
                    closestHit.t = t;
                    closestHit.position = (modelMatrix * vec4(localRay.origin + t * localRay.direction, 1.0)).xyz;
                    closestHit.hit = true;
					closestHit.normal = normalize(mat3(modelMatrix) * n);
                }
            }
        }
	}

    return closestHit;
}

HitInfo isInShadow(vec3 point, vec3 toLight, float maxDist)
{
    Ray shadowRay;
    shadowRay.origin = point + 0.1 * toLight;
    shadowRay.direction = toLight;
    shadowRay.inverseDirection = 1.0 / shadowRay.direction;

    HitInfo shadowHit = traceRay2(shadowRay);
    //return shadowHit.hit && shadowHit.t < maxDist;
    //return shadowHit.t;
    return shadowHit;
}

vec3 sampleDiskPosition(vec3 origin, vec3 observer, float radius, int number, int max_samples)
{
    float angle = float(number) / float(max_samples) * 6.2831853; // 2π
    vec2 diskPos = vec2(cos(angle), sin(angle)) * radius;

    // Disk direction: face observer
    vec3 forward = normalize(observer - origin);

    // Project disk in a plane perpendicular to forward
    vec3 up = vec3(0.0, 1.0, 0.0);
    if (abs(dot(forward, up)) > 0.99) up = vec3(1.0, 0.0, 0.0); // avoid degenerate case

    vec3 right = normalize(cross(up, forward));
    vec3 realUp = normalize(cross(forward, right));

    // Construct rotated disk point
    vec3 offset = diskPos.x * right + diskPos.y * realUp;
    return origin + offset;
}

/*
vec3 rayTrace3(Ray primaryRay)
{
    vec3 pixelColor = vec3(0.0);

    // Choose your poison
    //HitInfo hit = traceRay(primaryRay);
    //HitInfo hit = traceRayNoAS(primaryRay);
	HitInfo hit = traceRay2(primaryRay);
    //HitInfo hit = visualizeBVHRoot(primaryRay);
    //HitInfo hit = visualizeBVHLeaves(primaryRay);

    if (hit.hit)
    {
        //pixelColor = vec3(0, 1, 0);
        

        LightInstance light;
        light.position = vec3(1.0, 35.0, 0.0);
        light.color = vec3(1.0, 1.0, 1.0);
        light.radius = 10.0;

        vec3 toLight = light.position - hit.position;
        float distToLight = length(toLight);
        vec3 lightDir = normalize(toLight);
        
        //if (!isInShadow(shadowRayOrigin, shadowRayDir, sampleDist))
        //{
        //    shadowFactor += 1.0;
        //}

        float ndotl = max(dot(hit.normal, lightDir), 0.0);

        const int numShadowSamples = 16;
        float shadowFactor = 0.0;

        vec3 up = abs(lightDir.y) < 0.999 ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 0.0);
        vec3 tangent = normalize(cross(lightDir, up));
        vec3 bitangent = cross(lightDir, tangent);

        for (int i = 0; i < numShadowSamples; ++i)
        {
            //vec2 xi = hammersley2dOriginal(i, numShadowSamples);
            //vec2 diskSample = sampleConcentricDiskOriginal(xi);
            //vec3 offset = (tangent * diskSample.x + bitangent * diskSample.y) * light.radius;
            //vec3 samplePosition = light.position + offset;
            vec3 samplePosition = sampleDiskPosition(light.position, hit.position, light.radius, i, numShadowSamples);

            vec3 shadowRayDir = normalize(samplePosition - hit.position);
            float sampleDist = length(samplePosition - hit.position);

            vec3 shadowRayOrigin = hit.position + hit.normal * 0.001;

            if (!isInShadow(shadowRayOrigin, shadowRayDir, sampleDist))
            {
                shadowFactor += 1.0;
            }
        }

        shadowFactor /= float(numShadowSamples);

        pixelColor += light.color * ndotl * shadowFactor;

        
        
    }

    return pixelColor;
}
*/

float rand(vec2 co) {
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

vec3 rayTrace(Ray primaryRay)
{
    vec3 pixelColor = vec3(0.0);
    HitInfo hit = traceRay2(primaryRay);

    if (hit.hit)
    {
        LightInstance light;
        light.position = vec3(1.0, 10035.0, 0.0);
        light.color = vec3(1.0, 1.0, 1.0);
        light.radius = 1000.0;

        const int numShadowSamples = 3;
        float shadowFactor = 0.0;

        //double d = 0;
        HitInfo shadowHit;

        for (int i = 0; i < numShadowSamples; ++i)
        {
            // Jittering (you can use a real seed per-pixel for better randomness)
            vec2 seed = vec2(float(i), dot(hit.position.xy, vec2(12.9898, 78.233)));
            float angle = (float(i) + rand(seed)) / float(numShadowSamples) * 6.2831853;
            float r = sqrt(rand(seed + 1.23));
            vec2 diskPos = r * vec2(cos(angle), sin(angle)) * light.radius;

            // Build disk aligned to light -> hit (more stable than camera-facing)
            vec3 forward = normalize(hit.position - light.position);
            vec3 up = abs(dot(forward, vec3(0.0, 1.0, 0.0))) > 0.99 ? vec3(1.0, 0.0, 0.0) : vec3(0.0, 1.0, 0.0);
            vec3 right = normalize(cross(up, forward));
            vec3 realUp = normalize(cross(forward, right));
            vec3 offset = diskPos.x * right + diskPos.y * realUp;

            vec3 samplePosition = light.position + offset;

            vec3 toSample = samplePosition - hit.position;
            float sampleDist = length(toSample);
            vec3 shadowRayDir = normalize(toSample);

            vec3 shadowRayOrigin = hit.position + hit.normal * 0.001;

            //return shadowHit.hit && shadowHit.t < maxDist;
            shadowHit = isInShadow(shadowRayOrigin, shadowRayDir, sampleDist);
            //d = shadowHit.t;
            if (!(shadowHit.hit && shadowHit.t < sampleDist))
            {
                shadowFactor += 1.0;
            }
        }

        shadowFactor /= float(numShadowSamples) + 1;

        // Base lighting
        vec3 toLight = light.position - hit.position;
        float distToLight = length(toLight);
        vec3 lightDir = normalize(toLight);
        float ndotl = max(dot(hit.normal, lightDir), 0.0);

        float distToBlocker = length(shadowHit.position - hit.position);

        // Optional attenuation
        float attenuation = 1.0;// / (distToLight * distToLight);
        float penumbraBias = 1.0;//smoothstep(0.0, distToBlocker, distToLight);

        pixelColor += light.color * ndotl * shadowFactor * attenuation * penumbraBias;
    }

    return pixelColor;
}

// ========== MAIN ==========

void main()
{
    ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);
    ivec2 imageSize = imageSize(outputImage);
    //if (pixelCoords.x >= imageSize.x || pixelCoords.y >= imageSize.y)
    //{
    //    return;
    //}

    vec2 uv = (vec2(pixelCoords) + 0.5) / vec2(imageSize) * 2.0 - 1.0;
    uv.y = -uv.y;

    vec3 rayDir = normalize(
        uv.x * camera.right.xyz +
        uv.y * camera.up.xyz +
        camera.direction.xyz
    );

    Ray primaryRay;
    primaryRay.origin = camera.position.xyz;
    primaryRay.direction = rayDir;
    primaryRay.inverseDirection = 1.0 / primaryRay.direction;

    imageStore(outputImage, pixelCoords, vec4(rayTrace(primaryRay), 1.0));
}