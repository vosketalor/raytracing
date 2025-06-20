uniform sampler2D textureAtlas;

vec3 getColorTexture(vec2 uv) {
    vec4 color = texture(textureAtlas, uv);
    return vec3(color.x, color.y, color.z);
}