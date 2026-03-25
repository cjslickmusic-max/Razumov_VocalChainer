#include "PluginProcessor.h"
#include "PluginEditor.h"

RazumovVocalChainAudioProcessor::RazumovVocalChainAudioProcessor()
    : AudioProcessor(
        BusesProperties()
            .withInput("Input", juce::AudioChannelSet::stereo(), true)
            .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
}

RazumovVocalChainAudioProcessor::~RazumovVocalChainAudioProcessor() = default;

void RazumovVocalChainAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(sampleRate, samplesPerBlock);
}

void RazumovVocalChainAudioProcessor::releaseResources() {}

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

    juce::ignoreUnused(buffer);
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
    juce::ignoreUnused(destData);
}

void RazumovVocalChainAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    juce::ignoreUnused(data, sizeInBytes);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RazumovVocalChainAudioProcessor();
}
