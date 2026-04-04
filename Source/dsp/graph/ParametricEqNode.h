#pragma once

#include "AudioNode.h"
#include "ISpectrumSource.h"
#include "SpectrumTap.h"
#include "dsp/eq/EqBandShapes.h"

#include <array>
#include <juce_dsp/juce_dsp.h>

namespace razumov::params
{
struct Phase3RealtimeParams;
}

namespace razumov::graph
{

/** 5-band EQ (stereo linked), RBJ/SVF-style per-band types; spectrum = input before EQ. */
class ParametricEqNode final : public AudioNode, public ISpectrumSource
{
public:
    static constexpr int kNumBands = 5;

    ParametricEqNode() = default;

    AudioNodeKind getKind() const noexcept override { return AudioNodeKind::ParametricEq; }

    ISpectrumSource* asSpectrumSource() noexcept override { return this; }

    void prepare(double sampleRate, int maxBlockSize, int numChannels) override;
    void reset() override;
    void process(juce::AudioBuffer<float>& buffer) override;

    void applyPhase3(const razumov::params::Phase3RealtimeParams& p) noexcept;

    void copySpectrum256(float* dst) const noexcept override;

private:
    using IIRFilter = juce::dsp::IIR::Filter<float>;
    using Coefficients = juce::dsp::IIR::Coefficients<float>;

    void smoothStep() noexcept;
    void updateBandCoeffs(int bandIndex) noexcept;
    void processOneChannel(float* data, int numSamples, int channelIndex) noexcept;

    double sampleRate_ { 44100.0 };
    int maxBlockSize_ { 512 };

    SpectrumTap spectrumTap_;

    /** [band][L/R] */
    std::array<std::array<IIRFilter, 2>, kNumBands> bands_ {};

    std::array<float, kNumBands> tgtFreq_ { 120.f, 400.f, 2500.f, 7000.f, 10000.f };
    std::array<float, kNumBands> tgtGainDb_ {};
    std::array<float, kNumBands> tgtQ_ { 1.f, 1.f, 1.f, 1.f, 1.f };
    std::array<float, kNumBands> tgtType_ {}; /** 0..5 EqBandType */
    bool tgtBypass_ { false };

    std::array<float, kNumBands> smoothFreq_ { 120.f, 400.f, 2500.f, 7000.f, 10000.f };
    std::array<float, kNumBands> smoothGainDb_ {};
    std::array<float, kNumBands> smoothQ_ { 1.f, 1.f, 1.f, 1.f, 1.f };
    std::array<float, kNumBands> smoothType_ {};
    bool smoothBypass_ { false };
};

} // namespace razumov::graph
