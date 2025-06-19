bool handleWireframe(Ray ray, vec3 point, int shapeIndex, out vec3 color) {
    float dist = getDistanceNearestEdge(ray, point, shapeIndex);
    if (dist <= wireframeThickness) {
        color =  wireframeColor;
        return true;
    }
    return false;
}