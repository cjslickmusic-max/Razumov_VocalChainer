#pragma once

#include "AudioNode.h"
#include "ISpectrumSource.h"
#include "SpectrumTap.h"

#include <array>
#include <juce_dsp/juce_dsp.h>

namespace razumov::params
{
struct Phase3RealtimeParams;
}

namespace razumov::graph
{

/** 4-band parametric peaking EQ (stereo linked), спектр с входа (до EQ). */
class ParametricEqNode final : public AudioNode, public ISpectrumSource
{
public:
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
    std::array<std::array<IIRFilter, 2>, 4> bands_ {};

    std::array<float, 4> tgtFreq_ { 120.f, 400.f, 2500.f, 7000.f };
    std::array<float, 4> tgtGainDb_ {};
    std::array<float, 4> tgtQ_ { 1.f, 1.f, 1.f, 1.f };
    bool tgtBypass_ { false };

    std::array<float, 4> smoothFreq_ { 120.f, 400.f, 2500.f, 7000.f };
    std::array<float, 4> smoothGainDb_ {};
    std::array<float, 4> smoothQ_ { 1.f, 1.f, 1.f, 1.f };
    bool smoothBypass_ { false };
};

} // namespace razumov::graph
