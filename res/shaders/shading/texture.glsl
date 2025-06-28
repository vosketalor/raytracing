uniform sampler2D textureAtlas;
uniform vec2 textureAtlasSize;

struct SubTexture {
    int x, y;
    int width, height;
};

layout(std430, binding = 6) buffer TextureData {
    SubTexture textures[];
};

vec3 getColorTexture(vec2 uv, int textureIndex) {
    SubTexture subTex = textures[textureIndex];
    vec2 subTexPos = vec2(subTex.x, subTex.y);
    vec2 subTexSize = vec2(subTex.width, subTex.height);
    vec2 newUV = (subTexPos + uv * subTexSize) / textureAtlasSize;

    vec4 color = texture(textureAtlas, newUV);
    return vec3(color.x, color.y, color.z);
}

