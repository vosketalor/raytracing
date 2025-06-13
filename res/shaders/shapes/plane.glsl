bool intersectPlane(Ray ray, vec3 normal, float dist, out float t) {
    float denom = dot(normal, ray.direction);
    if (abs(denom) < EPSILON) return false;

    float numerator = dot(normal, ray.origin) + dist;
    t = -numerator / denom;

    return t > EPSILON;
};