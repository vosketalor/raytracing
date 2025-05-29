#pragma once
#include <memory>
#include "Vector.h"

class Shape; // Forward declaration

/**
 * @brief Represents the result of an intersection between a Ray and a Shape
 */
class Intersection {
public:
    double lambda;      ///< Distance from ray origin (-1 means no intersection)
    Vector3 normal;     ///< Surface normal at intersection point
    const Shape* shape; ///< Intersected shape
    
    /**
     * @brief Constructs a valid intersection
     * @param lambda Distance from ray origin
     * @param normal Surface normal
     * @param shape Intersected shape
     */
    Intersection(const double lambda, const Vector3& normal, const Shape* shape)
    : lambda(lambda), normal(normal), shape(shape) {}
    
    /**
     * @brief Constructs an invalid intersection (no hit)
     */
    Intersection() : lambda(-1), shape(nullptr) {}
    
    /**
     * @brief Checks if the intersection is valid
     * @return true if the intersection is valid
     */
    explicit operator bool() const { return lambda >= 0; }
    
    /**
     * @brief Comparison operator
     */
    bool operator==(const Intersection& other) const {
        return lambda == other.lambda && 
               normal == other.normal && 
               shape == other.shape;
    }
    
    /**
     * @brief Inequality operator
     */
    bool operator!=(const Intersection& other) const {
        return !(*this == other);
    }
};