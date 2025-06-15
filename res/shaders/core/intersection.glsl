#include "includes_structs.glsl"

HitInfo findNearestIntersection(Ray ray) {
    HitInfo hit;
    hit.hit = false;
    hit.t = MAX_DIST;

    for (int i = 0; i < numShapes; i++) {
        float t;
        GPUShape shape = shapes[i];
        bool intersected = intersect(ray, shape, t);

        if (intersected && t > EPSILON && t < hit.t) {
            hit.hit = true;
            hit.t = t;
            hit.point = ray.origin + ray.direction * t;
            hit.normal = getNormal(shape, hit.point);
            hit.color = vec3(shape.color[0], shape.color[1], shape.color[2]);
            hit.shapeIndex = i;
        }
    }

    return hit;
}

