#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "dsp/graph/GraphPlanFactory.h"
#include "params/ParamIDs.h"
#include "params/ParameterLayout.h"
#include "params/Phase3RealtimeParams.h"
#include "presets/FactoryPresets.h"

#include <juce_dsp/juce_dsp.h>

namespace
{

int getStartupChainIndex(juce::AudioProcessorValueTreeState& apvts)
{
    if (auto* p = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter(razumov::params::chainProfile)))
        return p->getIndex();
    return 0;
}

float bipolarMacro01(float normalized01) noexcept
{
    return (normalized01 - 0.5f) * 2.0f;
}

razumov::params::Phase3RealtimeParams buildPhase3RealtimeParams(juce::AudioProcessorValueTreeState& apvts)
{
    using namespace razumov::params;

    Phase3RealtimeParams p;
    p.micBypass = apvts.getRawParameterValue(micBypass)->load() > 0.5f;
    p.micAmount = apvts.getRawParameterValue(micAmount)->load();
    p.gainLinear = juce::Decibels::decibelsToGain(apvts.getRawParameterValue(gainDb)->load());
    p.lowpassHz = apvts.getRawParameterValue(lowpassHz)->load();

    p.deessCrossoverHz = apvts.getRawParameterValue(deessCrossoverHz)->load();
    p.deessThresholdDb = apvts.getRawParameterValue(deessThresholdDb)->load();
    p.deessRatio = apvts.getRawParameterValue(deessRatio)->load();

    p.optoThresholdDb = apvts.getRawParameterValue(optoThresholdDb)->load();
    p.optoRatio = apvts.getRawParameterValue(optoRatio)->load();
    p.optoMakeupDb = apvts.getRawParameterValue(optoMakeupDb)->load();

    p.fetThresholdDb = apvts.getRawParameterValue(fetThresholdDb)->load();
    p.fetRatio = apvts.getRawParameterValue(fetRatio)->load();
    p.fetMakeupDb = apvts.getRawParameterValue(fetMakeupDb)->load();

    p.vcaThresholdDb = apvts.getRawParameterValue(vcaThresholdDb)->load();
    p.vcaRatio = apvts.getRawParameterValue(vcaRatio)->load();
    p.vcaMakeupDb = apvts.getRawParameterValue(vcaMakeupDb)->load();

    p.exciterDrive = apvts.getRawParameterValue(exciterDrive)->load();
    p.exciterMix = apvts.getRawParameterValue(exciterMix)->load();

    p.spectralBypass = apvts.getRawParameterValue(spectralBypass)->load() > 0.5f;
    p.spectralMix = apvts.getRawParameterValue(spectralMix)->load();
    p.spectralThresholdDb = apvts.getRawParameterValue(spectralThresholdDb)->load();
    p.spectralRatio = apvts.getRawParameterValue(spectralRatio)->load();

    const float bGlue = bipolarMacro01(apvts.getRawParameterValue(macroGlue)->load());
    p.optoMakeupDb = juce::jlimit(0.0f, 24.0f, p.optoMakeupDb + bGlue * 4.0f);
    p.fetMakeupDb = juce::jlimit(0.0f, 24.0f, p.fetMakeupDb + bGlue * 3.0f);
    p.vcaMakeupDb = juce::jlimit(0.0f, 24.0f, p.vcaMakeupDb + bGlue * 3.0f);

    const float bAir = bipolarMacro01(apvts.getRawParameterValue(macroAir)->load());
    p.lowpassHz = juce::jlimit(400.0f, 20000.0f, p.lowpassHz + bAir * 4500.0f);
    p.exciterMix = juce::jlimit(0.0f, 1.0f, p.exciterMix + bAir * 0.12f);

    const float bSib = bipolarMacro01(apvts.getRawParameterValue(macroSibilance)->load());
    p.deessThresholdDb = juce::jlimit(-60.0f, 0.0f, p.deessThresholdDb + bSib * -10.0f);

    const float bPr = bipolarMacro01(apvts.getRawParameterValue(macroPresence)->load());
    p.exciterDrive = juce::jlimit(0.1f, 8.0f, p.exciterDrive + bPr * 2.0f);
    p.gainLinear *= juce::Decibels::decibelsToGain(bPr * 2.0f);

    return p;
}

} // namespace

RazumovVocalChainAudioProcessor::RazumovVocalChainAudioProcessor()
    : AudioProcessor(
        BusesProperties()
            .withInput("Input", juce::AudioChannelSet::stereo(), true)
            .withOutput("Output", juce::AudioChannelSet::stereo(), true))
    , apvts(*this, nullptr, "PARAMS", razumov::params::createParameterLayout())
{
    graphEngine_.setLatencyCallback([this](int latency) { setLatencySamples(latency); });
    apvts.addParameterListener(razumov::params::chainProfile, this);
}

RazumovVocalChainAudioProcessor::~RazumovVocalChainAudioProcessor()
{
    apvts.removeParameterListener(razumov::params::chainProfile, this);
}

void RazumovVocalChainAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    lastSampleRate_ = sampleRate;
    auto plan = razumov::graph::GraphPlanFactory::makeStartupChainForIndex(getStartupChainIndex(apvts), sampleRate);
    graphEngine_.submitPlan(std::shared_ptr<razumov::graph::GraphPlan>(std::move(plan)));
    graphEngine_.prepare(sampleRate, samplesPerBlock, 2);
}

void RazumovVocalChainAudioProcessor::releaseResources()
{
    graphEngine_.releaseResources();
}

bool RazumovVocalChainAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto mainOut = layouts.getMainOutputChannelSet();
    const auto mainIn = layouts.getMainInputChannelSet();

    if (mainOut != juce::AudioChannelSet::stereo())
        return false;

    if (mainIn.isDisabled() || mainIn != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void RazumovVocalChainAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                                   juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);

    juce::ScopedNoDenormals noDenormals;

    graphEngine_.applyPhase3Parameters(buildPhase3RealtimeParams(apvts));
    graphEngine_.process(buffer);
}

juce::AudioProcessorEditor* RazumovVocalChainAudioProcessor::createEditor()
{
    return new RazumovVocalChainAudioProcessorEditor(*this);
}

int RazumovVocalChainAudioProcessor::getNumPrograms()
{
    return razumov::presets::getNumFactoryPresets();
}

int RazumovVocalChainAudioProcessor::getCurrentProgram()
{
    return currentProgram_;
}

void RazumovVocalChainAudioProcessor::setCurrentProgram(int index)
{
    applyFactoryPreset(index);
}

const juce::String RazumovVocalChainAudioProcessor::getProgramName(int index)
{
    return juce::String(razumov::presets::getFactoryPresetName(index));
}

void RazumovVocalChainAudioProcessor::applyFactoryPreset(int index)
{
    const int n = razumov::presets::getNumFactoryPresets();
    currentProgram_ = juce::jlimit(0, juce::jmax(0, n - 1), index);
    razumov::presets::applyFactoryPreset(currentProgram_, apvts);
}

void RazumovVocalChainAudioProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{
    juce::ignoreUnused(newValue);
    if (parameterID != razumov::params::chainProfile)
        return;
    juce::MessageManager::callAsync([this]() { submitGraphPlanForCurrentParameter(); });
}

void RazumovVocalChainAudioProcessor::submitGraphPlanForCurrentParameter()
{
    const int idx = getStartupChainIndex(apvts);
    auto plan = razumov::graph::GraphPlanFactory::makeStartupChainForIndex(idx, lastSampleRate_);
    graphEngine_.submitPlan(std::shared_ptr<razumov::graph::GraphPlan>(std::move(plan)));
}

void RazumovVocalChainAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

void RazumovVocalChainAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary(*xml, destData);
}

void RazumovVocalChainAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        if (xml->hasTagName(apvts.state.getType()))
        {
            apvts.replaceState(juce::ValueTree::fromXml(*xml));
            submitGraphPlanForCurrentParameter();
        }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RazumovVocalChainAudioProcessor();
}
