#pragma once
#include <array>
#include <cmath>
#include <numeric>
#include <stdexcept>
#include <iostream>
#include <immintrin.h>  // Pour les intrinsics AVX
#include <type_traits>
#include <algorithm>

// Alignement pour les opérations SIMD (256 bits = 32 octets)
constexpr size_t SIMD_ALIGNMENT = 32;

template<int k>
class Vector {
    alignas(SIMD_ALIGNMENT) std::array<double, k> data;

public:
    // ============== CONSTRUCTORS ==============
    constexpr Vector() noexcept : data{} {}

    // Constructeur variadique
    template<typename... Args, typename = std::enable_if_t<sizeof...(Args) == k>>
    constexpr Vector(Args... args) noexcept : data{static_cast<double>(args)...} {}

    // Constructeur depuis initializer_list
    Vector(std::initializer_list<double> init) {
        if (init.size() != k) {
            throw std::invalid_argument("Incorrect number of parameters");
        }
        std::copy(init.begin(), init.end(), data.begin());
    }

    // ============== ACCESS OPERATORS ==============
    constexpr double& operator[](size_t index) noexcept { return data[index]; }
    constexpr const double& operator[](size_t index) const noexcept { return data[index]; }

    // ============== VECTOR OPERATIONS ==============
    double dot(const Vector& other) const noexcept {
        if constexpr (k % 4 == 0) {
            return dot_simd(other);
        } else {
            return std::inner_product(data.begin(), data.end(), other.data.begin(), 0.0);
        }
    }

    double norm() const noexcept { return std::sqrt(dot(*this)); }

    Vector normalized() const {
        const double n = norm();
        if (n == 0.0) throw std::runtime_error("Cannot normalize zero vector");
        return (*this) * (1.0 / n);
    }

    // ============== ARITHMETIC OPERATORS ==============
    Vector operator+(const Vector& other) const noexcept {
        Vector result;
        if constexpr (k % 4 == 0) {
            add_simd(other, result);
        } else {
            std::transform(data.begin(), data.end(), other.data.begin(),
                          result.data.begin(), std::plus<double>());
        }
        return result;
    }

    Vector operator-(const Vector& other) const noexcept {
        Vector result;
        std::transform(data.begin(), data.end(), other.data.begin(),
                      result.data.begin(), std::minus<double>());
        return result;
    }

    Vector operator*(double scalar) const noexcept {
        Vector result;
        std::transform(data.begin(), data.end(), result.data.begin(),
                      [scalar](double val) { return val * scalar; });
        return result;
    }

    // ============== CROSS PRODUCT (3D ONLY) ==============
    template<int k2 = k>
    std::enable_if_t<k2 == 3, Vector<3>> cross(const Vector<3>& other) const noexcept {
        return Vector<3>{
            data[1] * other[2] - data[2] * other[1],
            data[2] * other[0] - data[0] * other[2],
            data[0] * other[1] - data[1] * other[0]
        };
    }

    // ============== UTILITIES ==============
    static constexpr int size() noexcept { return k; }
    constexpr auto begin() noexcept { return data.begin(); }
    constexpr auto end() noexcept { return data.end(); }
    constexpr auto begin() const noexcept { return data.begin(); }
    constexpr auto end() const noexcept { return data.end(); }

    // ============== COMPARISON OPERATORS ==============
    bool operator==(const Vector& other) const noexcept {
        return std::equal(data.begin(), data.end(), other.data.begin());
    }

    bool operator!=(const Vector& other) const noexcept {
        return !(*this == other);
    }

private:
    // ============== SIMD IMPLEMENTATIONS ==============
    double dot_simd(const Vector& other) const noexcept {
        __m256d sum = _mm256_setzero_pd();
        for (size_t i = 0; i < k; i += 4) {
            const __m256d a = _mm256_load_pd(&data[i]);
            const __m256d b = _mm256_load_pd(&other.data[i]);
            sum = _mm256_add_pd(sum, _mm256_mul_pd(a, b));
        }
        double result[4];
        _mm256_store_pd(result, sum);
        return result[0] + result[1] + result[2] + result[3];
    }

    void add_simd(const Vector& other, Vector& result) const noexcept {
        for (size_t i = 0; i < k; i += 4) {
            const __m256d a = _mm256_load_pd(&data[i]);
            const __m256d b = _mm256_load_pd(&other.data[i]);
            _mm256_store_pd(&result.data[i], _mm256_add_pd(a, b));
        }
    }
};

// ============== EXTERNAL OPERATORS ==============
template<int k>
Vector<k> operator*(double scalar, const Vector<k>& vec) noexcept {
    return vec * scalar;
}

template<int k>
std::ostream& operator<<(std::ostream& os, const Vector<k>& vec) {
    os << "(";
    for (int i = 0; i < k; ++i) {
        os << vec[i];
        if (i < k - 1) os << ", ";
    }
    os << ")";
    return os;
}

// ============== TYPE ALIASES FOR COMMON VECTORS ==============
using Vector2 = Vector<2>;
using Vector3 = Vector<3>;
using Vector4 = Vector<4>;