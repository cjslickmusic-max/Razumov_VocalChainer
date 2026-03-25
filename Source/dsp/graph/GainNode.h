#pragma once

#include "AudioNode.h"
#include <juce_dsp/juce_dsp.h>

namespace razumov::graph
{

/** Простое стерео-усиление с сглаживанием. */
class GainNode final : public AudioNode
{
public:
    GainNode() = default;

    explicit GainNode(float linearGain) { setLinearGain(linearGain); }

    GainNode* asGain() noexcept override { return this; }

    void setLinearGain(float g) noexcept
    {
        targetGain_ = juce::jlimit(0.0f, 4.0f, g);
    }

    float getLinearGain() const noexcept { return targetGain_; }

    void prepare(double sampleRate, int maxBlockSize, int numChannels) override
    {
        juce::ignoreUnused(maxBlockSize, numChannels);
        gainSmoother_.reset(sampleRate, 0.02);
        gainSmoother_.setCurrentAndTargetValue(targetGain_);
    }

    void reset() override { gainSmoother_.setCurrentAndTargetValue(targetGain_); }

    void process(juce::AudioBuffer<float>& buffer) override
    {
        const int n = buffer.getNumSamples();
        gainSmoother_.setTargetValue(targetGain_);

        for (int i = 0; i < n; ++i)
        {
            const float g = gainSmoother_.getNextValue();
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
                buffer.getWritePointer(ch)[i] *= g;
        }
    }

private:
    float targetGain_ { 1.0f };
    juce::LinearSmoothedValue<float> gainSmoother_ { 1.0f };
};

} // namespace razumov::graph
