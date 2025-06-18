vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float fresnelSchlick(float cosTheta, float F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float fresnelExact(float cosI, float etaI, float etaT) {
    cosI = clamp(cosI, -1.0, 1.0);
    float sinT2 = (etaI / etaT) * (etaI / etaT) * (1.0 - cosI * cosI);

    // Réflexion totale interne
    if (sinT2 >= 1.0) return 1.0;

    float cosT = sqrt(1.0 - sinT2);

    float Rs = (etaI * cosI - etaT * cosT) / (etaI * cosI + etaT * cosT);
    float Rp = (etaT * cosI - etaI * cosT) / (etaT * cosI + etaI * cosT);

    return 0.5 * (Rs * Rs + Rp * Rp);
}

vec3 computeFresnel(vec3 rayDir, vec3 normal, float etaI, float etaT, vec3 F0, float metallic) {
    float cosI = abs(dot(-normalize(rayDir), normalize(normal)));

    if (metallic > 0.5) {
        // Matériau métallique - utilise F0 directement (déjà vectoriel)
        return fresnelSchlick(cosI, F0);
    } else {
        // Matériau diélectrique
        if (length(F0) > 0.1) {
            // F0 défini - utilise Schlick vectoriel
            return fresnelSchlick(cosI, F0);
        } else {
            // Calcul exact du Fresnel pour diélectriques avec dispersion simple
            // Simulation basique de dispersion chromatique
            vec3 etaRGB = vec3(1.513, 1.517, 1.528); // Pour verre crown (BK7)

            vec3 fresnelValues;
            fresnelValues.r = fresnelExact(cosI, etaI, etaRGB.r);
            fresnelValues.g = fresnelExact(cosI, etaI, etaRGB.g);
            fresnelValues.b = fresnelExact(cosI, etaI, etaRGB.b);

            return fresnelValues;
        }
    }
}
