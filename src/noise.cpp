// Perlin noise implementation by Luke Benstead
// http://blog.kazade.co.uk/2014/05/a-public-domain-c11-1d2d3d-perlin-noise.html
//
// This is free and unencumbered software released into the public domain.
//
// Anyone is free to copy, modify, publish, use, compile, sell, or
// distribute this software, either in source code form or as a compiled
// binary, for any purpose, commercial or non-commercial, and by any
// means.
//
// In jurisdictions that recognize copyright laws, the author or authors
// of this software dedicate any and all copyright interest in the
// software to the public domain. We make this dedication for the benefit
// of the public at large and to the detriment of our heirs and
// successors. We intend this dedication to be an overt act of
// relinquishment in perpetuity of all present and future rights to this
// software under copyright law.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
//
// For more information, please refer to <http://unlicense.org/>

#include <algorithm>
#include <random>
#include <ctime>

#include "noise.hpp"

namespace noise {

double fade(double t) {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

double lerp(double t, double a, double b) {
    return a + t * (b - a);
}

double grad(int hash, double x, double y, double z) {
    int h = hash & 15;
    double u = h < 8 ? x : y;
    double v = h < 4 ? y : h == 12 || h == 14 ? x : z;
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

Perlin::Perlin(uint32_t seed) {
    if(!seed) {
        seed = static_cast<uint32_t>(std::time(nullptr));
    }

    auto mid_range = p.begin() + 256;

    std::mt19937 engine(seed);

    std::iota(p.begin(), mid_range, 0); //Generate sequential numbers in the lower half
    std::shuffle(p.begin(), mid_range, engine); //Shuffle the lower half
    std::copy(p.begin(), mid_range, mid_range); //Copy the lower half to the upper half
    //p now has the numbers 0-255, shuffled, and duplicated
}

double Perlin::noise(double x, double y, double z) const {
    //See here for algorithm: http://cs.nyu.edu/~perlin/noise/

    const int32_t X = static_cast<int32_t>(std::floor(x)) & 255;
    const int32_t Y = static_cast<int32_t>(std::floor(y)) & 255;
    const int32_t Z = static_cast<int32_t>(std::floor(z)) & 255;

    x -= std::floor(x);
    y -= std::floor(y);
    z -= std::floor(z);

    const double u = fade(x);
    const double v = fade(y);
    const double w = fade(z);

    const auto A = p[X] + Y;
    const auto AA = p[A] + Z;
    const auto AB = p[A + 1] + Z;
    const auto B = p[X + 1] + Y;
    const auto BA = p[B] + Z;
    const auto BB = p[B + 1] + Z;

    const auto PAA = p[AA];
    const auto PBA = p[BA];
    const auto PAB = p[AB];
    const auto PBB = p[BB];
    const auto PAA1 = p[AA + 1];
    const auto PBA1 = p[BA + 1];
    const auto PAB1 = p[AB + 1];
    const auto PBB1 = p[BB + 1];

    const auto a = lerp(v,
                        lerp(u, grad(PAA, x, y, z), grad(PBA, x-1, y, z)),
                        lerp(u, grad(PAB, x, y-1, z), grad(PBB, x-1, y-1, z))
    );

    const auto b = lerp(v,
                        lerp(u, grad(PAA1, x, y, z-1), grad(PBA1, x-1, y, z-1)),
                        lerp(u, grad(PAB1, x, y-1, z-1), grad(PBB1, x-1, y-1, z-1))
    );

    return lerp(w, a, b);
}

PerlinOctave::PerlinOctave(int octaves, uint32_t seed):
    perlin_(seed),
    octaves_(octaves) {

}

double PerlinOctave::noise(double x, double y, double z) const {
    double result = 0.0;
    double amp = 1.0;

    int i = octaves_;
    while(i--) {
        result += perlin_.noise(x, y, z) * amp;
        x *= 2.0;
        y *= 2.0;
        z *= 2.0;
        amp *= 0.5;
    }

    return result;
}

}