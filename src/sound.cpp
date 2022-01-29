#include "sound.hpp"

SoundManager::~SoundManager()
{
    if(!_soundMap.empty()) {
        unloadSounds();
    }
}

SoundManager* SoundManager::instance()
{
    static SoundManager _instance;
    return &_instance;
}


void SoundManager::generateSounds(int id, std::initializer_list<VolumeWaveParams> variants)
{
    for(const auto& [volume,variant] : variants) {
        auto wave = Procedural::generateWave(variant);
        _soundMap[id].push_back(LoadSoundFromWave(wave));
        SetSoundVolume(_soundMap[id].back(), volume);
        UnloadWave(wave);
    }
}

void SoundManager::generateAndPlaySound(NoiseType type)
{
    constexpr int MAGIC = 0x123456;
    StopSoundMulti();
    auto params = Procedural::generateRandomSet(type);
    auto wave = Procedural::generateWave(params);
    if(_soundMap.count(MAGIC)) {
        UnloadSound(_soundMap[MAGIC][0]);
        _soundMap.erase(MAGIC);
    }
    _soundMap[MAGIC].push_back(LoadSoundFromWave(wave));
    playSound(MAGIC);
    UnloadWave(wave);
    SetClipboardText(TextFormat("{ %d, %d, %ff, %ff, %ff, %ff, %ff, %ff, %ff, %ff, %ff, %ff, %ff, %ff, %ff, %ff, %ff, %ff, %ff, %ff, %ff, %ff, %ff, %ff }",
                                params.randSeed, params.waveTypeValue,

                                // Wave envelope parameters
                                params.attackTimeValue,
                                params.sustainTimeValue,
                                params.sustainPunchValue,
                                params.decayTimeValue,

                                // Frequency parameters
                                params.startFrequencyValue,
                                params.minFrequencyValue,
                                params.slideValue,
                                params.deltaSlideValue,
                                params.vibratoDepthValue,
                                params.vibratoSpeedValue,
                                // float vibratoPhaseDelayValue;

                                // Tone change parameters
                                params.changeAmountValue,
                                params.changeSpeedValue,

                                // Square wave parameters
                                params.squareDutyValue,
                                params.dutySweepValue,

                                // Repeat parameters
                                params.repeatSpeedValue,

                                // Phaser parameters
                                params.phaserOffsetValue,
                                params.phaserSweepValue,

                                // Filter parameters
                                params.lpfCutoffValue,
                                params.lpfCutoffSweepValue,
                                params.lpfResonanceValue,
                                params.hpfCutoffValue,
                                params.hpfCutoffSweepValue));
}

void SoundManager::unloadSounds()
{
    StopSoundMulti();
    for(auto& [id, variants] : _soundMap) {
        for(auto& sound : variants) {
            UnloadSound(sound);
        }
    }
    _soundMap.clear();
}

void SoundManager::playSound(int id) const
{
    auto iter = _soundMap.find(id);
    if(iter != _soundMap.end()) {
        auto variant = GetRandomValue(0, static_cast<int>(iter->second.size()-1));
        PlaySoundMulti(iter->second[variant]);
    }
}
