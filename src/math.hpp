#pragma once

#include <raymath.h>
#include <cmath>

inline Vector2 operator+(const Vector2& a, const Vector2& b)
{
    return {a.x + b.x, a.y + b.y};
}

inline const Vector2& operator+=(Vector2& a, const Vector2& b)
{
    a.x += b.x;
    a.y += b.y;
    return a;
}

inline Vector2 operator-(const Vector2& a, const Vector2& b)
{
    return {a.x - b.x, a.y - b.y};
}

inline const Vector2& operator-=(Vector2& a, const Vector2& b)
{
    a.x -= b.x;
    a.y -= b.y;
    return a;
}

inline Vector2 operator*(const Vector2& a, float b)
{
    return {a.x * b, a.y * b};
}

inline Vector2 operator/(const Vector2& a, float b)
{
    return {a.x / b, a.y / b};
}

inline Vector3 operator+(const Vector3& a, const Vector3& b)
{
    return {a.x + b.x, a.y + b.y, a.z + b.z};
}

inline Vector3 operator-(const Vector3& a, const Vector3& b)
{
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}

inline Vector3 operator*(const Vector3& a, float b)
{
    return {a.x * b, a.y * b, a.z * b};
}

inline Vector3 operator/(const Vector3& a, float b)
{
    return {a.x / b, a.y / b, a.z / b};
}

struct SplineSegment
{
    Vector3 a;
    Vector3 b;
    Vector3 c;
    Vector3 d;
};

inline SplineSegment catmullRomSpline(Vector3 p0, Vector3 p1, Vector3 p2, Vector3 p3, float tension = 0, float alpha = 0.5)
{
    float t01 = std::pow(Vector3Distance(p0, p1), alpha);
    float t12 = std::pow(Vector3Distance(p1, p2), alpha);
    float t23 = std::pow(Vector3Distance(p2, p3), alpha);

    Vector3 m1 = (p2 - p1 + ((p1 - p0) / t01 - (p2 - p0) / (t01 + t12)) * t12) * (1.0f - tension);
    Vector3 m2 = (p2 - p1 +  ((p3 - p2) / t23 - (p3 - p1) / (t12 + t23)) * t12) * (1.0f - tension);

    SplineSegment segment{};
    segment.a = (p1 - p2) * 2.0f + m1 + m2;
    segment.b = (p1 - p2) * -3.0f - m1 - m1 - m2;
    segment.c = m1;
    segment.d = p1;
    return segment;
}