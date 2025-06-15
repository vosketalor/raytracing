vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float fresnelSchlick(float cosTheta, float F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float fresnelExact(float cosI, float etaI, float etaT) {
    float sinT2 = (etaI / etaT) * (etaI / etaT) * (1.0 - cosI * cosI);

    // Réflexion totale interne
    if (sinT2 >= 1.0) return 1.0;

    float cosT = sqrt(1.0 - sinT2);

    float Rs = (etaI * cosI - etaT * cosT) / (etaI * cosI + etaT * cosT);
    float Rp = (etaT * cosI - etaI * cosT) / (etaT * cosI + etaI * cosT);

    return 0.5 * (Rs * Rs + Rp * Rp);
}

// Version hybride avec support des métaux
vec3 computeFresnel(vec3 rayDir, vec3 normal, float etaI, float etaT, vec3 F0, float metallic) {
    float cosI = abs(dot(-rayDir, normal));

    if (metallic > 0.5) {
        // Matériau métallique : utiliser F0 coloré avec Schlick
        return fresnelSchlick(cosI, F0);
    } else {
        // Matériau diélectrique : utiliser la moyenne de F0 ou calcul exact
        float F0_avg = (F0.r + F0.g + F0.b) / 3.0;
        if (F0_avg > 0.0) {
            float fresnel = fresnelSchlick(cosI, F0_avg);
            return vec3(fresnel); // Fresnel identique sur tous les canaux
        } else {
            // Calcul exact pour diélectriques purs
            float fresnel = fresnelExact(cosI, etaI, etaT);
            return vec3(fresnel);
        }
    }
}