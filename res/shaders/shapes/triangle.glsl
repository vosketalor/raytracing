bool intersectTriangle(Ray ray, vec3 normal, float dist, vec3 A, vec3 B, vec3 C, out float t) {
    if (!intersectPlane(ray, normal, dist, t)) return false;

    vec3 M = ray.origin + ray.direction * t;

    vec3 MA = A - M;
    vec3 MB = B - M;
    vec3 MC = C - M;

    float norme2 = length(normal) * length(normal);

    float alpha = dot(normal, cross(MB, MC));
    float beta = dot(normal, cross(MC, MA));
    float gamma = dot(normal, cross(MA, MB));

    return (alpha >= 0) && (beta >= 0) && (gamma >= 0);
};

vec2 getTextureCoordinatesTriangle(vec3 point, vec3 normal, float dist, vec3 A, vec3 B, vec3 C) {
    vec3 AB = B - A;
    vec3 AC = C - A;
    vec3 AP = point - A;

    float u = dot(AB, AP) / dot(AB, AB);
    float v = dot(AC, AP) / dot(AC, AC);

    return vec2(u, v);
}

float distancePointSegment(vec3 point, vec3 A, vec3 B) {
    vec3 AB = B - A;
    vec3 AP = point - A;

    float ab2 = dot(AB, AB);
    if (ab2 == 0.0) return length(point - A);

    float t = dot(AP, AB) / ab2;
    t = clamp(t, 0.0, 1.0);

    vec3 projection = A + AB * t;
    return length(point - projection);
}

float getDistanceNearestEdgeTriangle(Ray ray, vec3 point, vec3 normal, float dist, vec3 A, vec3 B, vec3 C) {
    float d1 = distancePointSegment(point, A, B);
    float d2 = distancePointSegment(point, B, C);
    float d3 = distancePointSegment(point, C, A);

    return min(d1, min(d2, d3));
}