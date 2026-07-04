#pragma once

struct Vector3
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    Vector3() = default;

    Vector3(float inX, float inY, float inZ)
        : x(inX), y(inY), z(inZ)
    {
    }

    Vector3 operator*(float scale) const
    {
        return Vector3{ x * scale, y * scale, z * scale };
    }

    Vector3& operator+=(const Vector3& other)
    {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }
};
