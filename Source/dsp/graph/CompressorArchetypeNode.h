#pragma once

#include "AudioNode.h"
#include <atomic>
#include <juce_dsp/juce_dsp.h>

namespace razumov::graph
{

/** Один архетип на экземпляр: Opto / FET / VCA — разные тайминги и смысл ручек. */
class CompressorArchetypeNode final : public AudioNode
{
public:
    enum class Archetype
    {
        Opto,
        Fet,
        Vca
    };

    explicit CompressorArchetypeNode(Archetype a);

    AudioNodeKind getKind() const noexcept override;

    void setThresholdDb(float db) noexcept { thresholdDb_ = juce::jlimit(-60.0f, 0.0f, db); }
    void setRatio(float r) noexcept { ratio_ = juce::jlimit(1.0f, 20.0f, r); }
    void setMakeupDb(float db) noexcept { makeupDb_ = juce::jlimit(0.0f, 24.0f, db); }

    void prepare(double sampleRate, int maxBlockSize, int numChannels) override;
    void reset() override;
    void process(juce::AudioBuffer<float>& buffer) override;

    float getGainReductionDbForUi() const noexcept override;

private:
    void applyArchetypeTiming();

    Archetype archetype_;
    juce::dsp::Compressor<float> comp_;

    float thresholdDb_ { -12.0f };
    float ratio_ { 3.0f };
    float makeupDb_ { 0.0f };

    double sampleRate_ { 44100.0 };
    float smoothedGrDb_ { 0.f };
    std::atomic<float> grMeterDbAtomic_ { 0.f };
};

} // namespace razumov::graph
