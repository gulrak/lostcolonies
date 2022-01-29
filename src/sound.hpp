#pragma once

#include "procedural.hpp"

#include <cstdint>
#include <map>
#include <vector>


using VolumeWaveParams = std::pair<float, WaveParams>;
class SoundManager
{
public:
    static SoundManager* instance();

    void unloadSounds();
    void generateSounds(int id, std::initializer_list<VolumeWaveParams> variants);
    void generateAndPlaySound(NoiseType type);

    void playSound(int id) const;

private:
    SoundManager() = default;
    ~SoundManager();

    using SoundVariants = std::vector<Sound>;
    std::map<int, SoundVariants> _soundMap;
};