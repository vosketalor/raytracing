bool intersectSphere(Ray ray, vec3 center, float radius, out float t) {
    vec3 oc = ray.origin - center;
    float a = dot(ray.direction, ray.direction);
    float b = 2.0 * dot(oc, ray.direction);
    float c = dot(oc, oc) - radius * radius;

    float discriminant = b * b - 4.0 * a * c;
    if (discriminant < 0.0) return false;

    float sqrt_disc = sqrt(discriminant);
    float t1 = (-b - sqrt_disc) / (2.0 * a);
    float t2 = (-b + sqrt_disc) / (2.0 * a);

    t = (t1 > EPSILON) ? t1 : t2;
    return t > EPSILON;
};