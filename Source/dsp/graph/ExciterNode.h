#pragma once

#include "AudioNode.h"
#include <juce_dsp/juce_dsp.h>

namespace razumov::graph
{

/** Простой exciter: смесь dry и сатурации tanh. */
class ExciterNode final : public AudioNode
{
public:
    ExciterNode() = default;

    AudioNodeKind getKind() const noexcept override { return AudioNodeKind::Exciter; }

    void setDrive(float d) noexcept { drive_ = juce::jlimit(0.1f, 8.0f, d); }
    void setMix(float m) noexcept { mix_ = juce::jlimit(0.0f, 1.0f, m); }

    void prepare(double sampleRate, int maxBlockSize, int numChannels) override;
    void reset() override;
    void process(juce::AudioBuffer<float>& buffer) override;

private:
    float drive_ { 1.5f };
    float mix_ { 0.15f };
};

} // namespace razumov::graph
