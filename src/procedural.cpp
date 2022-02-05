
#include "procedural.hpp"
#include "scene.hpp"

#include <cstdlib>
#include <cstring>
#include <vector>
#include <set>

Procedural* Procedural::instance()
{
    static Procedural _instance;
    return &_instance;
}

#define MAX_WAVE_LENGTH_SECONDS 10  // Max length for wave: 10 seconds
#define WAVE_SAMPLE_RATE 44100      // Default sample rate

#define frnd(range) ((float)randomValue(0, 10000) / 10000.0f * range)

//-----------------------------------------------------------------------------
// BEGIN Random Generation
//-----------------------------------------------------------------------------
// Based on splitmix64 public domain code by Sebastiano Vigna (vigna@acm.org), 2015
// and xoroshiro128++ 1.0 public domain code by David Blackman and Sebastiano Vigna (vigna@acm.org), 2019
//-----------------------------------------------------------------------------

static uint64_t s[2];

static inline uint64_t rotl(const uint64_t x, int k)
{
    return (x << k) | (x >> (64 - k));
}

static uint64_t splitmix64(uint64_t* seed)
{
    uint64_t z = (*seed += UINT64_C(0x9E3779B97F4A7C15));
    z = (z ^ (z >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
    z = (z ^ (z >> 27)) * UINT64_C(0x94D049BB133111EB);
    return z ^ (z >> 31);
}

static void seedRandom(uint64_t seed)
{
    s[0] = splitmix64(&seed);
    s[1] = splitmix64(&seed);
}

static uint64_t nextRandom()
{
    const uint64_t s0 = s[0];
    uint64_t s1 = s[1];
    const uint64_t result = rotl(s0 + s1, 17) + s0;

    s1 ^= s0;
    s[0] = rotl(s0, 49) ^ s1 ^ (s1 << 21);  // a, b
    s[1] = rotl(s1, 28);                    // c

    return result >> 1;  // ensure positive values if used in signed context
}

static int randomValue(int min, int max)
{
    if (min > max) {
        int tmp = max;
        max = min;
        min = tmp;
    }
    return int((int64_t)nextRandom() % (abs(max - min) + 1) + min);
}

//-----------------------------------------------------------------------------
// BEGIN Alien Generation
//-----------------------------------------------------------------------------
// Original implementation by Steffen "Gulrak" Schümann, based on ideas from:
//   * Zelimir Fedoran - https://github.com/zfedoran/pixel-sprite-generator
//   * David Bollinger - http://web.archive.org/web/20080228054410/http://www.davebollinger.com/works/pixelspaceships/
//-----------------------------------------------------------------------------

class PixelGenerator
{
public:
    PixelGenerator(int width, int height)
        : _width(width)
        , _height(height)
        , _data(width * height, 0)
    {
        for (int y = 0; y < _height; ++y) {
            for (int x = 0; x < _width; ++x) {
                if (x == _width - 1 && (y > 1 && y < _height - 2)) {
                    at(x, y) = -1;
                }
                else if (y && x + y / 2 + 3 > _width && x > 0 && y < _height - 1) {
                    at(x, y) = 1;
                }
                if (y >= _height / 2 && y < _height - 3 && x >= _width - 2) {
                    at(x, y) = 2;
                }
            }
        }
    }

    void generate(Random& rnd)
    {
        for (int y = 0; y < _height; ++y) {
            for (int x = 0; x < _width; ++x) {
                auto& c = at(x, y);
                if (c == 1) {
                    c = (rnd.randomUFloat() >= 0.5f ? 0 : c);
                }
                else if (c == 2) {
                    if (rnd.randomUFloat() >= 0.5f) {
                        c = 1;
                    }
                    else {
                        c = -1;
                    }
                }
            }
        }
    }

    void contour()
    {
        for (int y = 0; y < _height; ++y) {
            for (int x = 0; x < _width; ++x) {
                if (at(x, y) > 0) {
                    if (y - 1 >= 0 && at(x, y-1) == 0) {
                        at(x, y-1) = -1;
                    }
                    if (y + 1 < _height && at(x, y+1) == 0) {
                        at(x, y+1) = -1;
                    }
                    if (x - 1 >= 0 && at(x-1, y) == 0) {
                        at(x-1, y) = -1;
                    }
                    if (x + 1 < _width && at(x+1, y) == 0) {
                        at(x+1, y) = -1;
                    }
                }
            }
        }
    }

    void colorize(Random& rnd)
    {
        for (int y = 0; y < _height; ++y) {
            for (int x = 0; x < _width; ++x) {
                if (at(x, y) == 1) {
                    auto cidx = rnd.randomInt(2,5);
                    floodFill(x, y, cidx);
                }
            }
        }
    }

    void floodFill(int x, int y, int8_t newVal, int8_t oldVal = 0x7f)
    {
        if(x < 0 || x >= _width || y < 0 || y >= _height) {
            return;
        }
        auto& c = at(x,y);
        if(oldVal == 0x7f) {
            oldVal = c;
        }
        else if(c != oldVal) {
            return;
        }
        c = newVal;
        floodFill(x+1, y, newVal, oldVal);
        floodFill(x-1, y, newVal, oldVal);
        floodFill(x, y+1, newVal, oldVal);
        floodFill(x, y-1, newVal, oldVal);
    }

    int8_t& at(int x, int y)
    {
        return _data[y * _width + x];
    }

private:
    int _width;
    int _height;
    std::vector<int8_t> _data;
};

Image Procedural::generateSpaceship(const SpaceshipParams& params, int* numColorsUsed)
{
    Random rnd{params.randSeed ? params.randSeed : randomValue(1, 0xFFFE)};
    PixelGenerator pg(params.generatedWidth, params.generatedHeight);
    std::set<int> colorsUsed;
    pg.generate(rnd);
    pg.contour();
    pg.colorize(rnd);
    auto imageWidth = params.mirrorHorizontal ? params.generatedWidth * 2 : params.generatedWidth;
    auto imageHeight = params.mirrorVertical ? params.generatedHeight * 2 : params.generatedHeight;
    auto triple1 = rnd.randomInt(1, 8);
    auto triple2 = rnd.randomInt(1, 8);
    std::vector<Color> colors{};
    if(params.usePlayerColors) {
        colors = { Scene::BasePalette[1], Scene::BasePalette[2], Scene::BasePalette[3], Scene::BasePalette[13], Scene::BasePalette[14] };
    }
    else
    {
        colors = { Scene::getTripleColor(triple1, 2), Scene::getTripleColor(triple1, 1), Scene::getTripleColor(triple1, 0), Scene::getTripleColor(triple2, 0), Scene::getTripleColor(triple2, 1) };
    }
    Image img = GenImageColor(imageWidth, imageHeight, {0, 0, 0, 0});

    for (int y = 0; y < params.generatedHeight; ++y) {
        for (int x = 0; x < params.generatedWidth; ++x) {
            auto c = pg.at(x, y);
            if (c) {
                Color col{};
                switch (c) {
                    case -1:
                        col = colors[0];
                        break;
                    case 3:
                        col = colors[1];
                        break;
                    case 4:
                        col = colors[3];
                        break;
                    case 5:
                        col = colors[4];
                        break;
                    default:
                        col = colors[2];
                        break;
                }
                colorsUsed.insert(ColorToInt(col));
                ImageDrawPixel(&img, x, y, col);
                if(params.mirrorHorizontal) {
                    ImageDrawPixel(&img, imageWidth - x - 1, y, col);
                }
                if(params.mirrorVertical) {
                    ImageDrawPixel(&img, x, imageHeight - y - 1, col);
                }
                if(params.mirrorHorizontal && params.mirrorVertical) {
                    ImageDrawPixel(&img, imageWidth - x - 1, imageHeight - y - 1, col);
                }
            }
        }
    }
    if(numColorsUsed) {
        *numColorsUsed = colorsUsed.size();
    }
    return img;
}

SpaceshipParams Procedural::randomAlienSpaceshipParams(int minColors, int seed)
{
    SpaceshipParams params{};
    size_t count = 0;
    int numColors = 0;
    do {
        if(!seed) {
            seed = Random::instance()->randomInt(1, 0xfffe);
        }
        params = SpaceshipParams{seed, 6, 12, true, false, true, false};
        auto alien = generateSpaceship(params, &numColors);
        UnloadImage(alien);
        seed = 0;
    }
    while(numColors < minColors && ++count < 10);
    return params;
}

Image Procedural::generateBullet(BulletType type, int seed)
{
    Random rnd{seed ? seed : GetRandomValue(1,0xfffe)};
    Image result{};
    switch(type) {
        case BulletType::AlienLaser:
        case BulletType::PlayerLaser: {
            result = GenImageColor(8, 8, {0,0,0,0});
            float brightness = 1.0f;
            for(int y = 0; y < 8; ++y, brightness *= 0.9f) {
                auto l = type == BulletType::PlayerLaser ? y : 7 - y;
                if(!y) {
                    ImageDrawPixel(&result, 2, l, {255,255,255,40});
                    ImageDrawPixel(&result, 3, l, {255,255,255,60});
                    ImageDrawPixel(&result, 4, l, {255,255,255,60});
                    ImageDrawPixel(&result, 5, l, {255,255,255,30});
                }
                else if(y == 7) {
                    ImageDrawPixel(&result, 2, l, {255,255,255,10});
                    if(type == BulletType::PlayerLaser) {
                        ImageDrawPixel(&result, 3, l, {255, 255, 255, 20});
                        ImageDrawPixel(&result, 4, l, {255, 255, 255, 20});
                    }
                    ImageDrawPixel(&result, 5, l, {255,255,255,10});
                }
                else {
                    auto c = static_cast<uint8_t>(255 * brightness);
                    ImageDrawPixel(&result, 2, l, {c, c, c, 80});
                    ImageDrawPixel(&result, 3, l, {c, c, c, 255});
                    ImageDrawPixel(&result, 4, l, {c, c, c, 255});
                    ImageDrawPixel(&result, 5, l, {c, c, c, 80});
                }
            }
            break;
        }
        default:
            break;
    }
    return result;
}

//-----------------------------------------------------------------------------
// END Alien Generation
//-----------------------------------------------------------------------------


namespace {
//-----------------------------------------------------------------------------
// BEGIN Noise Generation
//-----------------------------------------------------------------------------
// Based on rFXGen v2.5
// LICENSE: zlib/libpng
//
// Copyright (c) 2014-2022 raylib technologies (@raylibtech).
//
// This software is provided "as-is", without any express or implied warranty. In no event
// will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose, including commercial
// applications, and to alter it and redistribute it freely, subject to the following restrictions:
//
//   1. The origin of this software must not be misrepresented; you must not claim that you
//   wrote the original software. If you use this software in a product, an acknowledgment
//   in the product documentation would be appreciated but is not required.
//
//   2. Altered source versions must be plainly marked as such, and must not be misrepresented
//   as being the original software.
//
//   3. This notice may not be removed or altered from any source distribution.
//-----------------------------------------------------------------------------

// Reset wave parameters
static void ResetWaveParams(WaveParams* params)
{
    // NOTE: Random seed is set to a random value
    params->randSeed = randomValue(0x1, 0xFFFE);
    seedRandom(params->randSeed);

    // Wave type
    params->waveTypeValue = 0;

    // Wave envelope params
    params->attackTimeValue = 0.0f;
    params->sustainTimeValue = 0.3f;
    params->sustainPunchValue = 0.0f;
    params->decayTimeValue = 0.4f;

    // Frequency params
    params->startFrequencyValue = 0.3f;
    params->minFrequencyValue = 0.0f;
    params->slideValue = 0.0f;
    params->deltaSlideValue = 0.0f;
    params->vibratoDepthValue = 0.0f;
    params->vibratoSpeedValue = 0.0f;
    // params->vibratoPhaseDelay = 0.0f;

    // Tone change params
    params->changeAmountValue = 0.0f;
    params->changeSpeedValue = 0.0f;

    // Square wave params
    params->squareDutyValue = 0.0f;
    params->dutySweepValue = 0.0f;

    // Repeat params
    params->repeatSpeedValue = 0.0f;

    // Phaser params
    params->phaserOffsetValue = 0.0f;
    params->phaserSweepValue = 0.0f;

    // Filter params
    params->lpfCutoffValue = 1.0f;
    params->lpfCutoffSweepValue = 0.0f;
    params->lpfResonanceValue = 0.0f;
    params->hpfCutoffValue = 0.0f;
    params->hpfCutoffSweepValue = 0.0f;
}

#define MAX_WAVE_LENGTH_SECONDS 10  // Max length for wave: 10 seconds
#define WAVE_SAMPLE_RATE 44100      // Default sample rate

#define rnd(n) (nextRandom() % (n + 1))
#define GetRandomFloat(range) ((float)rnd(10000) / 10000 * range)

// Generates new wave from wave parameters
// NOTE: By default wave is generated as 44100Hz, 32bit float, mono
static Wave GenerateWave(WaveParams params)
{
    if (params.randSeed != 0)
        seedRandom(params.randSeed);  // Initialize seed if required

    // Configuration parameters for generation
    // NOTE: Those parameters are calculated from selected values
    int phase = 0;
    double fperiod = 0.0;
    double fmaxperiod = 0.0;
    double fslide = 0.0;
    double fdslide = 0.0;
    int period = 0;
    float squareDuty = 0.0f;
    float squareSlide = 0.0f;
    int envelopeStage = 0;
    int envelopeTime = 0;
    int envelopeLength[3] = {0};
    float envelopeVolume = 0.0f;
    float fphase = 0.0f;
    float fdphase = 0.0f;
    int iphase = 0;
    float phaserBuffer[1024] = {0};
    int ipp = 0;
    float noiseBuffer[32] = {0};  // Required for noise wave, depends on random seed!
    float fltp = 0.0f;
    float fltdp = 0.0f;
    float fltw = 0.0f;
    float fltwd = 0.0f;
    float fltdmp = 0.0f;
    float fltphp = 0.0f;
    float flthp = 0.0f;
    float flthpd = 0.0f;
    float vibratoPhase = 0.0f;
    float vibratoSpeed = 0.0f;
    float vibratoAmplitude = 0.0f;
    int repeatTime = 0;
    int repeatLimit = 0;
    int arpeggioTime = 0;
    int arpeggioLimit = 0;
    double arpeggioModulation = 0.0;

    // HACK: Security check to avoid crash (why?)
    if (params.minFrequencyValue > params.startFrequencyValue)
        params.minFrequencyValue = params.startFrequencyValue;
    if (params.slideValue < params.deltaSlideValue)
        params.slideValue = params.deltaSlideValue;

    // Reset sample parameters
    //----------------------------------------------------------------------------------------
    fperiod = 100.0 / (params.startFrequencyValue * params.startFrequencyValue + 0.001);
    period = (int)fperiod;
    fmaxperiod = 100.0 / (params.minFrequencyValue * params.minFrequencyValue + 0.001);
    fslide = 1.0 - pow((double)params.slideValue, 3.0) * 0.01;
    fdslide = -pow((double)params.deltaSlideValue, 3.0) * 0.000001;
    squareDuty = 0.5f - params.squareDutyValue * 0.5f;
    squareSlide = -params.dutySweepValue * 0.00005f;

    if (params.changeAmountValue >= 0.0f)
        arpeggioModulation = 1.0 - pow((double)params.changeAmountValue, 2.0) * 0.9;
    else
        arpeggioModulation = 1.0 + pow((double)params.changeAmountValue, 2.0) * 10.0;

    arpeggioLimit = (int)(powf(1.0f - params.changeSpeedValue, 2.0f) * 20000 + 32);

    if (params.changeSpeedValue == 1.0f)
        arpeggioLimit = 0;  // WATCH OUT: float comparison

    // Reset filter parameters
    fltw = powf(params.lpfCutoffValue, 3.0f) * 0.1f;
    fltwd = 1.0f + params.lpfCutoffSweepValue * 0.0001f;
    fltdmp = 5.0f / (1.0f + powf(params.lpfResonanceValue, 2.0f) * 20.0f) * (0.01f + fltw);
    if (fltdmp > 0.8f)
        fltdmp = 0.8f;
    flthp = powf(params.hpfCutoffValue, 2.0f) * 0.1f;
    flthpd = 1.0f + params.hpfCutoffSweepValue * 0.0003f;

    // Reset vibrato
    vibratoSpeed = powf(params.vibratoSpeedValue, 2.0f) * 0.01f;
    vibratoAmplitude = params.vibratoDepthValue * 0.5f;

    // Reset envelope
    envelopeLength[0] = (int)(params.attackTimeValue * params.attackTimeValue * 100000.0f);
    envelopeLength[1] = (int)(params.sustainTimeValue * params.sustainTimeValue * 100000.0f);
    envelopeLength[2] = (int)(params.decayTimeValue * params.decayTimeValue * 100000.0f);

    fphase = powf(params.phaserOffsetValue, 2.0f) * 1020.0f;
    if (params.phaserOffsetValue < 0.0f)
        fphase = -fphase;

    fdphase = powf(params.phaserSweepValue, 2.0f) * 1.0f;
    if (params.phaserSweepValue < 0.0f)
        fdphase = -fdphase;

    iphase = abs((int)fphase);

    for (int i = 0; i < 32; i++)
        noiseBuffer[i] = GetRandomFloat(2.0f) - 1.0f;  // WATCH OUT: GetRandomFloat()

    repeatLimit = (int)(powf(1.0f - params.repeatSpeedValue, 2.0f) * 20000 + 32);

    if (params.repeatSpeedValue == 0.0f)
        repeatLimit = 0;
    //----------------------------------------------------------------------------------------

    // NOTE: We reserve enough space for up to 10 seconds of wave audio at given sample rate
    // By default we use float size samples, they are converted to desired sample size at the end
    float* buffer = (float*)calloc(MAX_WAVE_LENGTH_SECONDS * WAVE_SAMPLE_RATE, sizeof(float));
    bool generatingSample = true;
    int sampleCount = 0;

    for (int i = 0; i < MAX_WAVE_LENGTH_SECONDS * WAVE_SAMPLE_RATE; i++) {
        if (!generatingSample) {
            sampleCount = i;
            break;
        }

        // Generate sample using selected parameters
        //------------------------------------------------------------------------------------
        repeatTime++;

        if ((repeatLimit != 0) && (repeatTime >= repeatLimit)) {
            // Reset sample parameters (only some of them)
            repeatTime = 0;

            fperiod = 100.0 / (params.startFrequencyValue * params.startFrequencyValue + 0.001);
            period = (int)fperiod;
            fmaxperiod = 100.0 / (params.minFrequencyValue * params.minFrequencyValue + 0.001);
            fslide = 1.0 - pow((double)params.slideValue, 3.0) * 0.01;
            fdslide = -pow((double)params.deltaSlideValue, 3.0) * 0.000001;
            squareDuty = 0.5f - params.squareDutyValue * 0.5f;
            squareSlide = -params.dutySweepValue * 0.00005f;

            if (params.changeAmountValue >= 0.0f)
                arpeggioModulation = 1.0 - pow((double)params.changeAmountValue, 2.0) * 0.9;
            else
                arpeggioModulation = 1.0 + pow((double)params.changeAmountValue, 2.0) * 10.0;

            arpeggioTime = 0;
            arpeggioLimit = (int)(powf(1.0f - params.changeSpeedValue, 2.0f) * 20000 + 32);

            if (params.changeSpeedValue == 1.0f)
                arpeggioLimit = 0;  // WATCH OUT: float comparison
        }

        // Frequency envelopes/arpeggios
        arpeggioTime++;

        if ((arpeggioLimit != 0) && (arpeggioTime >= arpeggioLimit)) {
            arpeggioLimit = 0;
            fperiod *= arpeggioModulation;
        }

        fslide += fdslide;
        fperiod *= fslide;

        if (fperiod > fmaxperiod) {
            fperiod = fmaxperiod;

            if (params.minFrequencyValue > 0.0f)
                generatingSample = false;
        }

        float rfperiod = (float)fperiod;

        if (vibratoAmplitude > 0.0f) {
            vibratoPhase += vibratoSpeed;
            rfperiod = (float)(fperiod * (1.0 + sinf(vibratoPhase) * vibratoAmplitude));
        }

        period = (int)rfperiod;

        if (period < 8)
            period = 8;

        squareDuty += squareSlide;

        if (squareDuty < 0.0f)
            squareDuty = 0.0f;
        if (squareDuty > 0.5f)
            squareDuty = 0.5f;

        // Volume envelope
        envelopeTime++;

        if (envelopeTime > envelopeLength[envelopeStage]) {
            envelopeTime = 0;
            envelopeStage++;

            if (envelopeStage == 3)
                generatingSample = false;
        }

        if (envelopeStage == 0)
            envelopeVolume = (float)envelopeTime / (float)envelopeLength[0];
        if (envelopeStage == 1)
            envelopeVolume = 1.0f + powf(1.0f - (float)envelopeTime / (float)envelopeLength[1], 1.0f) * 2.0f * params.sustainPunchValue;
        if (envelopeStage == 2)
            envelopeVolume = 1.0f - (float)envelopeTime / (float)envelopeLength[2];

        // Phaser step
        fphase += fdphase;
        iphase = abs((int)fphase);

        if (iphase > 1023)
            iphase = 1023;

        if (flthpd != 0.0f)  // WATCH OUT!
        {
            flthp *= flthpd;
            if (flthp < 0.00001f)
                flthp = 0.00001f;
            if (flthp > 0.1f)
                flthp = 0.1f;
        }

        float ssample = 0.0f;

#define MAX_SUPERSAMPLING 8

        // Supersampling x8
        for (int si = 0; si < MAX_SUPERSAMPLING; si++) {
            float sample = 0.0f;
            phase++;

            if (phase >= period) {
                // phase = 0;
                phase %= period;

                if (params.waveTypeValue == 3) {
                    for (int ni = 0; ni < 32; ++ni)
                        noiseBuffer[ni] = GetRandomFloat(2.0f) - 1.0f;  // WATCH OUT: GetRandomFloat()
                }
            }

            // base waveform
            float fp = (float)phase / (float)period;

            switch (params.waveTypeValue) {
                case 0:  // Square wave
                {
                    if (fp < squareDuty)
                        sample = 0.5f;
                    else
                        sample = -0.5f;

                } break;
                case 1:
                    sample = 1.0f - fp * 2;
                    break;  // Sawtooth wave
                case 2:
                    sample = sinf(fp * 2 * PI);
                    break;  // Sine wave
                case 3:
                    sample = noiseBuffer[phase * 32 / period];
                    break;  // Noise wave
                default:
                    break;
            }

            // LP filter
            float pp = fltp;
            fltw *= fltwd;

            if (fltw < 0.0f)
                fltw = 0.0f;
            if (fltw > 0.1f)
                fltw = 0.1f;

            if (params.lpfCutoffValue != 1.0f)  // WATCH OUT!
            {
                fltdp += (sample - fltp) * fltw;
                fltdp -= fltdp * fltdmp;
            }
            else {
                fltp = sample;
                fltdp = 0.0f;
            }

            fltp += fltdp;

            // HP filter
            fltphp += fltp - pp;
            fltphp -= fltphp * flthp;
            sample = fltphp;

            // Phaser
            phaserBuffer[ipp & 1023] = sample;
            sample += phaserBuffer[(ipp - iphase + 1024) & 1023];
            ipp = (ipp + 1) & 1023;

            // Final accumulation and envelope application
            ssample += sample * envelopeVolume;
        }

#define SAMPLE_SCALE_COEFICIENT 0.2f  // NOTE: Used to scale sample value to [-1..1]

        ssample = (ssample / MAX_SUPERSAMPLING) * SAMPLE_SCALE_COEFICIENT;
        //------------------------------------------------------------------------------------

        // Accumulate samples in the buffer
        if (ssample > 1.0f)
            ssample = 1.0f;
        if (ssample < -1.0f)
            ssample = -1.0f;

        buffer[i] = ssample;
    }

    Wave genWave = {0};
    genWave.frameCount = sampleCount / 1;   // Number of samples / channels
    genWave.sampleRate = WAVE_SAMPLE_RATE;  // By default 44100 Hz
    genWave.sampleSize = 32;                // By default 32 bit float samples
    genWave.channels = 1;                   // By default 1 channel (mono)

    // NOTE: Wave can be converted to desired format after generation

    genWave.data = (float*)calloc(genWave.frameCount * genWave.channels, sizeof(float));
    memcpy(genWave.data, buffer, genWave.frameCount * genWave.channels * sizeof(float));

    free(buffer);

    return genWave;
}

//--------------------------------------------------------------------------------------------
// Sound generation functions
//--------------------------------------------------------------------------------------------

}

// Generate sound: Pickup/Coin
static WaveParams GenPickupCoin(void)
{
    WaveParams params = {0};
    ResetWaveParams(&params);

    params.startFrequencyValue = 0.4f + frnd(0.5f);
    params.attackTimeValue = 0.0f;
    params.sustainTimeValue = frnd(0.1f);
    params.decayTimeValue = 0.1f + frnd(0.4f);
    params.sustainPunchValue = 0.3f + frnd(0.3f);

    if (randomValue(0, 1)) {
        params.changeSpeedValue = 0.5f + frnd(0.2f);
        params.changeAmountValue = 0.2f + frnd(0.4f);
    }

    return params;
}

// Generate sound: Laser shoot
static WaveParams GenLaserShoot(void)
{
    WaveParams params = {0};
    ResetWaveParams(&params);

    params.waveTypeValue = randomValue(0, 2);

    if ((params.waveTypeValue == 2) && randomValue(0, 1))
        params.waveTypeValue = randomValue(0, 1);

    params.startFrequencyValue = 0.5f + frnd(0.5f);
    params.minFrequencyValue = params.startFrequencyValue - 0.2f - frnd(0.6f);

    if (params.minFrequencyValue < 0.2f)
        params.minFrequencyValue = 0.2f;

    params.slideValue = -0.15f - frnd(0.2f);

    if (randomValue(0, 2) == 0) {
        params.startFrequencyValue = 0.3f + frnd(0.6f);
        params.minFrequencyValue = frnd(0.1f);
        params.slideValue = -0.35f - frnd(0.3f);
    }

    if (randomValue(0, 1)) {
        params.squareDutyValue = frnd(0.5f);
        params.dutySweepValue = frnd(0.2f);
    }
    else {
        params.squareDutyValue = 0.4f + frnd(0.5f);
        params.dutySweepValue = -frnd(0.7f);
    }

    params.attackTimeValue = 0.0f;
    params.sustainTimeValue = 0.1f + frnd(0.2f);
    params.decayTimeValue = frnd(0.4f);

    if (randomValue(0, 1))
        params.sustainPunchValue = frnd(0.3f);

    if (randomValue(0, 2) == 0) {
        params.phaserOffsetValue = frnd(0.2f);
        params.phaserSweepValue = -frnd(0.2f);
    }

    if (randomValue(0, 1))
        params.hpfCutoffValue = frnd(0.3f);

    return params;
}

// Generate sound: Explosion
static WaveParams GenExplosion(void)
{
    WaveParams params = {0};
    ResetWaveParams(&params);

    params.waveTypeValue = 3;

    if (randomValue(0, 1)) {
        params.startFrequencyValue = 0.1f + frnd(0.4f);
        params.slideValue = -0.1f + frnd(0.4f);
    }
    else {
        params.startFrequencyValue = 0.2f + frnd(0.7f);
        params.slideValue = -0.2f - frnd(0.2f);
    }

    params.startFrequencyValue *= params.startFrequencyValue;

    if (randomValue(0, 4) == 0)
        params.slideValue = 0.0f;
    if (randomValue(0, 2) == 0)
        params.repeatSpeedValue = 0.3f + frnd(0.5f);

    params.attackTimeValue = 0.0f;
    params.sustainTimeValue = 0.1f + frnd(0.3f);
    params.decayTimeValue = frnd(0.5f);

    if (randomValue(0, 1) == 0) {
        params.phaserOffsetValue = -0.3f + frnd(0.9f);
        params.phaserSweepValue = -frnd(0.3f);
    }

    params.sustainPunchValue = 0.2f + frnd(0.6f);

    if (randomValue(0, 1)) {
        params.vibratoDepthValue = frnd(0.7f);
        params.vibratoSpeedValue = frnd(0.6f);
    }

    if (randomValue(0, 2) == 0) {
        params.changeSpeedValue = 0.6f + frnd(0.3f);
        params.changeAmountValue = 0.8f - frnd(1.6f);
    }

    return params;
}

// Generate sound: Powerup
static WaveParams GenPowerup(void)
{
    WaveParams params = {0};
    ResetWaveParams(&params);

    if (randomValue(0, 1))
        params.waveTypeValue = 1;
    else
        params.squareDutyValue = frnd(0.6f);

    if (randomValue(0, 1)) {
        params.startFrequencyValue = 0.2f + frnd(0.3f);
        params.slideValue = 0.1f + frnd(0.4f);
        params.repeatSpeedValue = 0.4f + frnd(0.4f);
    }
    else {
        params.startFrequencyValue = 0.2f + frnd(0.3f);
        params.slideValue = 0.05f + frnd(0.2f);

        if (randomValue(0, 1)) {
            params.vibratoDepthValue = frnd(0.7f);
            params.vibratoSpeedValue = frnd(0.6f);
        }
    }

    params.attackTimeValue = 0.0f;
    params.sustainTimeValue = frnd(0.4f);
    params.decayTimeValue = 0.1f + frnd(0.4f);

    return params;
}

// Generate sound: Hit/Hurt
static WaveParams GenHitHurt(void)
{
    WaveParams params = {0};
    ResetWaveParams(&params);

    params.waveTypeValue = randomValue(0, 2);
    if (params.waveTypeValue == 2)
        params.waveTypeValue = 3;
    if (params.waveTypeValue == 0)
        params.squareDutyValue = frnd(0.6f);

    params.startFrequencyValue = 0.2f + frnd(0.6f);
    params.slideValue = -0.3f - frnd(0.4f);
    params.attackTimeValue = 0.0f;
    params.sustainTimeValue = frnd(0.1f);
    params.decayTimeValue = 0.1f + frnd(0.2f);

    if (randomValue(0, 1))
        params.hpfCutoffValue = frnd(0.3f);

    return params;
}

// Generate sound: Jump
static WaveParams GenJump(void)
{
    WaveParams params = {0};
    ResetWaveParams(&params);

    params.waveTypeValue = 0;
    params.squareDutyValue = frnd(0.6f);
    params.startFrequencyValue = 0.3f + frnd(0.3f);
    params.slideValue = 0.1f + frnd(0.2f);
    params.attackTimeValue = 0.0f;
    params.sustainTimeValue = 0.1f + frnd(0.3f);
    params.decayTimeValue = 0.1f + frnd(0.2f);

    if (randomValue(0, 1))
        params.hpfCutoffValue = frnd(0.3f);
    if (randomValue(0, 1))
        params.lpfCutoffValue = 1.0f - frnd(0.6f);

    return params;
}

// Generate sound: Blip/Select
static WaveParams GenBlipSelect(void)
{
    WaveParams params = {0};
    ResetWaveParams(&params);

    params.waveTypeValue = randomValue(0, 1);
    if (params.waveTypeValue == 0)
        params.squareDutyValue = frnd(0.6f);
    params.startFrequencyValue = 0.2f + frnd(0.4f);
    params.attackTimeValue = 0.0f;
    params.sustainTimeValue = 0.1f + frnd(0.1f);
    params.decayTimeValue = frnd(0.2f);
    params.hpfCutoffValue = 0.1f;

    return params;
}

// Generate random sound
static WaveParams GenRandomize(void)
{
    WaveParams params = {0};
    ResetWaveParams(&params);

    params.randSeed = randomValue(0, 0xFFFE);

    params.startFrequencyValue = powf(frnd(2.0f) - 1.0f, 2.0f);

    if (randomValue(0, 1))
        params.startFrequencyValue = powf(frnd(2.0f) - 1.0f, 3.0f) + 0.5f;

    params.minFrequencyValue = 0.0f;
    params.slideValue = powf(frnd(2.0f) - 1.0f, 5.0f);

    if ((params.startFrequencyValue > 0.7f) && (params.slideValue > 0.2f))
        params.slideValue = -params.slideValue;
    if ((params.startFrequencyValue < 0.2f) && (params.slideValue < -0.05f))
        params.slideValue = -params.slideValue;

    params.deltaSlideValue = powf(frnd(2.0f) - 1.0f, 3.0f);
    params.squareDutyValue = frnd(2.0f) - 1.0f;
    params.dutySweepValue = powf(frnd(2.0f) - 1.0f, 3.0f);
    params.vibratoDepthValue = powf(frnd(2.0f) - 1.0f, 3.0f);
    params.vibratoSpeedValue = frnd(2.0f) - 1.0f;
    // params.vibratoPhaseDelay = frnd(2.0f) - 1.0f;
    params.attackTimeValue = powf(frnd(2.0f) - 1.0f, 3.0f);
    params.sustainTimeValue = powf(frnd(2.0f) - 1.0f, 2.0f);
    params.decayTimeValue = frnd(2.0f) - 1.0f;
    params.sustainPunchValue = powf(frnd(0.8f), 2.0f);

    if (params.attackTimeValue + params.sustainTimeValue + params.decayTimeValue < 0.2f) {
        params.sustainTimeValue += 0.2f + frnd(0.3f);
        params.decayTimeValue += 0.2f + frnd(0.3f);
    }

    params.lpfResonanceValue = frnd(2.0f) - 1.0f;
    params.lpfCutoffValue = 1.0f - powf(frnd(1.0f), 3.0f);
    params.lpfCutoffSweepValue = powf(frnd(2.0f) - 1.0f, 3.0f);

    if (params.lpfCutoffValue < 0.1f && params.lpfCutoffSweepValue < -0.05f)
        params.lpfCutoffSweepValue = -params.lpfCutoffSweepValue;

    params.hpfCutoffValue = powf(frnd(1.0f), 5.0f);
    params.hpfCutoffSweepValue = powf(frnd(2.0f) - 1.0f, 5.0f);
    params.phaserOffsetValue = powf(frnd(2.0f) - 1.0f, 3.0f);
    params.phaserSweepValue = powf(frnd(2.0f) - 1.0f, 3.0f);
    params.repeatSpeedValue = frnd(2.0f) - 1.0f;
    params.changeSpeedValue = frnd(2.0f) - 1.0f;
    params.changeAmountValue = frnd(2.0f) - 1.0f;

    return params;
}

//-----------------------------------------------------------------------------
// END Nose Generation from rFXGen
//-----------------------------------------------------------------------------

Wave Procedural::generateWave(const WaveParams& params)
{
    return GenerateWave(params);
}

WaveParams Procedural::generateRandomSet(NoiseType type)
{
    switch (type) {
        case NoiseType::RANDOM:
            return GenRandomize();
        case NoiseType::PICKUP_COIN:
            return GenPickupCoin();
        case NoiseType::LASER_SHOOT:
            return GenLaserShoot();
        case NoiseType::EXPLOSION:
            return GenExplosion();
        case NoiseType::POWER_UP:
            return GenPowerup();
        case NoiseType::HIT_HURT:
            return GenHitHurt();
        case NoiseType::JUMP:
            return GenJump();
        case NoiseType::BLIP_SELECT:
            return GenBlipSelect();
    }
}
