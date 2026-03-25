#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "dsp/graph/GraphPlanFactory.h"
#include "params/ParamIDs.h"
#include "params/ParameterLayout.h"

RazumovVocalChainAudioProcessor::RazumovVocalChainAudioProcessor()
    : AudioProcessor(
        BusesProperties()
            .withInput("Input", juce::AudioChannelSet::stereo(), true)
            .withOutput("Output", juce::AudioChannelSet::stereo(), true))
    , apvts(*this, nullptr, "PARAMS", razumov::params::createParameterLayout())
{
    graphEngine_.setLatencyCallback([this](int latency) { setLatencySamples(latency); });
}

RazumovVocalChainAudioProcessor::~RazumovVocalChainAudioProcessor() = default;

void RazumovVocalChainAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    if (!graphSeeded_)
    {
        graphEngine_.submitPlan(razumov::graph::GraphPlanFactory::makeSerialGainAndWideFilter(sampleRate));
        graphSeeded_ = true;
    }

    graphEngine_.prepare(sampleRate, samplesPerBlock, 2);
}

void RazumovVocalChainAudioProcessor::releaseResources()
{
    graphEngine_.releaseResources();
    graphSeeded_ = false;
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

    const float gainDb = apvts.getRawParameterValue(razumov::params::gainDb)->load();
    const float lowpassHz = apvts.getRawParameterValue(razumov::params::lowpassHz)->load();
    graphEngine_.applyLiveParameters(juce::Decibels::decibelsToGain(gainDb), lowpassHz);
    graphEngine_.process(buffer);
}

juce::AudioProcessorEditor* RazumovVocalChainAudioProcessor::createEditor()
{
    return new RazumovVocalChainAudioProcessorEditor(*this);
}

int RazumovVocalChainAudioProcessor::getNumPrograms()
{
    return 1;
}

int RazumovVocalChainAudioProcessor::getCurrentProgram()
{
    return 0;
}

void RazumovVocalChainAudioProcessor::setCurrentProgram(int index)
{
    juce::ignoreUnused(index);
}

const juce::String RazumovVocalChainAudioProcessor::getProgramName(int index)
{
    juce::ignoreUnused(index);
    return {};
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
            apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RazumovVocalChainAudioProcessor();
}
