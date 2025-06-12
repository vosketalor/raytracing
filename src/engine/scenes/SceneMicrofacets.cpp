// // SceneMicrofacets.cpp - Exemple d'utilisation des microfacettes
//
// #include "SceneMicrofacets.h"
// #include "shapes/Plane.h"
// #include "shapes/Sphere.h"
//
// void SceneMicrofacets::createShapes()
// {
//     // Plan du sol avec une légère rugosité
//     auto plane = std::make_shared<Plane>(Vector3{0, -1, 0}, Vector3{0, 1, 0});
//     plane->setColor(Vector3{0.8f, 0.8f, 0.8f});
//     plane->setMaterial(Material(0.1, 0.0, 1000, Scene::ETA_AIR, 0.3, 0.0));
//     addShape(plane);
//
//     // Sphère métallique dorée (conducteur)
//     auto sphereGold = std::make_shared<Sphere>(Vector3{-2, -0.5f, -4}, 0.5f);
//     sphereGold->setColor(Vector3{1.0f, 0.86f, 0.57f}); // Couleur or
//     sphereGold->setMaterial(Material::MetalGold);
//     addShape(sphereGold);
//
//     // Sphère métallique cuivre avec plus de rugosité
//     auto sphereCopper = std::make_shared<Sphere>(Vector3{0, -0.5f, -4}, 0.5f);
//     sphereCopper->setColor(Vector3{0.95f, 0.64f, 0.54f}); // Couleur cuivre
//     sphereCopper->setMaterial(Material::MetalCopper);
//     addShape(sphereCopper);
//
//     // Sphère de verre avec microfacettes (diélectrique lisse)
//     auto sphereGlass = std::make_shared<Sphere>(Vector3{2, -0.5f, -4}, 0.5f);
//     sphereGlass->setColor(Vector3{0.95f, 0.95f, 0.95f});
//     sphereGlass->setMaterial(Material::Glass);
//     addShape(sphereGlass);
//
//     // Sphère plastique rugueuse (diélectrique rugueux)
//     auto spherePlastic = std::make_shared<Sphere>(Vector3{-1, 0.5f, -6}, 0.5f);
//     spherePlastic->setColor(Vector3{0.8f, 0.2f, 0.2f}); // Rouge plastique
//     spherePlastic->setMaterial(Material::PlasticRough);
//     addShape(spherePlastic);
//
//     // Sphère métal poli (faible rugosité)
//     auto spherePolished = std::make_shared<Sphere>(Vector3{1, 0.5f, -6}, 0.5f);
//     spherePolished->setColor(Vector3{0.9f, 0.9f, 0.9f});
//     spherePolished->setMaterial(Material(0.9, 0.0, 1000, 1.0, 0.01, 1.0, Vector3(0.9, 0.9, 0.9)));
//     addShape(spherePolished);
//
//     // Démonstration de différents niveaux de rugosité
//     for (int i = 0; i < 5; ++i) {
//         auto sphere = std::make_shared<Sphere>(
//             Vector3{-2.5f + i * 1.25f, 1.5f, -8}, 0.3f
//         );
//         sphere->setColor(Vector3{0.7f, 0.7f, 0.9f});
//
//         double roughness = i * 0.25; // 0.0, 0.25, 0.5, 0.75, 1.0
//         sphere->setMaterial(Material(0.8, 0.0, 1000, 1.0, roughness, 1.0, Vector3(0.8, 0.8, 0.9)));
//         addShape(sphere);
//     }
//
//     // Sphère de référence diffuse pure
//     auto sphereDiffuse = std::make_shared<Sphere>(Vector3{0, -0.5f, -2}, 0.3f);
//     sphereDiffuse->setColor(Vector3{0.2f, 0.8f, 0.2f});
//     sphereDiffuse->setMaterial(Material(0.0, 0.0, 100, Scene::ETA_AIR, 1.0, 0.0)); // Parfaitement diffus
//     addShape(sphereDiffuse);
// }
//
// void SceneMicrofacets::createLights()
// {
//     // Lumière principale (area light)
//     Vector3 position{0, 6, 2};
//     Vector3 uDir{1, 0, 0};
//     Vector3 vDir{0, 0, 1};
//     double width = 4.0;
//     double height = 4.0;
//     Vector3 colorDiffuse{1.0, 1.0, 1.0};
//     Vector3 colorSpecular{1.0, 1.0, 1.0};
//
//     auto mainLight = std::make_shared<LightSource>(
//         position, uDir, vDir, width, height, colorDiffuse, colorSpecular
//     );
//     mainLight->setIntensity(1.5);
//     addLightSource(mainLight);
//
//     // Lumière d'appoint plus douce
//     Vector3 position2{-4, 3, -2};
//     Vector3 uDir2{0, 1, 0};
//     Vector3 vDir2{0, 0, 1};
//     Vector3 colorWarm{1.0, 0.9, 0.8}; // Légèrement chaude
//
//     auto fillLight = std::make_shared<LightSource>(
//         position2, uDir2, vDir2, 2.0, 2.0, colorWarm, colorWarm
//     );
//     fillLight->setIntensity(0.8);
//     addLightSource(fillLight);
// }
