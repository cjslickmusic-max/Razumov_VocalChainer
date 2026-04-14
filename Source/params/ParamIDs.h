#pragma once

#include <cmath>

namespace razumov::params
{

/** Строковые ID параметров APVTS (версия задаётся в ParameterLayout через ParameterID). */
inline constexpr const char* micBypass = "micBypass";
inline constexpr const char* micAmount = "micAmount";

inline constexpr const char* gainDb = "gainDb";
/** Gain module: knob / macro range (dB). Min maps to true silence (linear 0), not 10^(min/20). */
inline constexpr float kGainModuleDbRangeMin = -120.0f;
inline constexpr float kGainModuleDbRangeMax = 12.0f;

/** Maps module gain dB to linear gain; at/near UI minimum -> 0 (mute), matching "-inf" behavior. */
inline float gainModuleDbToLinear(float db) noexcept
{
    if (db <= kGainModuleDbRangeMin + 1e-3f)
        return 0.0f;
    return std::pow(10.0f, 0.05f * db);
}
inline constexpr const char* lowpassHz = "lowpassHz";

inline constexpr const char* deessCrossoverHz = "deessCrossoverHz";
inline constexpr const char* deessThresholdDb = "deessThresholdDb";
inline constexpr const char* deessRatio = "deessRatio";

inline constexpr const char* optoThresholdDb = "optoThresholdDb";
inline constexpr const char* optoRatio = "optoRatio";
inline constexpr const char* optoMakeupDb = "optoMakeupDb";

inline constexpr const char* fetThresholdDb = "fetThresholdDb";
inline constexpr const char* fetRatio = "fetRatio";
inline constexpr const char* fetMakeupDb = "fetMakeupDb";

inline constexpr const char* vcaThresholdDb = "vcaThresholdDb";
inline constexpr const char* vcaRatio = "vcaRatio";
inline constexpr const char* vcaMakeupDb = "vcaMakeupDb";

inline constexpr const char* exciterDrive = "exciterDrive";
inline constexpr const char* exciterMix = "exciterMix";

inline constexpr const char* spectralBypass = "spectralBypass";
inline constexpr const char* spectralMix = "spectralMix";
inline constexpr const char* spectralThresholdDb = "spectralThresholdDb";
inline constexpr const char* spectralRatio = "spectralRatio";
/** Sidechain band (frequency-selective detector). */
inline constexpr const char* spectralScFreqHz = "spectralScFreqHz";
inline constexpr const char* spectralScQ = "spectralScQ";
inline constexpr const char* spectralAttackMs = "spectralAttackMs";
inline constexpr const char* spectralReleaseMs = "spectralReleaseMs";

inline constexpr const char* eqBypass = "eqBypass";
inline constexpr const char* eqBand1FreqHz = "eqBand1FreqHz";
inline constexpr const char* eqBand1GainDb = "eqBand1GainDb";
inline constexpr const char* eqBand1Q = "eqBand1Q";
inline constexpr const char* eqBand2FreqHz = "eqBand2FreqHz";
inline constexpr const char* eqBand2GainDb = "eqBand2GainDb";
inline constexpr const char* eqBand2Q = "eqBand2Q";
inline constexpr const char* eqBand3FreqHz = "eqBand3FreqHz";
inline constexpr const char* eqBand3GainDb = "eqBand3GainDb";
inline constexpr const char* eqBand3Q = "eqBand3Q";
inline constexpr const char* eqBand4FreqHz = "eqBand4FreqHz";
inline constexpr const char* eqBand4GainDb = "eqBand4GainDb";
inline constexpr const char* eqBand4Q = "eqBand4Q";
inline constexpr const char* eqBand5FreqHz = "eqBand5FreqHz";
inline constexpr const char* eqBand5GainDb = "eqBand5GainDb";
inline constexpr const char* eqBand5Q = "eqBand5Q";
inline constexpr const char* eqBand1Type = "eqBand1Type";
inline constexpr const char* eqBand2Type = "eqBand2Type";
inline constexpr const char* eqBand3Type = "eqBand3Type";
inline constexpr const char* eqBand4Type = "eqBand4Type";
inline constexpr const char* eqBand5Type = "eqBand5Type";
/** 0...10: сколько полос EQ реально в каскаде (остальные не обрабатываются). */
inline constexpr const char* eqActiveBandCount = "eqActiveBandCount";
inline constexpr const char* eqBand6FreqHz = "eqBand6FreqHz";
inline constexpr const char* eqBand6GainDb = "eqBand6GainDb";
inline constexpr const char* eqBand6Q = "eqBand6Q";
inline constexpr const char* eqBand7FreqHz = "eqBand7FreqHz";
inline constexpr const char* eqBand7GainDb = "eqBand7GainDb";
inline constexpr const char* eqBand7Q = "eqBand7Q";
inline constexpr const char* eqBand8FreqHz = "eqBand8FreqHz";
inline constexpr const char* eqBand8GainDb = "eqBand8GainDb";
inline constexpr const char* eqBand8Q = "eqBand8Q";
inline constexpr const char* eqBand9FreqHz = "eqBand9FreqHz";
inline constexpr const char* eqBand9GainDb = "eqBand9GainDb";
inline constexpr const char* eqBand9Q = "eqBand9Q";
inline constexpr const char* eqBand10FreqHz = "eqBand10FreqHz";
inline constexpr const char* eqBand10GainDb = "eqBand10GainDb";
inline constexpr const char* eqBand10Q = "eqBand10Q";
inline constexpr const char* eqBand6Type = "eqBand6Type";
inline constexpr const char* eqBand7Type = "eqBand7Type";
inline constexpr const char* eqBand8Type = "eqBand8Type";
inline constexpr const char* eqBand9Type = "eqBand9Type";
inline constexpr const char* eqBand10Type = "eqBand10Type";
/** LP/HP: slope 0...96 dB/oct (cascade stages). */
inline constexpr const char* eqBand1Slope = "eqBand1Slope";
inline constexpr const char* eqBand2Slope = "eqBand2Slope";
inline constexpr const char* eqBand3Slope = "eqBand3Slope";
inline constexpr const char* eqBand4Slope = "eqBand4Slope";
inline constexpr const char* eqBand5Slope = "eqBand5Slope";
inline constexpr const char* eqBand6Slope = "eqBand6Slope";
inline constexpr const char* eqBand7Slope = "eqBand7Slope";
inline constexpr const char* eqBand8Slope = "eqBand8Slope";
inline constexpr const char* eqBand9Slope = "eqBand9Slope";
inline constexpr const char* eqBand10Slope = "eqBand10Slope";

/** Стартовая топология графа (фаза 4.2): Full / Compact / FET-forward. */
inline constexpr const char* chainProfile = "chainProfile";

/** Профиль микрофона (данные позже); сейчас только выбор в UI. */
inline constexpr const char* micProfile = "micProfile";

/** Макросы 0…1, нейтраль 0.5 (фаза 4.3) — смещение групп параметров в DSP. */
inline constexpr const char* macroGlue = "macroGlue";
inline constexpr const char* macroAir = "macroAir";
inline constexpr const char* macroSibilance = "macroSibilance";
inline constexpr const char* macroPresence = "macroPresence";

/** Доп. макросы (0...1, нейтраль 0.5) — расширение до x8. */
inline constexpr const char* macroPunch = "macroPunch";
inline constexpr const char* macroBody = "macroBody";
inline constexpr const char* macroSmooth = "macroSmooth";
inline constexpr const char* macroDensity = "macroDensity";

} // namespace razumov::params
