#include "FactoryPresets.h"
#include "params/ParamIDs.h"

namespace razumov::presets
{
namespace
{

void setFloatParam(juce::AudioProcessorValueTreeState& apvts, const juce::String& id, float value)
{
    if (auto* p = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter(id)))
        p->setValueNotifyingHost(p->getNormalisableRange().convertTo0to1(value));
}

void setBoolParam(juce::AudioProcessorValueTreeState& apvts, const juce::String& id, bool value)
{
    if (auto* p = apvts.getParameter(id))
        p->setValueNotifyingHost(value ? 1.0f : 0.0f);
}

void applyDefault(juce::AudioProcessorValueTreeState& apvts)
{
    using namespace razumov::params;
    setBoolParam(apvts, micBypass, false);
    setFloatParam(apvts, micAmount, 1.0f);
    setFloatParam(apvts, gainDb, 0.0f);
    setFloatParam(apvts, lowpassHz, 20000.0f);
    setFloatParam(apvts, deessCrossoverHz, 6000.0f);
    setFloatParam(apvts, deessThresholdDb, -20.0f);
    setFloatParam(apvts, deessRatio, 3.0f);
    setFloatParam(apvts, optoThresholdDb, -12.0f);
    setFloatParam(apvts, optoRatio, 3.0f);
    setFloatParam(apvts, optoMakeupDb, 0.0f);
    setFloatParam(apvts, fetThresholdDb, -12.0f);
    setFloatParam(apvts, fetRatio, 4.0f);
    setFloatParam(apvts, fetMakeupDb, 0.0f);
    setFloatParam(apvts, vcaThresholdDb, -12.0f);
    setFloatParam(apvts, vcaRatio, 2.5f);
    setFloatParam(apvts, vcaMakeupDb, 0.0f);
    setFloatParam(apvts, exciterDrive, 1.5f);
    setFloatParam(apvts, exciterMix, 0.15f);
    setBoolParam(apvts, spectralBypass, false);
    setFloatParam(apvts, spectralMix, 0.75f);
    setFloatParam(apvts, spectralThresholdDb, -24.0f);
    setFloatParam(apvts, spectralRatio, 3.0f);
    setFloatParam(apvts, macroGlue, 0.5f);
    setFloatParam(apvts, macroAir, 0.5f);
    setFloatParam(apvts, macroSibilance, 0.5f);
    setFloatParam(apvts, macroPresence, 0.5f);
}

void applyCleanVocal(juce::AudioProcessorValueTreeState& apvts)
{
    applyDefault(apvts);
    using namespace razumov::params;
    setFloatParam(apvts, gainDb, 1.5f);
    setFloatParam(apvts, lowpassHz, 18000.0f);
    setFloatParam(apvts, deessCrossoverHz, 7000.0f);
    setFloatParam(apvts, deessThresholdDb, -28.0f);
    setFloatParam(apvts, deessRatio, 4.0f);
    setFloatParam(apvts, optoThresholdDb, -18.0f);
    setFloatParam(apvts, optoRatio, 2.5f);
    setFloatParam(apvts, fetThresholdDb, -20.0f);
    setFloatParam(apvts, fetRatio, 3.0f);
    setFloatParam(apvts, vcaThresholdDb, -18.0f);
    setFloatParam(apvts, vcaRatio, 2.0f);
    setFloatParam(apvts, exciterDrive, 1.2f);
    setFloatParam(apvts, exciterMix, 0.08f);
}

void applyAggressive(juce::AudioProcessorValueTreeState& apvts)
{
    applyDefault(apvts);
    using namespace razumov::params;
    setFloatParam(apvts, gainDb, 3.0f);
    setFloatParam(apvts, deessThresholdDb, -14.0f);
    setFloatParam(apvts, deessRatio, 5.5f);
    setFloatParam(apvts, optoThresholdDb, -8.0f);
    setFloatParam(apvts, optoRatio, 5.0f);
    setFloatParam(apvts, optoMakeupDb, 2.0f);
    setFloatParam(apvts, fetThresholdDb, -10.0f);
    setFloatParam(apvts, fetRatio, 6.0f);
    setFloatParam(apvts, fetMakeupDb, 1.5f);
    setFloatParam(apvts, vcaThresholdDb, -9.0f);
    setFloatParam(apvts, vcaRatio, 4.0f);
    setFloatParam(apvts, vcaMakeupDb, 1.0f);
    setFloatParam(apvts, exciterDrive, 2.8f);
    setFloatParam(apvts, exciterMix, 0.28f);
}

void applyBroadcast(juce::AudioProcessorValueTreeState& apvts)
{
    applyDefault(apvts);
    using namespace razumov::params;
    setFloatParam(apvts, gainDb, 4.5f);
    setFloatParam(apvts, lowpassHz, 16500.0f);
    setFloatParam(apvts, deessCrossoverHz, 6500.0f);
    setFloatParam(apvts, deessThresholdDb, -22.0f);
    setFloatParam(apvts, deessRatio, 6.0f);
    setFloatParam(apvts, optoThresholdDb, -14.0f);
    setFloatParam(apvts, optoRatio, 4.0f);
    setFloatParam(apvts, optoMakeupDb, 3.0f);
    setFloatParam(apvts, fetThresholdDb, -14.0f);
    setFloatParam(apvts, fetRatio, 5.0f);
    setFloatParam(apvts, fetMakeupDb, 2.5f);
    setFloatParam(apvts, vcaThresholdDb, -11.0f);
    setFloatParam(apvts, vcaRatio, 3.5f);
    setFloatParam(apvts, vcaMakeupDb, 2.0f);
    setFloatParam(apvts, exciterDrive, 1.8f);
    setFloatParam(apvts, exciterMix, 0.12f);
    setBoolParam(apvts, spectralBypass, false);
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

void applyFactoryPreset(int index, juce::AudioProcessorValueTreeState& apvts)
{
    switch (index)
    {
        case 0:
            applyDefault(apvts);
            break;
        case 1:
            applyCleanVocal(apvts);
            break;
        case 2:
            applyAggressive(apvts);
            break;
        case 3:
            applyBroadcast(apvts);
            break;
        default:
            applyDefault(apvts);
            break;
    }
}

} // namespace razumov::presets
