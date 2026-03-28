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
