#pragma once

namespace razumov::params
{

/** Снимок параметров для audio thread (после чтения APVTS). */
struct Phase3RealtimeParams
{
    bool micBypass { false };
    float micAmount { 1.0f };

    float gainLinear { 1.0f };
    float lowpassHz { 20000.0f };

    float deessCrossoverHz { 6000.0f };
    float deessThresholdDb { -20.0f };
    float deessRatio { 3.0f };

    float optoThresholdDb { -12.0f };
    float optoRatio { 3.0f };
    float optoMakeupDb { 0.0f };

    float fetThresholdDb { -12.0f };
    float fetRatio { 4.0f };
    float fetMakeupDb { 0.0f };

    float vcaThresholdDb { -12.0f };
    float vcaRatio { 2.5f };
    float vcaMakeupDb { 0.0f };

    float exciterDrive { 1.5f };
    float exciterMix { 0.15f };

    bool spectralBypass { false };
    float spectralMix { 0.75f };
    float spectralThresholdDb { -24.0f };
    float spectralRatio { 3.0f };
};

} // namespace razumov::params
