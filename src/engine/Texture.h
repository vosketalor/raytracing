#pragma once

#include <string>
#include <memory>
#include <vector>
#include "Vector.h"

/**
 * @brief Represents a 2D texture loaded from an image file.
 * Provides access to texture colors using UV coordinates.
 * Supports JPEG, PNG, BMP, TGA, and other common formats via stb_image.
 */
class Texture {
private:
    /** The loaded texture image data (RGBA format). */
    std::vector<std::vector<Vector3>> textureData;

    /** The width of the texture in pixels. */
    int width;

    /** The height of the texture in pixels. */
    int height;

    /** The filename of the texture image. */
    std::string textureFileName;

    /** Flag indicating if the texture was loaded successfully. */
    bool isLoaded;

public:
    /**
     * @brief Constructs a texture by loading an image file.
     * @param textureFileName The name of the texture file to load.
     */
    explicit Texture(const std::string& textureFileName);

    /**
     * @brief Default destructor.
     */
    ~Texture() = default;

    /**
     * @brief Retrieves the color from the texture at the specified UV coordinates.
     * The color is returned as a Vector3 with RGB values normalized in [0, 1].
     *
     * @param uv A Vector2 representing UV coordinates in [0, 1].
     * @return A Vector3 representing the color (R, G, B) normalized to [0, 1].
     */
    Vector3 getTextureColor(const Vector2& uv) const;

    /**
     * @brief Returns the width of the texture in pixels.
     * @return The width of the texture.
     */
    double getWidth() const { return static_cast<double>(width); }

    /**
     * @brief Returns the height of the texture in pixels.
     * @return The height of the texture.
     */
    double getHeight() const { return static_cast<double>(height); }

    /**
     * @brief Returns the filename used to load the texture.
     * @return The texture file name.
     */
    const std::string& getTextureFileName() const { return textureFileName; }

    /**
     * @brief Checks if the texture was loaded successfully.
     * @return True if the texture is loaded, false otherwise.
     */
    bool isTextureLoaded() const { return isLoaded; }

    /**
     * @brief Scales the texture by the given factor.
     * @param scale The scaling factor.
     */
    void scale(double scale);

private:
    /**
     * @brief Loads the texture from file using a simple image loading approach.
     * @param filename The path to the image file.
     * @return True if loading was successful, false otherwise.
     */
    bool loadFromFile(const std::string& filename);
};