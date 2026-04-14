#include "ModuleParamsRuntime.h"
#include "MacroRouting.h"
#include "ParamIDs.h"

#include <juce_dsp/juce_dsp.h>
#include <algorithm>
#include <cmath>

namespace razumov::params
{
namespace detail
{

constexpr float kEqGainDbAbsMax = 12.f;
inline float clampEqGainDb(float v) noexcept
{
    return juce::jlimit(-kEqGainDbAbsMax, kEqGainDbAbsMax, v);
}

constexpr float kEqSlopeDbPerOctMax = 96.f;
inline float clampEqSlopeDbPerOct(float v) noexcept
{
    return juce::jlimit(0.f, kEqSlopeDbPerOctMax, v);
}

struct ModuleSlotBlock
{
    std::atomic<float> micBypass { 0.0f };
    std::atomic<float> micAmount { 1.0f };
    std::atomic<float> gainDb { 0.0f };
    std::atomic<float> lowpassHz { 20000.0f };
    std::atomic<float> deessCrossoverHz { 6000.0f };
    std::atomic<float> deessThresholdDb { -20.0f };
    std::atomic<float> deessRatio { 3.0f };
    std::atomic<float> optoThresholdDb { -12.0f };
    std::atomic<float> optoRatio { 3.0f };
    std::atomic<float> optoMakeupDb { 0.0f };
    std::atomic<float> fetThresholdDb { -12.0f };
    std::atomic<float> fetRatio { 4.0f };
    std::atomic<float> fetMakeupDb { 0.0f };
    std::atomic<float> vcaThresholdDb { -12.0f };
    std::atomic<float> vcaRatio { 2.5f };
    std::atomic<float> vcaMakeupDb { 0.0f };
    std::atomic<float> exciterDrive { 1.5f };
    std::atomic<float> exciterMix { 0.15f };
    std::atomic<float> spectralBypass { 0.0f };
    std::atomic<float> spectralMix { 0.75f };
    std::atomic<float> spectralThresholdDb { -24.0f };
    std::atomic<float> spectralRatio { 3.0f };
    std::atomic<float> spectralScFreqHz { 2000.0f };
    std::atomic<float> spectralScQ { 1.2f };
    std::atomic<float> spectralAttackMs { 50.0f };
    std::atomic<float> spectralReleaseMs { 250.0f };

    std::atomic<float> eqBypass { 0.0f };
    std::atomic<float> eqBand1FreqHz { 120.0f };
    std::atomic<float> eqBand1GainDb { 0.0f };
    std::atomic<float> eqBand1Q { 1.0f };
    std::atomic<float> eqBand2FreqHz { 400.0f };
    std::atomic<float> eqBand2GainDb { 0.0f };
    std::atomic<float> eqBand2Q { 1.0f };
    std::atomic<float> eqBand3FreqHz { 2500.0f };
    std::atomic<float> eqBand3GainDb { 0.0f };
    std::atomic<float> eqBand3Q { 1.0f };
    std::atomic<float> eqBand4FreqHz { 7000.0f };
    std::atomic<float> eqBand4GainDb { 0.0f };
    std::atomic<float> eqBand4Q { 1.0f };
    std::atomic<float> eqBand5FreqHz { 10000.0f };
    std::atomic<float> eqBand5GainDb { 0.0f };
    std::atomic<float> eqBand5Q { 1.0f };
    std::atomic<float> eqBand1Type { 0.0f };
    std::atomic<float> eqBand2Type { 0.0f };
    std::atomic<float> eqBand3Type { 0.0f };
    std::atomic<float> eqBand4Type { 0.0f };
    std::atomic<float> eqBand5Type { 0.0f };
    std::atomic<float> eqActiveBandCount { 0.0f };
    std::atomic<float> eqBand6FreqHz { 12000.0f };
    std::atomic<float> eqBand6GainDb { 0.0f };
    std::atomic<float> eqBand6Q { 1.0f };
    std::atomic<float> eqBand7FreqHz { 13000.0f };
    std::atomic<float> eqBand7GainDb { 0.0f };
    std::atomic<float> eqBand7Q { 1.0f };
    std::atomic<float> eqBand8FreqHz { 15000.0f };
    std::atomic<float> eqBand8GainDb { 0.0f };
    std::atomic<float> eqBand8Q { 1.0f };
    std::atomic<float> eqBand9FreqHz { 16000.0f };
    std::atomic<float> eqBand9GainDb { 0.0f };
    std::atomic<float> eqBand9Q { 1.0f };
    std::atomic<float> eqBand10FreqHz { 18000.0f };
    std::atomic<float> eqBand10GainDb { 0.0f };
    std::atomic<float> eqBand10Q { 1.0f };
    std::atomic<float> eqBand6Type { 0.0f };
    std::atomic<float> eqBand7Type { 0.0f };
    std::atomic<float> eqBand8Type { 0.0f };
    std::atomic<float> eqBand9Type { 0.0f };
    std::atomic<float> eqBand10Type { 0.0f };
    std::atomic<float> eqBand1Slope { 48.0f };
    std::atomic<float> eqBand2Slope { 48.0f };
    std::atomic<float> eqBand3Slope { 48.0f };
    std::atomic<float> eqBand4Slope { 48.0f };
    std::atomic<float> eqBand5Slope { 48.0f };
    std::atomic<float> eqBand6Slope { 48.0f };
    std::atomic<float> eqBand7Slope { 48.0f };
    std::atomic<float> eqBand8Slope { 48.0f };
    std::atomic<float> eqBand9Slope { 48.0f };
    std::atomic<float> eqBand10Slope { 48.0f };
};

inline void copyModuleSlotBlockState(const ModuleSlotBlock& src, ModuleSlotBlock& dst)
{
    dst.micBypass.store(src.micBypass.load());
    dst.micAmount.store(src.micAmount.load());
    dst.gainDb.store(src.gainDb.load());
    dst.lowpassHz.store(src.lowpassHz.load());
    dst.deessCrossoverHz.store(src.deessCrossoverHz.load());
    dst.deessThresholdDb.store(src.deessThresholdDb.load());
    dst.deessRatio.store(src.deessRatio.load());
    dst.optoThresholdDb.store(src.optoThresholdDb.load());
    dst.optoRatio.store(src.optoRatio.load());
    dst.optoMakeupDb.store(src.optoMakeupDb.load());
    dst.fetThresholdDb.store(src.fetThresholdDb.load());
    dst.fetRatio.store(src.fetRatio.load());
    dst.fetMakeupDb.store(src.fetMakeupDb.load());
    dst.vcaThresholdDb.store(src.vcaThresholdDb.load());
    dst.vcaRatio.store(src.vcaRatio.load());
    dst.vcaMakeupDb.store(src.vcaMakeupDb.load());
    dst.exciterDrive.store(src.exciterDrive.load());
    dst.exciterMix.store(src.exciterMix.load());
    dst.spectralBypass.store(src.spectralBypass.load());
    dst.spectralMix.store(src.spectralMix.load());
    dst.spectralThresholdDb.store(src.spectralThresholdDb.load());
    dst.spectralRatio.store(src.spectralRatio.load());
    dst.spectralScFreqHz.store(src.spectralScFreqHz.load());
    dst.spectralScQ.store(src.spectralScQ.load());
    dst.spectralAttackMs.store(src.spectralAttackMs.load());
    dst.spectralReleaseMs.store(src.spectralReleaseMs.load());
    dst.eqBypass.store(src.eqBypass.load());
    dst.eqBand1FreqHz.store(src.eqBand1FreqHz.load());
    dst.eqBand1GainDb.store(detail::clampEqGainDb(src.eqBand1GainDb.load()));
    dst.eqBand1Q.store(src.eqBand1Q.load());
    dst.eqBand2FreqHz.store(src.eqBand2FreqHz.load());
    dst.eqBand2GainDb.store(detail::clampEqGainDb(src.eqBand2GainDb.load()));
    dst.eqBand2Q.store(src.eqBand2Q.load());
    dst.eqBand3FreqHz.store(src.eqBand3FreqHz.load());
    dst.eqBand3GainDb.store(detail::clampEqGainDb(src.eqBand3GainDb.load()));
    dst.eqBand3Q.store(src.eqBand3Q.load());
    dst.eqBand4FreqHz.store(src.eqBand4FreqHz.load());
    dst.eqBand4GainDb.store(detail::clampEqGainDb(src.eqBand4GainDb.load()));
    dst.eqBand4Q.store(src.eqBand4Q.load());
    dst.eqBand5FreqHz.store(src.eqBand5FreqHz.load());
    dst.eqBand5GainDb.store(detail::clampEqGainDb(src.eqBand5GainDb.load()));
    dst.eqBand5Q.store(src.eqBand5Q.load());
    dst.eqBand1Type.store(src.eqBand1Type.load());
    dst.eqBand2Type.store(src.eqBand2Type.load());
    dst.eqBand3Type.store(src.eqBand3Type.load());
    dst.eqBand4Type.store(src.eqBand4Type.load());
    dst.eqBand5Type.store(src.eqBand5Type.load());
    dst.eqActiveBandCount.store(src.eqActiveBandCount.load());
    dst.eqBand6FreqHz.store(src.eqBand6FreqHz.load());
    dst.eqBand6GainDb.store(detail::clampEqGainDb(src.eqBand6GainDb.load()));
    dst.eqBand6Q.store(src.eqBand6Q.load());
    dst.eqBand7FreqHz.store(src.eqBand7FreqHz.load());
    dst.eqBand7GainDb.store(detail::clampEqGainDb(src.eqBand7GainDb.load()));
    dst.eqBand7Q.store(src.eqBand7Q.load());
    dst.eqBand8FreqHz.store(src.eqBand8FreqHz.load());
    dst.eqBand8GainDb.store(detail::clampEqGainDb(src.eqBand8GainDb.load()));
    dst.eqBand8Q.store(src.eqBand8Q.load());
    dst.eqBand9FreqHz.store(src.eqBand9FreqHz.load());
    dst.eqBand9GainDb.store(detail::clampEqGainDb(src.eqBand9GainDb.load()));
    dst.eqBand9Q.store(src.eqBand9Q.load());
    dst.eqBand10FreqHz.store(src.eqBand10FreqHz.load());
    dst.eqBand10GainDb.store(detail::clampEqGainDb(src.eqBand10GainDb.load()));
    dst.eqBand10Q.store(src.eqBand10Q.load());
    dst.eqBand6Type.store(src.eqBand6Type.load());
    dst.eqBand7Type.store(src.eqBand7Type.load());
    dst.eqBand8Type.store(src.eqBand8Type.load());
    dst.eqBand9Type.store(src.eqBand9Type.load());
    dst.eqBand10Type.store(src.eqBand10Type.load());
    dst.eqBand1Slope.store(detail::clampEqSlopeDbPerOct(src.eqBand1Slope.load()));
    dst.eqBand2Slope.store(detail::clampEqSlopeDbPerOct(src.eqBand2Slope.load()));
    dst.eqBand3Slope.store(detail::clampEqSlopeDbPerOct(src.eqBand3Slope.load()));
    dst.eqBand4Slope.store(detail::clampEqSlopeDbPerOct(src.eqBand4Slope.load()));
    dst.eqBand5Slope.store(detail::clampEqSlopeDbPerOct(src.eqBand5Slope.load()));
    dst.eqBand6Slope.store(detail::clampEqSlopeDbPerOct(src.eqBand6Slope.load()));
    dst.eqBand7Slope.store(detail::clampEqSlopeDbPerOct(src.eqBand7Slope.load()));
    dst.eqBand8Slope.store(detail::clampEqSlopeDbPerOct(src.eqBand8Slope.load()));
    dst.eqBand9Slope.store(detail::clampEqSlopeDbPerOct(src.eqBand9Slope.load()));
    dst.eqBand10Slope.store(detail::clampEqSlopeDbPerOct(src.eqBand10Slope.load()));
}

} // namespace detail

namespace
{

void storeFromPhase3(detail::ModuleSlotBlock& b, const Phase3RealtimeParams& p)
{
    b.micBypass.store(p.micBypass ? 1.0f : 0.0f);
    b.micAmount.store(p.micAmount);
    b.gainDb.store(juce::Decibels::gainToDecibels(p.gainLinear, -200.0f));
    b.lowpassHz.store(p.lowpassHz);
    b.deessCrossoverHz.store(p.deessCrossoverHz);
    b.deessThresholdDb.store(p.deessThresholdDb);
    b.deessRatio.store(p.deessRatio);
    b.optoThresholdDb.store(p.optoThresholdDb);
    b.optoRatio.store(p.optoRatio);
    b.optoMakeupDb.store(p.optoMakeupDb);
    b.fetThresholdDb.store(p.fetThresholdDb);
    b.fetRatio.store(p.fetRatio);
    b.fetMakeupDb.store(p.fetMakeupDb);
    b.vcaThresholdDb.store(p.vcaThresholdDb);
    b.vcaRatio.store(p.vcaRatio);
    b.vcaMakeupDb.store(p.vcaMakeupDb);
    b.exciterDrive.store(p.exciterDrive);
    b.exciterMix.store(p.exciterMix);
    b.spectralBypass.store(p.spectralBypass ? 1.0f : 0.0f);
    b.spectralMix.store(p.spectralMix);
    b.spectralThresholdDb.store(p.spectralThresholdDb);
    b.spectralRatio.store(p.spectralRatio);
    b.spectralScFreqHz.store(p.spectralScFreqHz);
    b.spectralScQ.store(p.spectralScQ);
    b.spectralAttackMs.store(p.spectralAttackMs);
    b.spectralReleaseMs.store(p.spectralReleaseMs);
    b.eqBypass.store(p.eqBypass ? 1.0f : 0.0f);
    b.eqBand1FreqHz.store(p.eqBand1FreqHz);
    b.eqBand1GainDb.store(detail::clampEqGainDb(p.eqBand1GainDb));
    b.eqBand1Q.store(p.eqBand1Q);
    b.eqBand2FreqHz.store(p.eqBand2FreqHz);
    b.eqBand2GainDb.store(detail::clampEqGainDb(p.eqBand2GainDb));
    b.eqBand2Q.store(p.eqBand2Q);
    b.eqBand3FreqHz.store(p.eqBand3FreqHz);
    b.eqBand3GainDb.store(detail::clampEqGainDb(p.eqBand3GainDb));
    b.eqBand3Q.store(p.eqBand3Q);
    b.eqBand4FreqHz.store(p.eqBand4FreqHz);
    b.eqBand4GainDb.store(detail::clampEqGainDb(p.eqBand4GainDb));
    b.eqBand4Q.store(p.eqBand4Q);
    b.eqBand5FreqHz.store(p.eqBand5FreqHz);
    b.eqBand5GainDb.store(detail::clampEqGainDb(p.eqBand5GainDb));
    b.eqBand5Q.store(p.eqBand5Q);
    b.eqBand1Type.store(p.eqBand1Type);
    b.eqBand2Type.store(p.eqBand2Type);
    b.eqBand3Type.store(p.eqBand3Type);
    b.eqBand4Type.store(p.eqBand4Type);
    b.eqBand5Type.store(p.eqBand5Type);
    b.eqActiveBandCount.store(p.eqActiveBandCount);
    b.eqBand6FreqHz.store(p.eqBand6FreqHz);
    b.eqBand6GainDb.store(detail::clampEqGainDb(p.eqBand6GainDb));
    b.eqBand6Q.store(p.eqBand6Q);
    b.eqBand7FreqHz.store(p.eqBand7FreqHz);
    b.eqBand7GainDb.store(detail::clampEqGainDb(p.eqBand7GainDb));
    b.eqBand7Q.store(p.eqBand7Q);
    b.eqBand8FreqHz.store(p.eqBand8FreqHz);
    b.eqBand8GainDb.store(detail::clampEqGainDb(p.eqBand8GainDb));
    b.eqBand8Q.store(p.eqBand8Q);
    b.eqBand9FreqHz.store(p.eqBand9FreqHz);
    b.eqBand9GainDb.store(detail::clampEqGainDb(p.eqBand9GainDb));
    b.eqBand9Q.store(p.eqBand9Q);
    b.eqBand10FreqHz.store(p.eqBand10FreqHz);
    b.eqBand10GainDb.store(detail::clampEqGainDb(p.eqBand10GainDb));
    b.eqBand10Q.store(p.eqBand10Q);
    b.eqBand6Type.store(p.eqBand6Type);
    b.eqBand7Type.store(p.eqBand7Type);
    b.eqBand8Type.store(p.eqBand8Type);
    b.eqBand9Type.store(p.eqBand9Type);
    b.eqBand10Type.store(p.eqBand10Type);
    b.eqBand1Slope.store(detail::clampEqSlopeDbPerOct(p.eqBand1Slope));
    b.eqBand2Slope.store(detail::clampEqSlopeDbPerOct(p.eqBand2Slope));
    b.eqBand3Slope.store(detail::clampEqSlopeDbPerOct(p.eqBand3Slope));
    b.eqBand4Slope.store(detail::clampEqSlopeDbPerOct(p.eqBand4Slope));
    b.eqBand5Slope.store(detail::clampEqSlopeDbPerOct(p.eqBand5Slope));
    b.eqBand6Slope.store(detail::clampEqSlopeDbPerOct(p.eqBand6Slope));
    b.eqBand7Slope.store(detail::clampEqSlopeDbPerOct(p.eqBand7Slope));
    b.eqBand8Slope.store(detail::clampEqSlopeDbPerOct(p.eqBand8Slope));
    b.eqBand9Slope.store(detail::clampEqSlopeDbPerOct(p.eqBand9Slope));
    b.eqBand10Slope.store(detail::clampEqSlopeDbPerOct(p.eqBand10Slope));
}

void loadToPhase3(const detail::ModuleSlotBlock& b, Phase3RealtimeParams& out)
{
    out.micBypass = b.micBypass.load() > 0.5f;
    out.micAmount = b.micAmount.load();
    out.gainLinear = gainModuleDbToLinear(b.gainDb.load());
    out.lowpassHz = b.lowpassHz.load();
    out.deessCrossoverHz = b.deessCrossoverHz.load();
    out.deessThresholdDb = b.deessThresholdDb.load();
    out.deessRatio = b.deessRatio.load();
    out.optoThresholdDb = b.optoThresholdDb.load();
    out.optoRatio = b.optoRatio.load();
    out.optoMakeupDb = b.optoMakeupDb.load();
    out.fetThresholdDb = b.fetThresholdDb.load();
    out.fetRatio = b.fetRatio.load();
    out.fetMakeupDb = b.fetMakeupDb.load();
    out.vcaThresholdDb = b.vcaThresholdDb.load();
    out.vcaRatio = b.vcaRatio.load();
    out.vcaMakeupDb = b.vcaMakeupDb.load();
    out.exciterDrive = b.exciterDrive.load();
    out.exciterMix = b.exciterMix.load();
    out.spectralBypass = b.spectralBypass.load() > 0.5f;
    out.spectralMix = b.spectralMix.load();
    out.spectralThresholdDb = b.spectralThresholdDb.load();
    out.spectralRatio = b.spectralRatio.load();
    out.spectralScFreqHz = b.spectralScFreqHz.load();
    out.spectralScQ = b.spectralScQ.load();
    out.spectralAttackMs = b.spectralAttackMs.load();
    out.spectralReleaseMs = b.spectralReleaseMs.load();
    out.eqBypass = b.eqBypass.load() > 0.5f;
    out.eqBand1FreqHz = b.eqBand1FreqHz.load();
    out.eqBand1GainDb = detail::clampEqGainDb(b.eqBand1GainDb.load());
    out.eqBand1Q = b.eqBand1Q.load();
    out.eqBand2FreqHz = b.eqBand2FreqHz.load();
    out.eqBand2GainDb = detail::clampEqGainDb(b.eqBand2GainDb.load());
    out.eqBand2Q = b.eqBand2Q.load();
    out.eqBand3FreqHz = b.eqBand3FreqHz.load();
    out.eqBand3GainDb = detail::clampEqGainDb(b.eqBand3GainDb.load());
    out.eqBand3Q = b.eqBand3Q.load();
    out.eqBand4FreqHz = b.eqBand4FreqHz.load();
    out.eqBand4GainDb = detail::clampEqGainDb(b.eqBand4GainDb.load());
    out.eqBand4Q = b.eqBand4Q.load();
    out.eqBand5FreqHz = b.eqBand5FreqHz.load();
    out.eqBand5GainDb = detail::clampEqGainDb(b.eqBand5GainDb.load());
    out.eqBand5Q = b.eqBand5Q.load();
    out.eqBand1Type = b.eqBand1Type.load();
    out.eqBand2Type = b.eqBand2Type.load();
    out.eqBand3Type = b.eqBand3Type.load();
    out.eqBand4Type = b.eqBand4Type.load();
    out.eqBand5Type = b.eqBand5Type.load();
    out.eqActiveBandCount = b.eqActiveBandCount.load();
    out.eqBand6FreqHz = b.eqBand6FreqHz.load();
    out.eqBand6GainDb = detail::clampEqGainDb(b.eqBand6GainDb.load());
    out.eqBand6Q = b.eqBand6Q.load();
    out.eqBand7FreqHz = b.eqBand7FreqHz.load();
    out.eqBand7GainDb = detail::clampEqGainDb(b.eqBand7GainDb.load());
    out.eqBand7Q = b.eqBand7Q.load();
    out.eqBand8FreqHz = b.eqBand8FreqHz.load();
    out.eqBand8GainDb = detail::clampEqGainDb(b.eqBand8GainDb.load());
    out.eqBand8Q = b.eqBand8Q.load();
    out.eqBand9FreqHz = b.eqBand9FreqHz.load();
    out.eqBand9GainDb = detail::clampEqGainDb(b.eqBand9GainDb.load());
    out.eqBand9Q = b.eqBand9Q.load();
    out.eqBand10FreqHz = b.eqBand10FreqHz.load();
    out.eqBand10GainDb = detail::clampEqGainDb(b.eqBand10GainDb.load());
    out.eqBand10Q = b.eqBand10Q.load();
    out.eqBand6Type = b.eqBand6Type.load();
    out.eqBand7Type = b.eqBand7Type.load();
    out.eqBand8Type = b.eqBand8Type.load();
    out.eqBand9Type = b.eqBand9Type.load();
    out.eqBand10Type = b.eqBand10Type.load();
    out.eqBand1Slope = detail::clampEqSlopeDbPerOct(b.eqBand1Slope.load());
    out.eqBand2Slope = detail::clampEqSlopeDbPerOct(b.eqBand2Slope.load());
    out.eqBand3Slope = detail::clampEqSlopeDbPerOct(b.eqBand3Slope.load());
    out.eqBand4Slope = detail::clampEqSlopeDbPerOct(b.eqBand4Slope.load());
    out.eqBand5Slope = detail::clampEqSlopeDbPerOct(b.eqBand5Slope.load());
    out.eqBand6Slope = detail::clampEqSlopeDbPerOct(b.eqBand6Slope.load());
    out.eqBand7Slope = detail::clampEqSlopeDbPerOct(b.eqBand7Slope.load());
    out.eqBand8Slope = detail::clampEqSlopeDbPerOct(b.eqBand8Slope.load());
    out.eqBand9Slope = detail::clampEqSlopeDbPerOct(b.eqBand9Slope.load());
    out.eqBand10Slope = detail::clampEqSlopeDbPerOct(b.eqBand10Slope.load());
}

void initDefaults(detail::ModuleSlotBlock& b)
{
    Phase3RealtimeParams d;
    storeFromPhase3(b, d);
}

} // namespace

const juce::Identifier ModuleParamsRuntime::moduleParamsTreeType { "ModuleParams" };

ModuleParamsRuntime::ModuleParamsRuntime()
{
    static const char* const kDefaultNames[8] = { "Glue", "Air", "Sibil", "Presence", "Punch", "Body", "Smooth", "Density" };
    for (int i = 0; i < 8; ++i)
    {
        macroTargetSlot_[(size_t) i].store(0u, std::memory_order_relaxed);
        macroTargetParam_[(size_t) i].store((uint32_t) MacroTargetParam::None, std::memory_order_relaxed);
        macroDisplayNames_[(size_t) i] = kDefaultNames[(size_t) i];
    }
}

ModuleParamsRuntime::~ModuleParamsRuntime() = default;

void ModuleParamsRuntime::clear() noexcept
{
    slotIds_.clear();
    blocks_.clear();
    for (int i = 0; i < 8; ++i)
    {
        macroTargetSlot_[(size_t) i].store(0u, std::memory_order_relaxed);
        macroTargetParam_[(size_t) i].store((uint32_t) MacroTargetParam::None, std::memory_order_relaxed);
    }
}

int ModuleParamsRuntime::findSlotIndex(uint32_t slotId) const noexcept
{
    for (size_t i = 0; i < slotIds_.size(); ++i)
        if (slotIds_[i] == slotId)
            return (int) i;
    return -1;
}

detail::ModuleSlotBlock* ModuleParamsRuntime::findOrNull(uint32_t slotId) noexcept
{
    const int i = findSlotIndex(slotId);
    return i < 0 ? nullptr : blocks_[(size_t) i].get();
}

const detail::ModuleSlotBlock* ModuleParamsRuntime::findOrNull(uint32_t slotId) const noexcept
{
    const int i = findSlotIndex(slotId);
    return i < 0 ? nullptr : blocks_[(size_t) i].get();
}

void ModuleParamsRuntime::removeSlotAtIndex(size_t idx)
{
    if (idx >= slotIds_.size())
        return;
    slotIds_.erase(slotIds_.begin() + (ptrdiff_t) idx);
    blocks_.erase(blocks_.begin() + (ptrdiff_t) idx);
}

void ModuleParamsRuntime::ensureSlotWithDefaults(uint32_t slotId)
{
    if (findSlotIndex(slotId) >= 0)
        return;
    slotIds_.push_back(slotId);
    auto u = std::make_unique<detail::ModuleSlotBlock>();
    initDefaults(*u);
    blocks_.push_back(std::move(u));
}

void ModuleParamsRuntime::syncWithGraphSlotIds(const std::vector<uint32_t>& moduleSlotIds)
{
    for (uint32_t id : moduleSlotIds)
        ensureSlotWithDefaults(id);

    for (int i = (int) slotIds_.size() - 1; i >= 0; --i)
    {
        const uint32_t sid = slotIds_[(size_t) i];
        const bool keep = std::find(moduleSlotIds.begin(), moduleSlotIds.end(), sid) != moduleSlotIds.end();
        if (!keep)
            removeSlotAtIndex((size_t) i);
    }
}

void ModuleParamsRuntime::fillSlot(uint32_t slotId, const MacroAudioState& macros, Phase3RealtimeParams& out) const
{
    const detail::ModuleSlotBlock* b = findOrNull(slotId);
    if (b == nullptr)
        out = Phase3RealtimeParams {};
    else
        loadToPhase3(*b, out);

    for (int i = 0; i < 8; ++i)
    {
        const uint32_t ts = macroTargetSlot_[(size_t) i].load(std::memory_order_acquire);
        if (ts == 0 || ts != slotId)
            continue;
        const auto pk = (MacroTargetParam) macroTargetParam_[(size_t) i].load(std::memory_order_acquire);
        if (pk == MacroTargetParam::None)
            continue;
        applyMacroFullRangeToPhase3(out, pk, macros.getByIndex(i));
    }
}

float ModuleParamsRuntime::getFloat(uint32_t slotId, const juce::String& paramId) const
{
    const detail::ModuleSlotBlock* b = findOrNull(slotId);
    if (b == nullptr)
        return 0.0f;

    using namespace razumov::params;
    if (paramId == micAmount)
        return b->micAmount.load();
    if (paramId == gainDb)
        return b->gainDb.load();
    if (paramId == lowpassHz)
        return b->lowpassHz.load();
    if (paramId == deessCrossoverHz)
        return b->deessCrossoverHz.load();
    if (paramId == deessThresholdDb)
        return b->deessThresholdDb.load();
    if (paramId == deessRatio)
        return b->deessRatio.load();
    if (paramId == optoThresholdDb)
        return b->optoThresholdDb.load();
    if (paramId == optoRatio)
        return b->optoRatio.load();
    if (paramId == optoMakeupDb)
        return b->optoMakeupDb.load();
    if (paramId == fetThresholdDb)
        return b->fetThresholdDb.load();
    if (paramId == fetRatio)
        return b->fetRatio.load();
    if (paramId == fetMakeupDb)
        return b->fetMakeupDb.load();
    if (paramId == vcaThresholdDb)
        return b->vcaThresholdDb.load();
    if (paramId == vcaRatio)
        return b->vcaRatio.load();
    if (paramId == vcaMakeupDb)
        return b->vcaMakeupDb.load();
    if (paramId == exciterDrive)
        return b->exciterDrive.load();
    if (paramId == exciterMix)
        return b->exciterMix.load();
    if (paramId == spectralMix)
        return b->spectralMix.load();
    if (paramId == spectralThresholdDb)
        return b->spectralThresholdDb.load();
    if (paramId == spectralRatio)
        return b->spectralRatio.load();
    if (paramId == spectralScFreqHz)
        return b->spectralScFreqHz.load();
    if (paramId == spectralScQ)
        return b->spectralScQ.load();
    if (paramId == spectralAttackMs)
        return b->spectralAttackMs.load();
    if (paramId == spectralReleaseMs)
        return b->spectralReleaseMs.load();
    if (paramId == eqBand1FreqHz)
        return b->eqBand1FreqHz.load();
    if (paramId == eqBand1GainDb)
        return detail::clampEqGainDb(b->eqBand1GainDb.load());
    if (paramId == eqBand1Q)
        return b->eqBand1Q.load();
    if (paramId == eqBand2FreqHz)
        return b->eqBand2FreqHz.load();
    if (paramId == eqBand2GainDb)
        return detail::clampEqGainDb(b->eqBand2GainDb.load());
    if (paramId == eqBand2Q)
        return b->eqBand2Q.load();
    if (paramId == eqBand3FreqHz)
        return b->eqBand3FreqHz.load();
    if (paramId == eqBand3GainDb)
        return detail::clampEqGainDb(b->eqBand3GainDb.load());
    if (paramId == eqBand3Q)
        return b->eqBand3Q.load();
    if (paramId == eqBand4FreqHz)
        return b->eqBand4FreqHz.load();
    if (paramId == eqBand4GainDb)
        return detail::clampEqGainDb(b->eqBand4GainDb.load());
    if (paramId == eqBand4Q)
        return b->eqBand4Q.load();
    if (paramId == eqBand5FreqHz)
        return b->eqBand5FreqHz.load();
    if (paramId == eqBand5GainDb)
        return detail::clampEqGainDb(b->eqBand5GainDb.load());
    if (paramId == eqBand5Q)
        return b->eqBand5Q.load();
    if (paramId == eqBand1Type)
        return b->eqBand1Type.load();
    if (paramId == eqBand2Type)
        return b->eqBand2Type.load();
    if (paramId == eqBand3Type)
        return b->eqBand3Type.load();
    if (paramId == eqBand4Type)
        return b->eqBand4Type.load();
    if (paramId == eqBand5Type)
        return b->eqBand5Type.load();
    if (paramId == eqActiveBandCount)
        return b->eqActiveBandCount.load();
    if (paramId == eqBand6FreqHz)
        return b->eqBand6FreqHz.load();
    if (paramId == eqBand6GainDb)
        return detail::clampEqGainDb(b->eqBand6GainDb.load());
    if (paramId == eqBand6Q)
        return b->eqBand6Q.load();
    if (paramId == eqBand7FreqHz)
        return b->eqBand7FreqHz.load();
    if (paramId == eqBand7GainDb)
        return detail::clampEqGainDb(b->eqBand7GainDb.load());
    if (paramId == eqBand7Q)
        return b->eqBand7Q.load();
    if (paramId == eqBand8FreqHz)
        return b->eqBand8FreqHz.load();
    if (paramId == eqBand8GainDb)
        return detail::clampEqGainDb(b->eqBand8GainDb.load());
    if (paramId == eqBand8Q)
        return b->eqBand8Q.load();
    if (paramId == eqBand9FreqHz)
        return b->eqBand9FreqHz.load();
    if (paramId == eqBand9GainDb)
        return detail::clampEqGainDb(b->eqBand9GainDb.load());
    if (paramId == eqBand9Q)
        return b->eqBand9Q.load();
    if (paramId == eqBand10FreqHz)
        return b->eqBand10FreqHz.load();
    if (paramId == eqBand10GainDb)
        return detail::clampEqGainDb(b->eqBand10GainDb.load());
    if (paramId == eqBand10Q)
        return b->eqBand10Q.load();
    if (paramId == eqBand6Type)
        return b->eqBand6Type.load();
    if (paramId == eqBand7Type)
        return b->eqBand7Type.load();
    if (paramId == eqBand8Type)
        return b->eqBand8Type.load();
    if (paramId == eqBand9Type)
        return b->eqBand9Type.load();
    if (paramId == eqBand10Type)
        return b->eqBand10Type.load();
    if (paramId == eqBand1Slope)
        return detail::clampEqSlopeDbPerOct(b->eqBand1Slope.load());
    if (paramId == eqBand2Slope)
        return detail::clampEqSlopeDbPerOct(b->eqBand2Slope.load());
    if (paramId == eqBand3Slope)
        return detail::clampEqSlopeDbPerOct(b->eqBand3Slope.load());
    if (paramId == eqBand4Slope)
        return detail::clampEqSlopeDbPerOct(b->eqBand4Slope.load());
    if (paramId == eqBand5Slope)
        return detail::clampEqSlopeDbPerOct(b->eqBand5Slope.load());
    if (paramId == eqBand6Slope)
        return detail::clampEqSlopeDbPerOct(b->eqBand6Slope.load());
    if (paramId == eqBand7Slope)
        return detail::clampEqSlopeDbPerOct(b->eqBand7Slope.load());
    if (paramId == eqBand8Slope)
        return detail::clampEqSlopeDbPerOct(b->eqBand8Slope.load());
    if (paramId == eqBand9Slope)
        return detail::clampEqSlopeDbPerOct(b->eqBand9Slope.load());
    if (paramId == eqBand10Slope)
        return detail::clampEqSlopeDbPerOct(b->eqBand10Slope.load());
    return 0.0f;
}

void ModuleParamsRuntime::setFloat(uint32_t slotId, const juce::String& paramId, float value)
{
    detail::ModuleSlotBlock* b = findOrNull(slotId);
    if (b == nullptr)
        return;

    using namespace razumov::params;
    if (paramId == micAmount)
        b->micAmount.store(value);
    else if (paramId == gainDb)
        b->gainDb.store(value);
    else if (paramId == lowpassHz)
        b->lowpassHz.store(value);
    else if (paramId == deessCrossoverHz)
        b->deessCrossoverHz.store(value);
    else if (paramId == deessThresholdDb)
        b->deessThresholdDb.store(value);
    else if (paramId == deessRatio)
        b->deessRatio.store(value);
    else if (paramId == optoThresholdDb)
        b->optoThresholdDb.store(value);
    else if (paramId == optoRatio)
        b->optoRatio.store(value);
    else if (paramId == optoMakeupDb)
        b->optoMakeupDb.store(value);
    else if (paramId == fetThresholdDb)
        b->fetThresholdDb.store(value);
    else if (paramId == fetRatio)
        b->fetRatio.store(value);
    else if (paramId == fetMakeupDb)
        b->fetMakeupDb.store(value);
    else if (paramId == vcaThresholdDb)
        b->vcaThresholdDb.store(value);
    else if (paramId == vcaRatio)
        b->vcaRatio.store(value);
    else if (paramId == vcaMakeupDb)
        b->vcaMakeupDb.store(value);
    else if (paramId == exciterDrive)
        b->exciterDrive.store(value);
    else if (paramId == exciterMix)
        b->exciterMix.store(value);
    else if (paramId == spectralMix)
        b->spectralMix.store(value);
    else if (paramId == spectralThresholdDb)
        b->spectralThresholdDb.store(value);
    else if (paramId == spectralRatio)
        b->spectralRatio.store(value);
    else if (paramId == spectralScFreqHz)
        b->spectralScFreqHz.store(value);
    else if (paramId == spectralScQ)
        b->spectralScQ.store(value);
    else if (paramId == spectralAttackMs)
        b->spectralAttackMs.store(value);
    else if (paramId == spectralReleaseMs)
        b->spectralReleaseMs.store(value);
    else if (paramId == eqBand1FreqHz)
        b->eqBand1FreqHz.store(value);
    else if (paramId == eqBand1GainDb)
        b->eqBand1GainDb.store(detail::clampEqGainDb(value));
    else if (paramId == eqBand1Q)
        b->eqBand1Q.store(value);
    else if (paramId == eqBand2FreqHz)
        b->eqBand2FreqHz.store(value);
    else if (paramId == eqBand2GainDb)
        b->eqBand2GainDb.store(detail::clampEqGainDb(value));
    else if (paramId == eqBand2Q)
        b->eqBand2Q.store(value);
    else if (paramId == eqBand3FreqHz)
        b->eqBand3FreqHz.store(value);
    else if (paramId == eqBand3GainDb)
        b->eqBand3GainDb.store(detail::clampEqGainDb(value));
    else if (paramId == eqBand3Q)
        b->eqBand3Q.store(value);
    else if (paramId == eqBand4FreqHz)
        b->eqBand4FreqHz.store(value);
    else if (paramId == eqBand4GainDb)
        b->eqBand4GainDb.store(detail::clampEqGainDb(value));
    else if (paramId == eqBand4Q)
        b->eqBand4Q.store(value);
    else if (paramId == eqBand5FreqHz)
        b->eqBand5FreqHz.store(value);
    else if (paramId == eqBand5GainDb)
        b->eqBand5GainDb.store(detail::clampEqGainDb(value));
    else if (paramId == eqBand5Q)
        b->eqBand5Q.store(value);
    else if (paramId == eqBand1Type)
        b->eqBand1Type.store(value);
    else if (paramId == eqBand2Type)
        b->eqBand2Type.store(value);
    else if (paramId == eqBand3Type)
        b->eqBand3Type.store(value);
    else if (paramId == eqBand4Type)
        b->eqBand4Type.store(value);
    else if (paramId == eqBand5Type)
        b->eqBand5Type.store(value);
    else if (paramId == eqActiveBandCount)
        b->eqActiveBandCount.store(value);
    else if (paramId == eqBand6FreqHz)
        b->eqBand6FreqHz.store(value);
    else if (paramId == eqBand6GainDb)
        b->eqBand6GainDb.store(detail::clampEqGainDb(value));
    else if (paramId == eqBand6Q)
        b->eqBand6Q.store(value);
    else if (paramId == eqBand7FreqHz)
        b->eqBand7FreqHz.store(value);
    else if (paramId == eqBand7GainDb)
        b->eqBand7GainDb.store(detail::clampEqGainDb(value));
    else if (paramId == eqBand7Q)
        b->eqBand7Q.store(value);
    else if (paramId == eqBand8FreqHz)
        b->eqBand8FreqHz.store(value);
    else if (paramId == eqBand8GainDb)
        b->eqBand8GainDb.store(detail::clampEqGainDb(value));
    else if (paramId == eqBand8Q)
        b->eqBand8Q.store(value);
    else if (paramId == eqBand9FreqHz)
        b->eqBand9FreqHz.store(value);
    else if (paramId == eqBand9GainDb)
        b->eqBand9GainDb.store(detail::clampEqGainDb(value));
    else if (paramId == eqBand9Q)
        b->eqBand9Q.store(value);
    else if (paramId == eqBand10FreqHz)
        b->eqBand10FreqHz.store(value);
    else if (paramId == eqBand10GainDb)
        b->eqBand10GainDb.store(detail::clampEqGainDb(value));
    else if (paramId == eqBand10Q)
        b->eqBand10Q.store(value);
    else if (paramId == eqBand6Type)
        b->eqBand6Type.store(value);
    else if (paramId == eqBand7Type)
        b->eqBand7Type.store(value);
    else if (paramId == eqBand8Type)
        b->eqBand8Type.store(value);
    else if (paramId == eqBand9Type)
        b->eqBand9Type.store(value);
    else if (paramId == eqBand10Type)
        b->eqBand10Type.store(value);
    else if (paramId == eqBand1Slope)
        b->eqBand1Slope.store(detail::clampEqSlopeDbPerOct(value));
    else if (paramId == eqBand2Slope)
        b->eqBand2Slope.store(detail::clampEqSlopeDbPerOct(value));
    else if (paramId == eqBand3Slope)
        b->eqBand3Slope.store(detail::clampEqSlopeDbPerOct(value));
    else if (paramId == eqBand4Slope)
        b->eqBand4Slope.store(detail::clampEqSlopeDbPerOct(value));
    else if (paramId == eqBand5Slope)
        b->eqBand5Slope.store(detail::clampEqSlopeDbPerOct(value));
    else if (paramId == eqBand6Slope)
        b->eqBand6Slope.store(detail::clampEqSlopeDbPerOct(value));
    else if (paramId == eqBand7Slope)
        b->eqBand7Slope.store(detail::clampEqSlopeDbPerOct(value));
    else if (paramId == eqBand8Slope)
        b->eqBand8Slope.store(detail::clampEqSlopeDbPerOct(value));
    else if (paramId == eqBand9Slope)
        b->eqBand9Slope.store(detail::clampEqSlopeDbPerOct(value));
    else if (paramId == eqBand10Slope)
        b->eqBand10Slope.store(detail::clampEqSlopeDbPerOct(value));
}

bool ModuleParamsRuntime::getBool(uint32_t slotId, const juce::String& paramId) const
{
    const detail::ModuleSlotBlock* b = findOrNull(slotId);
    if (b == nullptr)
        return false;

    using namespace razumov::params;
    if (paramId == micBypass)
        return b->micBypass.load() > 0.5f;
    if (paramId == spectralBypass)
        return b->spectralBypass.load() > 0.5f;
    if (paramId == eqBypass)
        return b->eqBypass.load() > 0.5f;
    return false;
}

void ModuleParamsRuntime::setBool(uint32_t slotId, const juce::String& paramId, bool value)
{
    detail::ModuleSlotBlock* b = findOrNull(slotId);
    if (b == nullptr)
        return;

    using namespace razumov::params;
    const float v = value ? 1.0f : 0.0f;
    if (paramId == micBypass)
        b->micBypass.store(v);
    else if (paramId == spectralBypass)
        b->spectralBypass.store(v);
    else if (paramId == eqBypass)
        b->eqBypass.store(v);
}

void ModuleParamsRuntime::seedAllSlotsWithSameParams(const Phase3RealtimeParams& p)
{
    for (size_t i = 0; i < blocks_.size(); ++i)
        if (blocks_[i] != nullptr)
            storeFromPhase3(*blocks_[i], p);
}

void ModuleParamsRuntime::copySlotParamsFromTo(uint32_t fromSlotId, uint32_t toSlotId)
{
    const detail::ModuleSlotBlock* src = findOrNull(fromSlotId);
    if (src == nullptr)
        return;
    ensureSlotWithDefaults(toSlotId);
    detail::ModuleSlotBlock* dst = findOrNull(toSlotId);
    if (dst == nullptr)
        return;
    detail::copyModuleSlotBlockState(*src, *dst);
}

juce::ValueTree ModuleParamsRuntime::toValueTree() const
{
    juce::ValueTree root(moduleParamsTreeType);
    for (size_t i = 0; i < slotIds_.size(); ++i)
    {
        const uint32_t sid = slotIds_[i];
        const auto* b = blocks_[i].get();
        if (b == nullptr)
            continue;

        juce::ValueTree slot("Slot");
        using namespace razumov::params;
        slot.setProperty("slotId", (int) sid, nullptr);
        slot.setProperty(micBypass, b->micBypass.load(), nullptr);
        slot.setProperty(micAmount, b->micAmount.load(), nullptr);
        slot.setProperty(gainDb, b->gainDb.load(), nullptr);
        slot.setProperty(lowpassHz, b->lowpassHz.load(), nullptr);
        slot.setProperty(deessCrossoverHz, b->deessCrossoverHz.load(), nullptr);
        slot.setProperty(deessThresholdDb, b->deessThresholdDb.load(), nullptr);
        slot.setProperty(deessRatio, b->deessRatio.load(), nullptr);
        slot.setProperty(optoThresholdDb, b->optoThresholdDb.load(), nullptr);
        slot.setProperty(optoRatio, b->optoRatio.load(), nullptr);
        slot.setProperty(optoMakeupDb, b->optoMakeupDb.load(), nullptr);
        slot.setProperty(fetThresholdDb, b->fetThresholdDb.load(), nullptr);
        slot.setProperty(fetRatio, b->fetRatio.load(), nullptr);
        slot.setProperty(fetMakeupDb, b->fetMakeupDb.load(), nullptr);
        slot.setProperty(vcaThresholdDb, b->vcaThresholdDb.load(), nullptr);
        slot.setProperty(vcaRatio, b->vcaRatio.load(), nullptr);
        slot.setProperty(vcaMakeupDb, b->vcaMakeupDb.load(), nullptr);
        slot.setProperty(exciterDrive, b->exciterDrive.load(), nullptr);
        slot.setProperty(exciterMix, b->exciterMix.load(), nullptr);
        slot.setProperty(spectralBypass, b->spectralBypass.load(), nullptr);
        slot.setProperty(spectralMix, b->spectralMix.load(), nullptr);
        slot.setProperty(spectralThresholdDb, b->spectralThresholdDb.load(), nullptr);
        slot.setProperty(spectralRatio, b->spectralRatio.load(), nullptr);
        slot.setProperty(spectralScFreqHz, b->spectralScFreqHz.load(), nullptr);
        slot.setProperty(spectralScQ, b->spectralScQ.load(), nullptr);
        slot.setProperty(spectralAttackMs, b->spectralAttackMs.load(), nullptr);
        slot.setProperty(spectralReleaseMs, b->spectralReleaseMs.load(), nullptr);
        slot.setProperty(eqBypass, b->eqBypass.load(), nullptr);
        slot.setProperty(eqBand1FreqHz, b->eqBand1FreqHz.load(), nullptr);
        slot.setProperty(eqBand1GainDb, b->eqBand1GainDb.load(), nullptr);
        slot.setProperty(eqBand1Q, b->eqBand1Q.load(), nullptr);
        slot.setProperty(eqBand2FreqHz, b->eqBand2FreqHz.load(), nullptr);
        slot.setProperty(eqBand2GainDb, b->eqBand2GainDb.load(), nullptr);
        slot.setProperty(eqBand2Q, b->eqBand2Q.load(), nullptr);
        slot.setProperty(eqBand3FreqHz, b->eqBand3FreqHz.load(), nullptr);
        slot.setProperty(eqBand3GainDb, b->eqBand3GainDb.load(), nullptr);
        slot.setProperty(eqBand3Q, b->eqBand3Q.load(), nullptr);
        slot.setProperty(eqBand4FreqHz, b->eqBand4FreqHz.load(), nullptr);
        slot.setProperty(eqBand4GainDb, b->eqBand4GainDb.load(), nullptr);
        slot.setProperty(eqBand4Q, b->eqBand4Q.load(), nullptr);
        slot.setProperty(eqBand5FreqHz, b->eqBand5FreqHz.load(), nullptr);
        slot.setProperty(eqBand5GainDb, b->eqBand5GainDb.load(), nullptr);
        slot.setProperty(eqBand5Q, b->eqBand5Q.load(), nullptr);
        slot.setProperty(eqBand1Type, b->eqBand1Type.load(), nullptr);
        slot.setProperty(eqBand2Type, b->eqBand2Type.load(), nullptr);
        slot.setProperty(eqBand3Type, b->eqBand3Type.load(), nullptr);
        slot.setProperty(eqBand4Type, b->eqBand4Type.load(), nullptr);
        slot.setProperty(eqBand5Type, b->eqBand5Type.load(), nullptr);
        slot.setProperty(eqActiveBandCount, b->eqActiveBandCount.load(), nullptr);
        slot.setProperty(eqBand6FreqHz, b->eqBand6FreqHz.load(), nullptr);
        slot.setProperty(eqBand6GainDb, b->eqBand6GainDb.load(), nullptr);
        slot.setProperty(eqBand6Q, b->eqBand6Q.load(), nullptr);
        slot.setProperty(eqBand7FreqHz, b->eqBand7FreqHz.load(), nullptr);
        slot.setProperty(eqBand7GainDb, b->eqBand7GainDb.load(), nullptr);
        slot.setProperty(eqBand7Q, b->eqBand7Q.load(), nullptr);
        slot.setProperty(eqBand8FreqHz, b->eqBand8FreqHz.load(), nullptr);
        slot.setProperty(eqBand8GainDb, b->eqBand8GainDb.load(), nullptr);
        slot.setProperty(eqBand8Q, b->eqBand8Q.load(), nullptr);
        slot.setProperty(eqBand9FreqHz, b->eqBand9FreqHz.load(), nullptr);
        slot.setProperty(eqBand9GainDb, b->eqBand9GainDb.load(), nullptr);
        slot.setProperty(eqBand9Q, b->eqBand9Q.load(), nullptr);
        slot.setProperty(eqBand10FreqHz, b->eqBand10FreqHz.load(), nullptr);
        slot.setProperty(eqBand10GainDb, b->eqBand10GainDb.load(), nullptr);
        slot.setProperty(eqBand10Q, b->eqBand10Q.load(), nullptr);
        slot.setProperty(eqBand6Type, b->eqBand6Type.load(), nullptr);
        slot.setProperty(eqBand7Type, b->eqBand7Type.load(), nullptr);
        slot.setProperty(eqBand8Type, b->eqBand8Type.load(), nullptr);
        slot.setProperty(eqBand9Type, b->eqBand9Type.load(), nullptr);
        slot.setProperty(eqBand10Type, b->eqBand10Type.load(), nullptr);
        slot.setProperty(eqBand1Slope, b->eqBand1Slope.load(), nullptr);
        slot.setProperty(eqBand2Slope, b->eqBand2Slope.load(), nullptr);
        slot.setProperty(eqBand3Slope, b->eqBand3Slope.load(), nullptr);
        slot.setProperty(eqBand4Slope, b->eqBand4Slope.load(), nullptr);
        slot.setProperty(eqBand5Slope, b->eqBand5Slope.load(), nullptr);
        slot.setProperty(eqBand6Slope, b->eqBand6Slope.load(), nullptr);
        slot.setProperty(eqBand7Slope, b->eqBand7Slope.load(), nullptr);
        slot.setProperty(eqBand8Slope, b->eqBand8Slope.load(), nullptr);
        slot.setProperty(eqBand9Slope, b->eqBand9Slope.load(), nullptr);
        slot.setProperty(eqBand10Slope, b->eqBand10Slope.load(), nullptr);
        root.appendChild(slot, nullptr);
    }

    juce::ValueTree mr("MacroRouting");
    for (int i = 0; i < 8; ++i)
    {
        juce::ValueTree row("Macro");
        row.setProperty("i", i, nullptr);
        row.setProperty("slot", (int) macroTargetSlot_[(size_t) i].load(std::memory_order_relaxed), nullptr);
        row.setProperty("param", (int) macroTargetParam_[(size_t) i].load(std::memory_order_relaxed), nullptr);
        row.setProperty("name", macroDisplayNames_[(size_t) i], nullptr);
        mr.appendChild(row, nullptr);
    }
    root.appendChild(mr, nullptr);
    return root;
}

void ModuleParamsRuntime::fromValueTree(const juce::ValueTree& v)
{
    if (!v.isValid() || v.getType() != moduleParamsTreeType)
        return;

    for (int i = 0; i < v.getNumChildren(); ++i)
    {
        auto slot = v.getChild(i);
        if (!slot.isValid() || slot.getType().toString() != "Slot")
            continue;
        const int sid = (int) slot.getProperty("slotId", 0);
        if (sid <= 0)
            continue;
        ensureSlotWithDefaults((uint32_t) sid);
        detail::ModuleSlotBlock* b = findOrNull((uint32_t) sid);
        if (b == nullptr)
            continue;

        using namespace razumov::params;
        auto gf = [&](const char* id, float defV) {
            return (float) slot.getProperty(id, defV);
        };
        b->micBypass.store(gf(micBypass, 0.0f));
        b->micAmount.store(gf(micAmount, 1.0f));
        b->gainDb.store(gf(gainDb, 0.0f));
        b->lowpassHz.store(gf(lowpassHz, 20000.0f));
        b->deessCrossoverHz.store(gf(deessCrossoverHz, 6000.0f));
        b->deessThresholdDb.store(gf(deessThresholdDb, -20.0f));
        b->deessRatio.store(gf(deessRatio, 3.0f));
        b->optoThresholdDb.store(gf(optoThresholdDb, -12.0f));
        b->optoRatio.store(gf(optoRatio, 3.0f));
        b->optoMakeupDb.store(gf(optoMakeupDb, 0.0f));
        b->fetThresholdDb.store(gf(fetThresholdDb, -12.0f));
        b->fetRatio.store(gf(fetRatio, 4.0f));
        b->fetMakeupDb.store(gf(fetMakeupDb, 0.0f));
        b->vcaThresholdDb.store(gf(vcaThresholdDb, -12.0f));
        b->vcaRatio.store(gf(vcaRatio, 2.5f));
        b->vcaMakeupDb.store(gf(vcaMakeupDb, 0.0f));
        b->exciterDrive.store(gf(exciterDrive, 1.5f));
        b->exciterMix.store(gf(exciterMix, 0.15f));
        b->spectralBypass.store(gf(spectralBypass, 0.0f));
        b->spectralMix.store(gf(spectralMix, 0.75f));
        b->spectralThresholdDb.store(gf(spectralThresholdDb, -24.0f));
        b->spectralRatio.store(gf(spectralRatio, 3.0f));
        b->spectralScFreqHz.store(gf(spectralScFreqHz, 2000.0f));
        b->spectralScQ.store(gf(spectralScQ, 1.2f));
        b->spectralAttackMs.store(gf(spectralAttackMs, 50.0f));
        b->spectralReleaseMs.store(gf(spectralReleaseMs, 250.0f));
        b->eqBypass.store(gf(eqBypass, 0.0f));
        b->eqBand1FreqHz.store(gf(eqBand1FreqHz, 120.0f));
        b->eqBand1GainDb.store(detail::clampEqGainDb(gf(eqBand1GainDb, 0.0f)));
        b->eqBand1Q.store(gf(eqBand1Q, 1.0f));
        b->eqBand2FreqHz.store(gf(eqBand2FreqHz, 400.0f));
        b->eqBand2GainDb.store(detail::clampEqGainDb(gf(eqBand2GainDb, 0.0f)));
        b->eqBand2Q.store(gf(eqBand2Q, 1.0f));
        b->eqBand3FreqHz.store(gf(eqBand3FreqHz, 2500.0f));
        b->eqBand3GainDb.store(detail::clampEqGainDb(gf(eqBand3GainDb, 0.0f)));
        b->eqBand3Q.store(gf(eqBand3Q, 1.0f));
        b->eqBand4FreqHz.store(gf(eqBand4FreqHz, 7000.0f));
        b->eqBand4GainDb.store(detail::clampEqGainDb(gf(eqBand4GainDb, 0.0f)));
        b->eqBand4Q.store(gf(eqBand4Q, 1.0f));
        b->eqBand5FreqHz.store(gf(eqBand5FreqHz, 10000.0f));
        b->eqBand5GainDb.store(detail::clampEqGainDb(gf(eqBand5GainDb, 0.0f)));
        b->eqBand5Q.store(gf(eqBand5Q, 1.0f));
        b->eqBand1Type.store(gf(eqBand1Type, 0.0f));
        b->eqBand2Type.store(gf(eqBand2Type, 0.0f));
        b->eqBand3Type.store(gf(eqBand3Type, 0.0f));
        b->eqBand4Type.store(gf(eqBand4Type, 0.0f));
        b->eqBand5Type.store(gf(eqBand5Type, 0.0f));
        b->eqActiveBandCount.store(gf(eqActiveBandCount, 5.0f));
        b->eqBand6FreqHz.store(gf(eqBand6FreqHz, 12000.0f));
        b->eqBand6GainDb.store(detail::clampEqGainDb(gf(eqBand6GainDb, 0.0f)));
        b->eqBand6Q.store(gf(eqBand6Q, 1.0f));
        b->eqBand7FreqHz.store(gf(eqBand7FreqHz, 13000.0f));
        b->eqBand7GainDb.store(detail::clampEqGainDb(gf(eqBand7GainDb, 0.0f)));
        b->eqBand7Q.store(gf(eqBand7Q, 1.0f));
        b->eqBand8FreqHz.store(gf(eqBand8FreqHz, 15000.0f));
        b->eqBand8GainDb.store(detail::clampEqGainDb(gf(eqBand8GainDb, 0.0f)));
        b->eqBand8Q.store(gf(eqBand8Q, 1.0f));
        b->eqBand9FreqHz.store(gf(eqBand9FreqHz, 16000.0f));
        b->eqBand9GainDb.store(detail::clampEqGainDb(gf(eqBand9GainDb, 0.0f)));
        b->eqBand9Q.store(gf(eqBand9Q, 1.0f));
        b->eqBand10FreqHz.store(gf(eqBand10FreqHz, 18000.0f));
        b->eqBand10GainDb.store(detail::clampEqGainDb(gf(eqBand10GainDb, 0.0f)));
        b->eqBand10Q.store(gf(eqBand10Q, 1.0f));
        b->eqBand6Type.store(gf(eqBand6Type, 0.0f));
        b->eqBand7Type.store(gf(eqBand7Type, 0.0f));
        b->eqBand8Type.store(gf(eqBand8Type, 0.0f));
        b->eqBand9Type.store(gf(eqBand9Type, 0.0f));
        b->eqBand10Type.store(gf(eqBand10Type, 0.0f));
        b->eqBand1Slope.store(detail::clampEqSlopeDbPerOct(gf(eqBand1Slope, 48.0f)));
        b->eqBand2Slope.store(detail::clampEqSlopeDbPerOct(gf(eqBand2Slope, 48.0f)));
        b->eqBand3Slope.store(detail::clampEqSlopeDbPerOct(gf(eqBand3Slope, 48.0f)));
        b->eqBand4Slope.store(detail::clampEqSlopeDbPerOct(gf(eqBand4Slope, 48.0f)));
        b->eqBand5Slope.store(detail::clampEqSlopeDbPerOct(gf(eqBand5Slope, 48.0f)));
        b->eqBand6Slope.store(detail::clampEqSlopeDbPerOct(gf(eqBand6Slope, 48.0f)));
        b->eqBand7Slope.store(detail::clampEqSlopeDbPerOct(gf(eqBand7Slope, 48.0f)));
        b->eqBand8Slope.store(detail::clampEqSlopeDbPerOct(gf(eqBand8Slope, 48.0f)));
        b->eqBand9Slope.store(detail::clampEqSlopeDbPerOct(gf(eqBand9Slope, 48.0f)));
        b->eqBand10Slope.store(detail::clampEqSlopeDbPerOct(gf(eqBand10Slope, 48.0f)));
    }

    juce::ValueTree mr = v.getChildWithName("MacroRouting");
    if (mr.isValid())
    {
        for (int c = 0; c < mr.getNumChildren(); ++c)
        {
            auto row = mr.getChild(c);
            if (!row.isValid() || row.getType().toString() != "Macro")
                continue;
            const int idx = (int) row.getProperty("i", -1);
            if (idx < 0 || idx > 7)
                continue;
            macroTargetSlot_[(size_t) idx].store((uint32_t)(int) row.getProperty("slot", 0), std::memory_order_relaxed);
            macroTargetParam_[(size_t) idx].store((uint32_t)(int) row.getProperty("param", 0), std::memory_order_relaxed);
            const juce::String nm = row.getProperty("name").toString();
            if (nm.isNotEmpty())
                macroDisplayNames_[(size_t) idx] = nm;
        }
    }
}

void ModuleParamsRuntime::setMacroTarget(int macroIndex, uint32_t slotId, MacroTargetParam param) noexcept
{
    if (macroIndex < 0 || macroIndex > 7)
        return;
    macroTargetSlot_[(size_t) macroIndex].store(slotId, std::memory_order_release);
    macroTargetParam_[(size_t) macroIndex].store((uint32_t) param, std::memory_order_release);
}

void ModuleParamsRuntime::clearMacroTarget(int macroIndex) noexcept
{
    if (macroIndex < 0 || macroIndex > 7)
        return;
    macroTargetSlot_[(size_t) macroIndex].store(0u, std::memory_order_release);
    macroTargetParam_[(size_t) macroIndex].store((uint32_t) MacroTargetParam::None, std::memory_order_release);
}

uint32_t ModuleParamsRuntime::getMacroTargetSlot(int macroIndex) const noexcept
{
    if (macroIndex < 0 || macroIndex > 7)
        return 0;
    return macroTargetSlot_[(size_t) macroIndex].load(std::memory_order_acquire);
}

MacroTargetParam ModuleParamsRuntime::getMacroTargetParam(int macroIndex) const noexcept
{
    if (macroIndex < 0 || macroIndex > 7)
        return MacroTargetParam::None;
    return (MacroTargetParam) macroTargetParam_[(size_t) macroIndex].load(std::memory_order_acquire);
}

int ModuleParamsRuntime::findMacroIndexForTarget(uint32_t slotId, MacroTargetParam param) const noexcept
{
    if (slotId == 0 || param == MacroTargetParam::None)
        return -1;
    for (int i = 0; i < 8; ++i)
    {
        if (macroTargetSlot_[(size_t) i].load(std::memory_order_acquire) != slotId)
            continue;
        if ((MacroTargetParam) macroTargetParam_[(size_t) i].load(std::memory_order_acquire) == param)
            return i;
    }
    return -1;
}

void ModuleParamsRuntime::setMacroDisplayName(int macroIndex, juce::String name)
{
    if (macroIndex < 0 || macroIndex > 7)
        return;
    name = name.trim();
    if (name.isEmpty())
        return;
    macroDisplayNames_[(size_t) macroIndex] = name;
}

juce::String ModuleParamsRuntime::getMacroDisplayName(int macroIndex) const
{
    if (macroIndex < 0 || macroIndex > 7)
        return {};
    return macroDisplayNames_[(size_t) macroIndex];
}

} // namespace razumov::params
