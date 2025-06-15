#include "includes_structs.glsl"

bool isInShadow(HitInfo shadowHit, float lightDist) {

    return shadowHit.hit && shadowHit.t > EPSILON && shadowHit.t < lightDist;
}