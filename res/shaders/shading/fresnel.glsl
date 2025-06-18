vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float fresnelSchlick(float cosTheta, float F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

//vec3 fresnelExact(vec3 rayDir, vec3 normal, float etaI, vec3 etaT, vec3 k) {
//    float cosI = clamp(dot(-normalize(rayDir), normalize(normal)), 0.0, 1.0);
//    float sinI = sqrt(1.0 - cosI * cosI);
//    vec3 fresnel = vec3(0.0);
//
//    for (int i = 0; i < 3; i++) { // Boucle sur RGB
//      // Calcul des coefficients complexes
//      float sinT2 = (etaI / etaT[i]) * (etaI / etaT[i]) * (1.0 - cosI * cosI);
//      if (sinT2 >= 1.0) {
//          fresnel[i] = 1.0; // Réflexion totale interne
//          continue;
//      }
//
//      float cosT = sqrt(1.0 - sinT2);
//
//      // Termes complexes pour les matériaux conducteurs
//      float a = (etaT[i] * etaT[i] - k[i] * k[i] - etaI * etaI * sinI * sinI);
//      float b = sqrt(a * a + 4.0 * etaT[i] * etaT[i] * k[i] * k[i]);
//      float u = sqrt(0.5 * (a + b));
//      float v = sqrt(0.5 * (-a + b));
//
//      // Coefficients de Fresnel
//      float Rs = ((etaI * cosI - u) * (etaI * cosI - u) + v * v) /
//      ((etaI * cosI + u) * (etaI * cosI + u) + v * v);
//      float Rp = ((etaT[i] * etaT[i] + k[i] * k[i]) * cosI * cosI -
//      2.0 * etaI * u * cosI + etaI * etaI * (u * u + v * v)) /
//      ((etaT[i] * etaT[i] + k[i] * k[i]) * cosI * cosI +
//      2.0 * etaI * u * cosI + etaI * etaI * (u * u + v * v));
//
//      fresnel[i] = 0.5 * (Rs + Rp);
//    }
//    return fresnel;
//}
//vec3 computeFresnel(vec3 rayDir, vec3 normal, float etaI, float etaT, vec3 F0, float metallic) {
//    float cosI = abs(dot(-normalize(rayDir), normalize(normal)));
//
//    if (metallic > 0.5) {
//        // Matériau métallique - utilise F0 directement (déjà vectoriel)
//        return fresnelSchlick(cosI, F0);
//    } else {
//        // Matériau diélectrique
//        if (length(F0) > 0.1) {
//            // F0 défini - utilise Schlick vectoriel
//            return fresnelSchlick(cosI, F0);
//        } else {
//            // Calcul exact du Fresnel pour diélectriques avec dispersion simple
//            // Simulation basique de dispersion chromatique
//            vec3 etaRGB = vec3(1.513, 1.517, 1.528); // Pour verre crown (BK7)
//
//            vec3 fresnelValues;
//            fresnelValues.r = fresnelExact(cosI, etaI, etaRGB.r);
//            fresnelValues.g = fresnelExact(cosI, etaI, etaRGB.g);
//            fresnelValues.b = fresnelExact(cosI, etaI, etaRGB.b);
//
//            return fresnelValues;
//        }
//    }
//}

float fresnelExact(float cosI, float etaI, float etaT) {
    cosI = clamp(cosI, 0.0, 1.0);
    float sinI = sqrt(1.0 - cosI * cosI);
    float sinT2 = (etaI / etaT) * (etaI / etaT) * sinI * sinI;

    if (sinT2 >= 1.0) return 1.0; // Réflexion totale interne

    float cosT = sqrt(1.0 - sinT2);
    float Rs = ((etaI * cosI - etaT * cosT) / (etaI * cosI + etaT * cosT));
    float Rp = ((etaT * cosI - etaI * cosT) / (etaT * cosI + etaI * cosT));

    return 0.5 * (Rs*Rs + Rp*Rp);
}

vec3 computeFresnelConductor(vec3 rayDir, vec3 normal, float etaI, vec3 etaT, vec3 k) {
    vec3 fresnel;
    float cosI = clamp(dot(-normalize(rayDir), normalize(normal)), 0.0, 1.0);

    for (int i = 0; i < 3; i++) {
        // Termes complexes pour les matériaux conducteurs
        float a = (etaT[i]*etaT[i] - k[i]*k[i] - etaI*etaI*(1.0-cosI*cosI));
        float b = sqrt(a*a + 4.0*etaT[i]*etaT[i]*k[i]*k[i]);
        float u = sqrt(0.5*(a + b));
        float v = sqrt(0.5*(-a + b));

        // Coefficients de Fresnel
        float Rs = ((etaI*cosI - u)*(etaI*cosI - u) + v*v) /
        ((etaI*cosI + u)*(etaI*cosI + u) + v*v);
        float Rp = ((etaT[i]*etaT[i] + k[i]*k[i])*cosI*cosI -
        2.0*etaI*u*cosI + etaI*etaI*(u*u + v*v)) /
        ((etaT[i]*etaT[i] + k[i]*k[i])*cosI*cosI +
        2.0*etaI*u*cosI + etaI*etaI*(u*u + v*v));

        fresnel[i] = 0.5 * (Rs + Rp);
    }
    return fresnel;
}

vec3 computeFresnel(vec3 V, vec3 N, float etaI, vec3 etaT, vec3 k, float metallic) {
    float cosTheta = clamp(dot(-normalize(V), normalize(N)), 0.0, 1.0);

    if (metallic > 0.5) {
        // Matériau conducteur
        vec3 fresnel;
        for (int i = 0; i < 3; i++) {
            float n = etaT[i];
            float k_val = k[i];
            float n2 = n * n;
            float k2 = k_val * k_val;
            float cosTheta2 = cosTheta * cosTheta;
            float sinTheta2 = 1.0 - cosTheta2;

            float A = n2 - k2 - etaI * etaI * sinTheta2;
            float B = sqrt(A * A + 4.0 * n2 * k2);
            float U = sqrt(0.5 * (B + A));
            float V = sqrt(0.5 * (B - A));

            float Rs = ((etaI * cosTheta - U) * (etaI * cosTheta - U) + V * V) /
            ((etaI * cosTheta + U) * (etaI * cosTheta + U) + V * V);

            float Rp = ((n2 + k2) * cosTheta2 - 2.0 * etaI * U * cosTheta + etaI * etaI * (U * U + V * V)) /
            ((n2 + k2) * cosTheta2 + 2.0 * etaI * U * cosTheta + etaI * etaI * (U * U + V * V));

            fresnel[i] = 0.5 * (Rs + Rp);
        }
        return fresnel;
    } else {
        // Matériau diélectrique
        float F0 = pow((etaI - etaT.r) / (etaI + etaT.r), 2.0);
        return vec3(F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0));
    }
}
