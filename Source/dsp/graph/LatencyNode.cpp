#include "LatencyNode.h"

namespace razumov::graph
{

void LatencyNode::prepare(double sampleRate, int maxBlockSize, int numChannels)
{
    juce::ignoreUnused(sampleRate);
    writePos_ = 0;
    lineSize_ = latency_ + maxBlockSize + 8;
    line_.setSize(numChannels, lineSize_);
    line_.clear();
}

void LatencyNode::reset()
{
    writePos_ = 0;
    line_.clear();
}

void LatencyNode::process(juce::AudioBuffer<float>& buffer)
{
    if (latency_ <= 0)
        return;

    const int n = buffer.getNumSamples();
    const int ch = buffer.getNumChannels();
    const int sz = lineSize_;

    for (int i = 0; i < n; ++i)
    {
        for (int c = 0; c < ch; ++c)
        {
            const float x = buffer.getSample(c, i);
            const int wi = writePos_ % sz;
            const int ri = writePos_ - latency_;

            float y = 0.0f;
            if (ri >= 0)
            {
                const int idx = ri % sz;
                y = line_.getSample(c, idx);
            }

            line_.setSample(c, wi, x);
            buffer.setSample(c, i, y);
        }

        ++writePos_;
    }
}

} // namespace razumov::graph
