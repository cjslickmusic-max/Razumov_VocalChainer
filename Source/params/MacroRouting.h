#pragma once

#include "Phase3RealtimeParams.h"
#include "dsp/graph/AudioNode.h"
#include <juce_core/juce_core.h>

namespace razumov::params
{

/** Per-macro assignment to one module float parameter (full-range 0...1 -> param min...max). */
enum class MacroTargetParam : uint32_t
{
    None = 0,
    MicAmount,
    GainDb,
    LowpassHz,
    DeessCrossoverHz,
    DeessThresholdDb,
    DeessRatio,
    OptoThresholdDb,
    OptoRatio,
    OptoMakeupDb,
    FetThresholdDb,
    FetRatio,
    FetMakeupDb,
    VcaThresholdDb,
    VcaRatio,
    VcaMakeupDb,
    ExciterDrive,
    ExciterMix,
    SpectralMix,
    SpectralThresholdDb,
    SpectralRatio
};

MacroTargetParam macroTargetParamFromParamId(const juce::String& paramId) noexcept;
const char* macroTargetParamToParamId(MacroTargetParam p) noexcept;

bool moduleKindSupportsMacroTarget(razumov::graph::AudioNodeKind kind, MacroTargetParam p) noexcept;

void applyMacroFullRangeToPhase3(Phase3RealtimeParams& p, MacroTargetParam kind, float macro01) noexcept;

float macro01ToParamValue(MacroTargetParam kind, float macro01) noexcept;
float paramValueToMacro01(MacroTargetParam kind, float value) noexcept;

} // namespace razumov::params
