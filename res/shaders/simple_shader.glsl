#version 430
uniform ivec2 resolution;

layout(local_size_x = 16, local_size_y = 16) in;

layout(rgba32f, binding = 0) uniform image2D outputImage;

void main() {
    ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);

    int width = resolution.x;
    int height = resolution.y;

    if (pixel.x >= width || pixel.y >= height)
    return;

    float t = float(pixel.y) / float(height);
    vec4 color = vec4(t, 0.0, 0.0, 1.0);
    imageStore(outputImage, pixel, color);
}
