#include "ExciterNode.h"

#include <cmath>

namespace razumov::graph
{

void ExciterNode::prepare(double sampleRate, int maxBlockSize, int numChannels)
{
    juce::ignoreUnused(sampleRate, maxBlockSize, numChannels);
}

void ExciterNode::reset() {}

void ExciterNode::process(juce::AudioBuffer<float>& buffer)
{
    const int n = buffer.getNumSamples();
    const int ch = buffer.getNumChannels();

    for (int i = 0; i < n; ++i)
    {
        for (int c = 0; c < ch; ++c)
        {
            const float x = buffer.getSample(c, i);
            const float wet = std::tanh(x * drive_);
            const float y = x * (1.0f - mix_) + wet * mix_;
            buffer.setSample(c, i, y);
        }
    }
}

} // namespace razumov::graph
