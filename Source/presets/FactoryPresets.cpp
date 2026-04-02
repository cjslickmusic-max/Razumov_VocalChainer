#include "FactoryPresets.h"
#include "params/ParamIDs.h"

#include <juce_dsp/juce_dsp.h>

namespace razumov::presets
{
namespace
{

void setFloatParam(juce::AudioProcessorValueTreeState& apvts, const juce::String& id, float value)
{
    if (auto* p = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter(id)))
        p->setValueNotifyingHost(p->getNormalisableRange().convertTo0to1(value));
}

void setChoiceParam(juce::AudioProcessorValueTreeState& apvts, const juce::String& id, int index)
{
    if (auto* p = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter(id)))
    {
        const int n = p->choices.size();
        if (n <= 0)
            return;
        const int i = juce::jlimit(0, n - 1, index);
        p->setValueNotifyingHost(p->convertTo0to1((float) i));
    }
}

void applyPresetGlobalsNeutralMacros(juce::AudioProcessorValueTreeState& apvts)
{
    using namespace razumov::params;
    setChoiceParam(apvts, micProfile, 0);
    setFloatParam(apvts, macroGlue, 0.5f);
    setFloatParam(apvts, macroAir, 0.5f);
    setFloatParam(apvts, macroSibilance, 0.5f);
    setFloatParam(apvts, macroPresence, 0.5f);
    setFloatParam(apvts, macroPunch, 0.5f);
    setFloatParam(apvts, macroBody, 0.5f);
    setFloatParam(apvts, macroSmooth, 0.5f);
    setFloatParam(apvts, macroDensity, 0.5f);
}

razumov::params::Phase3RealtimeParams buildDefaultModule()
{
    using namespace razumov::params;
    Phase3RealtimeParams p {};
    p.micBypass = false;
    p.micAmount = 1.0f;
    p.gainLinear = juce::Decibels::decibelsToGain(0.0f);
    p.lowpassHz = 20000.0f;
    p.deessCrossoverHz = 6000.0f;
    p.deessThresholdDb = -20.0f;
    p.deessRatio = 3.0f;
    p.optoThresholdDb = -12.0f;
    p.optoRatio = 3.0f;
    p.optoMakeupDb = 0.0f;
    p.fetThresholdDb = -12.0f;
    p.fetRatio = 4.0f;
    p.fetMakeupDb = 0.0f;
    p.vcaThresholdDb = -12.0f;
    p.vcaRatio = 2.5f;
    p.vcaMakeupDb = 0.0f;
    p.exciterDrive = 1.5f;
    p.exciterMix = 0.15f;
    p.spectralBypass = false;
    p.spectralMix = 0.75f;
    p.spectralThresholdDb = -24.0f;
    p.spectralRatio = 3.0f;
    p.spectralScFreqHz = 2000.0f;
    p.spectralScQ = 1.2f;
    p.spectralAttackMs = 50.0f;
    p.spectralReleaseMs = 250.0f;
    return p;
}

razumov::params::Phase3RealtimeParams buildCleanVocalModule()
{
    auto p = buildDefaultModule();
    p.gainLinear = juce::Decibels::decibelsToGain(1.5f);
    p.lowpassHz = 18000.0f;
    p.deessCrossoverHz = 7000.0f;
    p.deessThresholdDb = -28.0f;
    p.deessRatio = 4.0f;
    p.optoThresholdDb = -18.0f;
    p.optoRatio = 2.5f;
    p.fetThresholdDb = -20.0f;
    p.fetRatio = 3.0f;
    p.vcaThresholdDb = -18.0f;
    p.vcaRatio = 2.0f;
    p.exciterDrive = 1.2f;
    p.exciterMix = 0.08f;
    return p;
}

razumov::params::Phase3RealtimeParams buildAggressiveModule()
{
    auto p = buildDefaultModule();
    p.gainLinear = juce::Decibels::decibelsToGain(3.0f);
    p.deessThresholdDb = -14.0f;
    p.deessRatio = 5.5f;
    p.optoThresholdDb = -8.0f;
    p.optoRatio = 5.0f;
    p.optoMakeupDb = 2.0f;
    p.fetThresholdDb = -10.0f;
    p.fetRatio = 6.0f;
    p.fetMakeupDb = 1.5f;
    p.vcaThresholdDb = -9.0f;
    p.vcaRatio = 4.0f;
    p.vcaMakeupDb = 1.0f;
    p.exciterDrive = 2.8f;
    p.exciterMix = 0.28f;
    return p;
}

razumov::params::Phase3RealtimeParams buildBroadcastModule()
{
    auto p = buildDefaultModule();
    p.gainLinear = juce::Decibels::decibelsToGain(4.5f);
    p.lowpassHz = 16500.0f;
    p.deessCrossoverHz = 6500.0f;
    p.deessThresholdDb = -22.0f;
    p.deessRatio = 6.0f;
    p.optoThresholdDb = -14.0f;
    p.optoRatio = 4.0f;
    p.optoMakeupDb = 3.0f;
    p.fetThresholdDb = -14.0f;
    p.fetRatio = 5.0f;
    p.fetMakeupDb = 2.5f;
    p.vcaThresholdDb = -11.0f;
    p.vcaRatio = 3.5f;
    p.vcaMakeupDb = 2.0f;
    p.exciterDrive = 1.8f;
    p.exciterMix = 0.12f;
    p.spectralBypass = false;
    return p;
}

} // namespace

int getNumFactoryPresets() noexcept
{
    return 4;
}

const char* getFactoryPresetName(int index) noexcept
{
    static const char* names[] = {
        "Default",
        "Clean vocal",
        "Aggressive",
        "Broadcast",
    };
    if (index < 0 || index >= getNumFactoryPresets())
        return "";
    return names[index];
}

void applyFactoryPresetGlobals(int index, juce::AudioProcessorValueTreeState& apvts)
{
    juce::ignoreUnused(index);
    applyPresetGlobalsNeutralMacros(apvts);
}

razumov::params::Phase3RealtimeParams buildFactoryPresetModulePhase3(int index)
{
    switch (index)
    {
        case 0:
            return buildDefaultModule();
        case 1:
            return buildCleanVocalModule();
        case 2:
            return buildAggressiveModule();
        case 3:
            return buildBroadcastModule();
        default:
            return buildDefaultModule();
    }
}

} // namespace razumov::presets
