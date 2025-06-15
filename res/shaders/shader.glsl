#version 430

layout(local_size_x = 16, local_size_y = 16) in;
layout(rgba32f, binding = 0) uniform image2D outputImage;

#include "includes_structs.glsl"

#include "core/intersection.glsl"
#include "shading/shadow.glsl"
#include "shading/reflection.glsl"
#include "shading/refraction.glsl"
#include "shading/lighting.glsl"
#include "core/trace.glsl"

void main() {
    ivec2 pixelCoord = ivec2(gl_GlobalInvocationID.xy);
    if (pixelCoord.x >= resolution.x || pixelCoord.y >= resolution.y) return;

    vec2 uv = (vec2(pixelCoord) + 0.5) / vec2(resolution);

    vec2 ndc = uv * 2.0 - 1.0;

    float fovRad = fov * 3.14159265359 / 180.0;
    float screenHeight = 2.0 * tan(fovRad / 2.0);
    float screenWidth = screenHeight * aspectRatio;

    vec3 rayDir = normalize(cameraDir +
                            ndc.x * screenWidth * 0.5 * cameraRight +
    ndc.y * screenHeight * 0.5 * cameraUp);

    Ray ray;
    ray.origin = cameraPos;
    ray.direction = rayDir;

    vec3 color = traceRay(ray);

    imageStore(outputImage, pixelCoord, vec4(color, 1.0));
}
