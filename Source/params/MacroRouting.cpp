#include "MacroRouting.h"
#include "ParamIDs.h"
#include "dsp/graph/AudioNode.h"

#include <cmath>

namespace razumov::params
{
namespace
{
constexpr float lerp(float a, float b, float t) noexcept
{
    return a + (b - a) * t;
}

float clamp01(float x) noexcept
{
    return juce::jlimit(0.0f, 1.0f, x);
}
} // namespace

MacroTargetParam macroTargetParamFromParamId(const juce::String& paramId) noexcept
{
    using namespace razumov::params;
    if (paramId == micAmount)
        return MacroTargetParam::MicAmount;
    if (paramId == gainDb)
        return MacroTargetParam::GainDb;
    if (paramId == lowpassHz)
        return MacroTargetParam::LowpassHz;
    if (paramId == deessCrossoverHz)
        return MacroTargetParam::DeessCrossoverHz;
    if (paramId == deessThresholdDb)
        return MacroTargetParam::DeessThresholdDb;
    if (paramId == deessRatio)
        return MacroTargetParam::DeessRatio;
    if (paramId == optoThresholdDb)
        return MacroTargetParam::OptoThresholdDb;
    if (paramId == optoRatio)
        return MacroTargetParam::OptoRatio;
    if (paramId == optoMakeupDb)
        return MacroTargetParam::OptoMakeupDb;
    if (paramId == fetThresholdDb)
        return MacroTargetParam::FetThresholdDb;
    if (paramId == fetRatio)
        return MacroTargetParam::FetRatio;
    if (paramId == fetMakeupDb)
        return MacroTargetParam::FetMakeupDb;
    if (paramId == vcaThresholdDb)
        return MacroTargetParam::VcaThresholdDb;
    if (paramId == vcaRatio)
        return MacroTargetParam::VcaRatio;
    if (paramId == vcaMakeupDb)
        return MacroTargetParam::VcaMakeupDb;
    if (paramId == exciterDrive)
        return MacroTargetParam::ExciterDrive;
    if (paramId == exciterMix)
        return MacroTargetParam::ExciterMix;
    if (paramId == spectralMix)
        return MacroTargetParam::SpectralMix;
    if (paramId == spectralThresholdDb)
        return MacroTargetParam::SpectralThresholdDb;
    if (paramId == spectralRatio)
        return MacroTargetParam::SpectralRatio;
    return MacroTargetParam::None;
}

const char* macroTargetParamToParamId(MacroTargetParam p) noexcept
{
    using namespace razumov::params;
    switch (p)
    {
        case MacroTargetParam::MicAmount:
            return micAmount;
        case MacroTargetParam::GainDb:
            return gainDb;
        case MacroTargetParam::LowpassHz:
            return lowpassHz;
        case MacroTargetParam::DeessCrossoverHz:
            return deessCrossoverHz;
        case MacroTargetParam::DeessThresholdDb:
            return deessThresholdDb;
        case MacroTargetParam::DeessRatio:
            return deessRatio;
        case MacroTargetParam::OptoThresholdDb:
            return optoThresholdDb;
        case MacroTargetParam::OptoRatio:
            return optoRatio;
        case MacroTargetParam::OptoMakeupDb:
            return optoMakeupDb;
        case MacroTargetParam::FetThresholdDb:
            return fetThresholdDb;
        case MacroTargetParam::FetRatio:
            return fetRatio;
        case MacroTargetParam::FetMakeupDb:
            return fetMakeupDb;
        case MacroTargetParam::VcaThresholdDb:
            return vcaThresholdDb;
        case MacroTargetParam::VcaRatio:
            return vcaRatio;
        case MacroTargetParam::VcaMakeupDb:
            return vcaMakeupDb;
        case MacroTargetParam::ExciterDrive:
            return exciterDrive;
        case MacroTargetParam::ExciterMix:
            return exciterMix;
        case MacroTargetParam::SpectralMix:
            return spectralMix;
        case MacroTargetParam::SpectralThresholdDb:
            return spectralThresholdDb;
        case MacroTargetParam::SpectralRatio:
            return spectralRatio;
        default:
            return nullptr;
    }
}

bool moduleKindSupportsMacroTarget(razumov::graph::AudioNodeKind kind, MacroTargetParam p) noexcept
{
    using namespace razumov::graph;
    switch (kind)
    {
        case AudioNodeKind::MicCorrection:
            return p == MacroTargetParam::MicAmount;
        case AudioNodeKind::Gain:
            return p == MacroTargetParam::GainDb;
        case AudioNodeKind::Filter:
            return p == MacroTargetParam::LowpassHz;
        case AudioNodeKind::Deesser:
            return p == MacroTargetParam::DeessCrossoverHz || p == MacroTargetParam::DeessThresholdDb
                   || p == MacroTargetParam::DeessRatio;
        case AudioNodeKind::OptoCompressor:
            return p == MacroTargetParam::OptoThresholdDb || p == MacroTargetParam::OptoRatio
                   || p == MacroTargetParam::OptoMakeupDb;
        case AudioNodeKind::FetCompressor:
            return p == MacroTargetParam::FetThresholdDb || p == MacroTargetParam::FetRatio
                   || p == MacroTargetParam::FetMakeupDb;
        case AudioNodeKind::VcaCompressor:
            return p == MacroTargetParam::VcaThresholdDb || p == MacroTargetParam::VcaRatio
                   || p == MacroTargetParam::VcaMakeupDb;
        case AudioNodeKind::Exciter:
            return p == MacroTargetParam::ExciterDrive || p == MacroTargetParam::ExciterMix;
        case AudioNodeKind::SpectralCompressor:
            return p == MacroTargetParam::SpectralMix || p == MacroTargetParam::SpectralThresholdDb
                   || p == MacroTargetParam::SpectralRatio;
        default:
            return false;
    }
}

float macro01ToParamValue(MacroTargetParam kind, float macro01) noexcept
{
    const float t = clamp01(macro01);
    switch (kind)
    {
        case MacroTargetParam::MicAmount:
            return lerp(0.0f, 1.0f, t);
        case MacroTargetParam::GainDb:
            return lerp(kGainModuleDbRangeMin, kGainModuleDbRangeMax, t);
        case MacroTargetParam::LowpassHz:
            return lerp(400.0f, 20000.0f, t);
        case MacroTargetParam::DeessCrossoverHz:
            return lerp(3000.0f, 10000.0f, t);
        case MacroTargetParam::DeessThresholdDb:
            return lerp(-60.0f, 0.0f, t);
        case MacroTargetParam::DeessRatio:
            return lerp(1.0f, 10.0f, t);
        case MacroTargetParam::OptoThresholdDb:
            return lerp(-60.0f, 0.0f, t);
        case MacroTargetParam::OptoRatio:
            return lerp(1.0f, 20.0f, t);
        case MacroTargetParam::OptoMakeupDb:
            return lerp(0.0f, 24.0f, t);
        case MacroTargetParam::FetThresholdDb:
            return lerp(-60.0f, 0.0f, t);
        case MacroTargetParam::FetRatio:
            return lerp(1.0f, 20.0f, t);
        case MacroTargetParam::FetMakeupDb:
            return lerp(0.0f, 24.0f, t);
        case MacroTargetParam::VcaThresholdDb:
            return lerp(-60.0f, 0.0f, t);
        case MacroTargetParam::VcaRatio:
            return lerp(1.0f, 20.0f, t);
        case MacroTargetParam::VcaMakeupDb:
            return lerp(0.0f, 24.0f, t);
        case MacroTargetParam::ExciterDrive:
            return lerp(0.1f, 8.0f, t);
        case MacroTargetParam::ExciterMix:
            return lerp(0.0f, 1.0f, t);
        case MacroTargetParam::SpectralMix:
            return lerp(0.0f, 1.0f, t);
        case MacroTargetParam::SpectralThresholdDb:
            return lerp(-60.0f, 0.0f, t);
        case MacroTargetParam::SpectralRatio:
            return lerp(1.0f, 20.0f, t);
        default:
            return 0.0f;
    }
}

float paramValueToMacro01(MacroTargetParam kind, float value) noexcept
{
    auto inv = [](float v, float lo, float hi) {
        if (hi <= lo)
            return 0.5f;
        return clamp01((v - lo) / (hi - lo));
    };

    switch (kind)
    {
        case MacroTargetParam::MicAmount:
            return inv(value, 0.0f, 1.0f);
        case MacroTargetParam::GainDb:
            return inv(value, kGainModuleDbRangeMin, kGainModuleDbRangeMax);
        case MacroTargetParam::LowpassHz:
            return inv(value, 400.0f, 20000.0f);
        case MacroTargetParam::DeessCrossoverHz:
            return inv(value, 3000.0f, 10000.0f);
        case MacroTargetParam::DeessThresholdDb:
            return inv(value, -60.0f, 0.0f);
        case MacroTargetParam::DeessRatio:
            return inv(value, 1.0f, 10.0f);
        case MacroTargetParam::OptoThresholdDb:
            return inv(value, -60.0f, 0.0f);
        case MacroTargetParam::OptoRatio:
            return inv(value, 1.0f, 20.0f);
        case MacroTargetParam::OptoMakeupDb:
            return inv(value, 0.0f, 24.0f);
        case MacroTargetParam::FetThresholdDb:
            return inv(value, -60.0f, 0.0f);
        case MacroTargetParam::FetRatio:
            return inv(value, 1.0f, 20.0f);
        case MacroTargetParam::FetMakeupDb:
            return inv(value, 0.0f, 24.0f);
        case MacroTargetParam::VcaThresholdDb:
            return inv(value, -60.0f, 0.0f);
        case MacroTargetParam::VcaRatio:
            return inv(value, 1.0f, 20.0f);
        case MacroTargetParam::VcaMakeupDb:
            return inv(value, 0.0f, 24.0f);
        case MacroTargetParam::ExciterDrive:
            return inv(value, 0.1f, 8.0f);
        case MacroTargetParam::ExciterMix:
            return inv(value, 0.0f, 1.0f);
        case MacroTargetParam::SpectralMix:
            return inv(value, 0.0f, 1.0f);
        case MacroTargetParam::SpectralThresholdDb:
            return inv(value, -60.0f, 0.0f);
        case MacroTargetParam::SpectralRatio:
            return inv(value, 1.0f, 20.0f);
        default:
            return 0.5f;
    }
}

void applyMacroFullRangeToPhase3(Phase3RealtimeParams& p, MacroTargetParam kind, float macro01) noexcept
{
    const float v = macro01ToParamValue(kind, macro01);
    switch (kind)
    {
        case MacroTargetParam::MicAmount:
            p.micAmount = v;
            break;
        case MacroTargetParam::GainDb:
            p.gainLinear = juce::Decibels::decibelsToGain(v);
            break;
        case MacroTargetParam::LowpassHz:
            p.lowpassHz = v;
            break;
        case MacroTargetParam::DeessCrossoverHz:
            p.deessCrossoverHz = v;
            break;
        case MacroTargetParam::DeessThresholdDb:
            p.deessThresholdDb = v;
            break;
        case MacroTargetParam::DeessRatio:
            p.deessRatio = v;
            break;
        case MacroTargetParam::OptoThresholdDb:
            p.optoThresholdDb = v;
            break;
        case MacroTargetParam::OptoRatio:
            p.optoRatio = v;
            break;
        case MacroTargetParam::OptoMakeupDb:
            p.optoMakeupDb = v;
            break;
        case MacroTargetParam::FetThresholdDb:
            p.fetThresholdDb = v;
            break;
        case MacroTargetParam::FetRatio:
            p.fetRatio = v;
            break;
        case MacroTargetParam::FetMakeupDb:
            p.fetMakeupDb = v;
            break;
        case MacroTargetParam::VcaThresholdDb:
            p.vcaThresholdDb = v;
            break;
        case MacroTargetParam::VcaRatio:
            p.vcaRatio = v;
            break;
        case MacroTargetParam::VcaMakeupDb:
            p.vcaMakeupDb = v;
            break;
        case MacroTargetParam::ExciterDrive:
            p.exciterDrive = v;
            break;
        case MacroTargetParam::ExciterMix:
            p.exciterMix = v;
            break;
        case MacroTargetParam::SpectralMix:
            p.spectralMix = v;
            break;
        case MacroTargetParam::SpectralThresholdDb:
            p.spectralThresholdDb = v;
            break;
        case MacroTargetParam::SpectralRatio:
            p.spectralRatio = v;
            break;
        default:
            break;
    }
}

} // namespace razumov::params
