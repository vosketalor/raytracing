bool intersectSphere(Ray ray, vec3 center, float radius, out float t) {//        shadowRay.direction = lightDir;

    vec3 oc = ray.origin - center;
    float a = dot(ray.direction, ray.direction);
    float b = 2.0 * dot(oc, ray.direction);
    float c = dot(oc, oc) - radius * radius;

    float discriminant = b * b - 4.0 * a * c;
    if (discriminant < 0.0) return false;

    float sqrt_disc = sqrt(discriminant);

    float t1 = (-b - sqrt_disc) / (2.0 * a);
    if (t1 > 0) {
        t = t1;
        return true;
    }

    float t2 = (-b + sqrt_disc) / (2.0 * a);
    if (t2 > 0) {
        t = t2;
        return true;
    }

    return false;
};

float getDistanceNearestEdgeSphere(Ray ray, vec3 point, vec3 center, float radius) {
     // 1. Direction depuis la caméra vers le centre
    vec3 toCenter = center - ray.origin;
    vec3 viewDir = normalize(toCenter);

     // 2. Vecteur du centre vers le point P
    vec3 toPoint = point - center;

     // 3. Projection de P sur le plan orthogonal à viewDir
    float distAlongView = dot(toPoint, viewDir);
    vec3 projected = point - viewDir * distAlongView;

     // 4. Rayon apparent dans ce plan
    float centerDistance = length(toCenter);
    if (centerDistance <= radius) {
        return 0.0;// Caméra à l'intérieur de la sphère : tout est "bord"
    }

     // Calcul du rayon apparent de la sphère dans le plan
    float sinTheta = radius / centerDistance;
    float apparentRadius = sqrt(length(toCenter)*length(toCenter) - radius*radius) * sinTheta;

     // 5. Distance dans le plan
    float distanceInPlane = length(projected - center);

     // 6. Distance au bord visible (disque)
     return abs(distanceInPlane - apparentRadius);
}

vec2 getTextureCoordinatesSphere(vec3 point, vec3 center, float radius) {
    vec3 P = normalize(point - center);
    float u = 0.5 + atan(P.z, P.x) / (2.0 * PI);
    float v = 0.5 - asin(P.y) / PI;

    return vec2(u, v);
}