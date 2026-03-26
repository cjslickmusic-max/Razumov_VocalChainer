#pragma once

#include "AudioNode.h"
#include <juce_dsp/juce_dsp.h>

namespace razumov::graph
{

/** De-esser: LR-кроссовер + компрессия только верхней полосы, сумма с низом. */
class DeesserNode final : public AudioNode
{
public:
    DeesserNode() = default;

    AudioNodeKind getKind() const noexcept override { return AudioNodeKind::Deesser; }

    void setCrossoverHz(float hz) noexcept { crossoverHz_ = juce::jlimit(2000.0f, 12000.0f, hz); }
    void setThresholdDb(float db) noexcept { thresholdDb_ = juce::jlimit(-60.0f, 0.0f, db); }
    void setRatio(float r) noexcept { ratio_ = juce::jlimit(1.0f, 20.0f, r); }

    void prepare(double sampleRate, int maxBlockSize, int numChannels) override;
    void reset() override;
    void process(juce::AudioBuffer<float>& buffer) override;

private:
    juce::dsp::LinkwitzRileyFilter<float> lr_;
    juce::dsp::Compressor<float> comp_;

    float crossoverHz_ { 6000.0f };
    float thresholdDb_ { -20.0f };
    float ratio_ { 3.0f };
    double sampleRate_ { 44100.0 };
};

} // namespace razumov::graph
