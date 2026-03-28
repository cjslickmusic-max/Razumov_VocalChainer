#include "MergeDelayPad.h"

#include <juce_core/juce_core.h>

namespace razumov::graph
{

void MergeDelayPad::prepare(int numChannels, int maxBlock, int maxDelay)
{
    delaySamples_ = 0;
    writePos_ = 0;
    lineSize_ = juce::jmax(16, maxDelay + maxBlock + 16);
    line_.setSize(numChannels, lineSize_);
    line_.clear();
}

void MergeDelayPad::setDelaySamples(int delaySamples)
{
    const int d = juce::jmax(0, delaySamples);
    if (d != delaySamples_)
    {
        delaySamples_ = d;
        writePos_ = 0;
        line_.clear();
    }
}

void MergeDelayPad::process(juce::AudioBuffer<float>& buf)
{
    if (delaySamples_ <= 0)
        return;

    const int n = buf.getNumSamples();
    const int ch = buf.getNumChannels();
    const int sz = lineSize_;

    for (int i = 0; i < n; ++i)
    {
        for (int c = 0; c < ch; ++c)
        {
            const float x = buf.getSample(c, i);
            const int wi = writePos_ % sz;
            const int ri = writePos_ - delaySamples_;

            float y = 0.0f;
            if (ri >= 0)
            {
                const int idx = ri % sz;
                y = line_.getSample(c, idx);
            }

            line_.setSample(c, wi, x);
            buf.setSample(c, i, y);
        }

        ++writePos_;
    }
}

} // namespace razumov::graph
