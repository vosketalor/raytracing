bool intersectPlane(Ray ray, vec3 normal, float dist, out float t) {
    float denom = dot(normal, ray.direction);
    if (abs(denom) < EPSILON) return false;

    float numerator = dot(normal, ray.origin) + dist;
    t = -numerator / denom;

    return t > EPSILON;
};

vec2 getTextureCoordinatesPlane(vec3 point, vec3 normal, float dist) {
    vec3 uAxis;
    if (abs(normal.x) > 0.9) {
        uAxis = vec3(0, 1, 0);
    } else {
        uAxis = vec3(1, 0, 0);
    }

    uAxis = normalize(cross(normal, uAxis));
    vec3 vAxis = normalize(cross(normal, uAxis));

    vec3 origin = normal * (-dist);
    vec3 relative = point - origin;

    float u = dot(relative, uAxis);
    float v = dot(relative, vAxis);

    u = u - floor(u);
    v = v - floor(v);

    return vec2(u, v);
}