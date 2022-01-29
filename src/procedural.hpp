#pragma once

#include <raylib.h>
#include "random.hpp"

// Wave parameters type (96 bytes)
struct WaveParams
{
    // Random seed used to generate the wave
    int randSeed{0};

    // Wave type (square, sawtooth, sine, noise)
    int waveTypeValue{0};

    // Wave envelope parameters
    float attackTimeValue{0};
    float sustainTimeValue{0.3f};
    float sustainPunchValue{0};
    float decayTimeValue{0.4f};

    // Frequency parameters
    float startFrequencyValue{0.3f};
    float minFrequencyValue{0};
    float slideValue{0};
    float deltaSlideValue{0};
    float vibratoDepthValue{0};
    float vibratoSpeedValue{0};
    // float vibratoPhaseDelayValue;

    // Tone change parameters
    float changeAmountValue{0};
    float changeSpeedValue{0};

    // Square wave parameters
    float squareDutyValue{0};
    float dutySweepValue{0};

    // Repeat parameters
    float repeatSpeedValue{0};

    // Phaser parameters
    float phaserOffsetValue{0};
    float phaserSweepValue{0};

    // Filter parameters
    float lpfCutoffValue{1.0f};
    float lpfCutoffSweepValue{0};
    float lpfResonanceValue{0};
    float hpfCutoffValue{0};
    float hpfCutoffSweepValue{0};
};

struct SpaceshipParams
{
    int randSeed{0};
    int generatedWidth{8};
    int generatedHeight{8};
    bool mirrorHorizontal{true};
    bool mirrorVertical{false};
    bool filter1Pixel{true};
    bool usePlayerColors{false};
};


enum class NoiseType { RANDOM, PICKUP_COIN, LASER_SHOOT, EXPLOSION, POWER_UP, HIT_HURT, JUMP, BLIP_SELECT };
enum class BulletType { PlayerLaser, PlayerPlasma, AlienLaser, AlienBomb, AlienMissile };

class Procedural
{
public:
    static Procedural* instance();

    static Wave generateWave(const WaveParams& params);
    static WaveParams generateRandomSet(NoiseType type);

    static Image generateSpaceship(const SpaceshipParams& params, int* numColorsUsed = nullptr);
    static SpaceshipParams randomAlienSpaceshipParams(int minColors, int seed = 0);

    static Image generateBullet(BulletType type, int seed = 0);

private:
    Procedural() = default;
    ~Procedural() = default;
};