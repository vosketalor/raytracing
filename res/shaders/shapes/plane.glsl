bool intersectPlane(Ray ray, vec3 normal, vec3 point, out float t) {
    float denom = dot(normal, ray.direction);
    if (abs(denom) < EPSILON) return false;

    t = dot(point - ray.origin, normal) / denom;
    return t > EPSILON;
};