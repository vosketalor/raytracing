#version 430

layout(local_size_x = 16, local_size_y = 16) in;

layout(rgba32f, binding = 0) uniform image2D outputImage;

void main() {
    ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);

    int width = 800;  // ou ta valeur
    int height = 600;

    if (pixel.x >= width || pixel.y >= height)
    return;

    float t = float(pixel.x) / float(width);
    vec4 color = vec4(t, 0.0, 0.0, 1.0);
    imageStore(outputImage, pixel, color);
}
