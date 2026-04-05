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
    /** Weighted sidechain band center (Hz) and Q; envelope times (ms). */
    float spectralScFreqHz { 2000.0f };
    float spectralScQ { 1.2f };
    float spectralAttackMs { 50.0f };
    float spectralReleaseMs { 250.0f };

    bool eqBypass { false };
    /** 0...10: number of EQ bands in cascade (new slots default 0; no dots until user adds). */
    float eqActiveBandCount { 0.0f };
    float eqBand1FreqHz { 120.0f };
    float eqBand1GainDb { 0.0f };
    float eqBand1Q { 1.0f };
    float eqBand2FreqHz { 400.0f };
    float eqBand2GainDb { 0.0f };
    float eqBand2Q { 1.0f };
    float eqBand3FreqHz { 2500.0f };
    float eqBand3GainDb { 0.0f };
    float eqBand3Q { 1.0f };
    float eqBand4FreqHz { 7000.0f };
    float eqBand4GainDb { 0.0f };
    float eqBand4Q { 1.0f };
    float eqBand5FreqHz { 10000.0f };
    float eqBand5GainDb { 0.0f };
    float eqBand5Q { 1.0f };
    /** EqBandType as float 0..5 (Peak, LowShelf, HighShelf, LowPass, HighPass, Notch). */
    float eqBand1Type { 0.0f };
    float eqBand2Type { 0.0f };
    float eqBand3Type { 0.0f };
    float eqBand4Type { 0.0f };
    float eqBand5Type { 0.0f };
    float eqBand6FreqHz { 12000.0f };
    float eqBand6GainDb { 0.0f };
    float eqBand6Q { 1.0f };
    float eqBand7FreqHz { 13000.0f };
    float eqBand7GainDb { 0.0f };
    float eqBand7Q { 1.0f };
    float eqBand8FreqHz { 15000.0f };
    float eqBand8GainDb { 0.0f };
    float eqBand8Q { 1.0f };
    float eqBand9FreqHz { 16000.0f };
    float eqBand9GainDb { 0.0f };
    float eqBand9Q { 1.0f };
    float eqBand10FreqHz { 18000.0f };
    float eqBand10GainDb { 0.0f };
    float eqBand10Q { 1.0f };
    float eqBand6Type { 0.0f };
    float eqBand7Type { 0.0f };
    float eqBand8Type { 0.0f };
    float eqBand9Type { 0.0f };
    float eqBand10Type { 0.0f };
    /** LowPass/HighPass: rolloff 0...96 dB/oct (other types: ignored in DSP). */
    float eqBand1Slope { 48.0f };
    float eqBand2Slope { 48.0f };
    float eqBand3Slope { 48.0f };
    float eqBand4Slope { 48.0f };
    float eqBand5Slope { 48.0f };
    float eqBand6Slope { 48.0f };
    float eqBand7Slope { 48.0f };
    float eqBand8Slope { 48.0f };
    float eqBand9Slope { 48.0f };
    float eqBand10Slope { 48.0f };
};

} // namespace razumov::params
