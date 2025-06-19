#version 430
layout(local_size_x = 1, local_size_y = 1) in; // 1 thread par picking

#include "includes_structs.glsl"
#include "core/bvh.glsl"
#include "core/intersection.glsl"

uniform ivec2 u_ClickCoord;
layout(std430, binding = 5) buffer PickResult {
    int pickedShapeID;
} pick;

void main() {
    ivec2 pixelCoord = u_ClickCoord;

    if (pixelCoord.x < 0 || pixelCoord.x >= resolution.x ||
        pixelCoord.y < 0 || pixelCoord.y >= resolution.y) {
        atomicExchange(pick.pickedShapeID, -1);
        return;
    }

    vec2 uv = (vec2(pixelCoord) + 0.5) / vec2(resolution);

    vec2 ndc = uv * 2.0 - 1.0;

    float fovRad = fov * 3.14159265359 / 180.0;
    float screenHeight = 2.0 * tan(fovRad / 2.0);
    float screenWidth = screenHeight * aspectRatio;

    vec3 rayDir = normalize(
        cameraDir +
        (ndc.x * screenWidth * 0.5) * cameraRight +
        (ndc.y * screenHeight * 0.5) * cameraUp
    );

    Ray ray;
    ray.origin = cameraPos;
    ray.direction = rayDir;

    HitInfo hit = findNearestIntersectionBVH(ray);

    atomicExchange(pick.pickedShapeID, hit.hit ? hit.shapeIndex : -1);

}