#include <iostream>
#include <fstream>
#include <chrono>
#include <memory>
#include "engine/Renderer.h"
#include "scenes/Scene1.h"
#include <algorithm> // Pour std::clamp
#include <iomanip>   // Pour std::setprecision

const Vector3 SKYCOLOR = {135.0/255, 206.0/255, 235.0/255};

int main(int argc, char *argv[]) {
    // Paramètres par défaut
    int width = 1024/1;
    int height = 768/1;
    float d = 1.0f;
    Vector3 obs = {0, 0, 0};
    std::string filename = "output.ppm";

    // Surcharge des paramètres si fournis en ligne de commande
    if (argc >= 3) {
        width = std::atoi(argv[1]);
        height = std::atoi(argv[2]);
        if (argc >= 4) {
            filename = argv[3];
        }
    }

    auto startTime = std::chrono::high_resolution_clock::now();

    // Création de la scène
    auto scene = std::make_unique<Scene1>();
    scene->setSkyColor(SKYCOLOR);
    scene->setAmbient({0.1f, 0.1f, 0.1f});
    scene->createLights();
    scene->createShapes();

    // Initialisation du rendu
    Renderer renderer(scene.get());
    std::vector<Vector3> frameBuffer(width * height);

    std::cout << "Resolution: " << width << "x" << height << std::endl;

    // Rendu de la scène
    renderer.render(width, height, frameBuffer);

    // Sauvegarde au format PPM
    std::ofstream image(filename);
    if (!image.is_open()) {
        std::cerr << "Error while opening file: " << filename << std::endl;
        return 1;
    }

    // En-tête PPM
    image << "P3\n" << width << " " << height << "\n255\n";

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            Vector3& color = frameBuffer[y * width + x];
            color.clamp(0.0, 1.0);

            // Clamping et conversion en entier [0-255]
            int r = static_cast<int>(color[0] * 255);
            int g = static_cast<int>(color[1] * 255);
            int b = static_cast<int>(color[2] * 255);

            // Écriture formatée
            image << r << ' ' << g << ' ' << b << '\n';
        }
    }

    image.close();

    // Calcul du temps d'exécution
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count() / 1000.0;

    std::cout << "Image generated: " << filename << std::endl;
    std::cout << "Rendering time: " << duration << " secs" << std::endl;

    return 0;
}