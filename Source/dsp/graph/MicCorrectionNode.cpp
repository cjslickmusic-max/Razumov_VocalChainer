#include "MicCorrectionNode.h"

namespace razumov::graph
{

void MicCorrectionNode::prepare(double sampleRate, int maxBlockSize, int numChannels)
{
    juce::ignoreUnused(sampleRate, maxBlockSize, numChannels);
}

void MicCorrectionNode::reset() {}

void MicCorrectionNode::process(juce::AudioBuffer<float>& buffer)
{
    juce::ignoreUnused(buffer);
    // Pass-through: буфер не трогаем. TODO: EQ/IR/профиль; тогда bypass_/amount_ и getLatencySamples().
}

} // namespace razumov::graph
