uniform int oversamplingFactor;
uniform bool oversamplingEnabled;

vec3 oversample(ivec2 pixelCoord) {
    vec3 finalColor = vec3(0.0);
    float subPixelSize = 1.0 / float(oversamplingFactor);
    float halfSubPixel = subPixelSize * 0.5;

    // Pré-calcul des constantes
    float fovRad = fov * 3.14159265359 / 180.0;
    float screenHeight = 2.0 * tan(fovRad / 2.0);
    float screenWidth = screenHeight * aspectRatio;

    for (int x = 0; x < oversamplingFactor; x++) {
        for (int y = 0; y < oversamplingFactor; y++) {
            // Offset centré dans le sous-pixel
            vec2 offset = vec2(
            float(x) * subPixelSize + halfSubPixel,
            float(y) * subPixelSize + halfSubPixel
            );

            // Calcul UV/NDC identique au main mais avec l'offset de sous-pixel
            vec2 uv = (vec2(pixelCoord) + offset) / vec2(resolution);
            vec2 ndc = uv * 2.0 - 1.0;

            // Génération du rayon
            vec3 rayDir = normalize(cameraDir +
                                    ndc.x * screenWidth * 0.5 * cameraRight +
            ndc.y * screenHeight * 0.5 * cameraUp);

            Ray ray;
            ray.origin = cameraPos;
            ray.direction = rayDir;

            // Accumulation de la couleur
            finalColor += traceRay(ray);
        }
    }

    // Moyenne des échantillons
    return finalColor / float(oversamplingFactor * oversamplingFactor);
}