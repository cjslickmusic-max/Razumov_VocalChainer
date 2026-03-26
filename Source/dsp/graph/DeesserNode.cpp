#include "DeesserNode.h"

namespace razumov::graph
{

void DeesserNode::prepare(double sampleRate, int maxBlockSize, int numChannels)
{
    juce::ignoreUnused(numChannels);
    sampleRate_ = sampleRate;

    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) juce::jmax(1, maxBlockSize), 2 };
    lr_.prepare(spec);
    lr_.setCutoffFrequency(crossoverHz_);

    comp_.prepare(spec);
    comp_.setAttack(1.0f);
    comp_.setRelease(50.0f);
    comp_.setThreshold(thresholdDb_);
    comp_.setRatio(ratio_);

    reset();
}

void DeesserNode::reset()
{
    lr_.reset();
    comp_.reset();
}

void DeesserNode::process(juce::AudioBuffer<float>& buffer)
{
    lr_.setCutoffFrequency(crossoverHz_);
    comp_.setThreshold(thresholdDb_);
    comp_.setRatio(ratio_);

    const int n = buffer.getNumSamples();
    const int ch = buffer.getNumChannels();

    for (int i = 0; i < n; ++i)
    {
        for (int c = 0; c < ch; ++c)
        {
            const float x = buffer.getSample(c, i);
            float low = 0.0f;
            float high = 0.0f;
            lr_.processSample(c, x, low, high);
            const float highComp = comp_.processSample(c, high);
            buffer.setSample(c, i, low + highComp);
        }
    }
}

} // namespace razumov::graph
