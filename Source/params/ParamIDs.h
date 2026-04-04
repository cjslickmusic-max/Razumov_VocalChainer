#pragma once

namespace razumov::params
{

/** Строковые ID параметров APVTS (версия задаётся в ParameterLayout через ParameterID). */
inline constexpr const char* micBypass = "micBypass";
inline constexpr const char* micAmount = "micAmount";

inline constexpr const char* gainDb = "gainDb";
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
