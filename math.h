#pragma once
#include <numbers>
#include <cmath>
#include <Windows.h>
#include <iostream>

struct view_matrix_t
{
    float* operator[](int index) const
    {
        return const_cast<float*>(matrix[index]);
    }

    float matrix[4][4];
};

class Vector
{
public:
    float x, y, z;

    constexpr Vector(float x = 0.f, float y = 0.f, float z = 0.f) noexcept : x(x), y(y), z(z) {}

    // Operator Overloads
    constexpr Vector operator-(const Vector& other) const noexcept { return Vector{ x - other.x, y - other.y, z - other.z }; }
    constexpr Vector operator+(const Vector& other) const noexcept { return Vector{ x + other.x, y + other.y, z + other.z }; }
    constexpr Vector operator/(float factor) const noexcept { return Vector{ x / factor, y / factor, z / factor }; }
    constexpr Vector operator*(float factor) const noexcept { return Vector{ x * factor, y * factor, z * factor }; }

    // Distance Calculation
    float Distance(const Vector& other) const {
        return std::sqrt((x - other.x) * (x - other.x) +
            (y - other.y) * (y - other.y) +
            (z - other.z) * (z - other.z));
    }

    // Zero Check
    bool IsZero() const { return x == 0.0f && y == 0.0f && z == 0.0f; }

    // Vector Length Functions
    float Length() const { return std::sqrt(x * x + y * y + z * z); }
    float LengthSqr() const { return (x * x + y * y + z * z); }

    // World to Screen Function
    static bool world_to_screen(const view_matrix_t& vm, Vector& in, Vector& out) {
        out.x = vm[0][0] * in.x + vm[0][1] * in.y + vm[0][2] * in.z + vm[0][3];
        out.y = vm[1][0] * in.x + vm[1][1] * in.y + vm[1][2] * in.z + vm[1][3];

        float width = vm[3][0] * in.x + vm[3][1] * in.y + vm[3][2] * in.z + vm[3][3];

        if (width < 0.01f) return false;

        float inverseWidth = 1.f / width;
        out.x *= inverseWidth;
        out.y *= inverseWidth;

        float x = screenWidth / 2 + 0.5f * out.x * screenWidth + 0.5f;
        float y = screenHeight / 2 - 0.5f * out.y * screenHeight + 0.5f;

        out.x = x;
        out.y = y;

        return true;
    }
};