#pragma once

#include "AudioNode.h"
#include <juce_dsp/juce_dsp.h>

namespace razumov::graph
{

/** Стерео low-pass IIR; при очень высокой частоте — почти bypass. */
class FilterNode final : public AudioNode
{
public:
    FilterNode() = default;

    void setCutoffHz(float hz) noexcept { cutoffHz_ = juce::jmax(20.0f, hz); }
    float getCutoffHz() const noexcept { return cutoffHz_; }

    void prepare(double sampleRate, int maxBlockSize, int numChannels) override;
    void reset() override;
    void process(juce::AudioBuffer<float>& buffer) override;

private:
    void updateCoeffs();

    double sampleRate_ { 44100.0 };
    float cutoffHz_ { 20000.0f };

    using IIRFilter = juce::dsp::IIR::Filter<float>;
    using Coefficients = juce::dsp::IIR::Coefficients<float>;
    juce::dsp::ProcessorDuplicator<IIRFilter, Coefficients> iir_;
};

} // namespace razumov::graph
