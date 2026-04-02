#pragma once

#include "AudioNode.h"
#include "ISpectrumSource.h"
#include "SpectrumTap.h"

namespace razumov::graph
{

/** Только FFT-спектр входа (pass-through), отдельный модуль как ReSpectrum. */
class SpectrumAnalyzerNode final : public AudioNode, public ISpectrumSource
{
public:
    SpectrumAnalyzerNode() = default;

    AudioNodeKind getKind() const noexcept override { return AudioNodeKind::SpectrumAnalyzer; }

    ISpectrumSource* asSpectrumSource() noexcept override { return this; }

    void prepare(double sampleRate, int maxBlockSize, int numChannels) override;
    void reset() override;
    void process(juce::AudioBuffer<float>& buffer) override;

    void copySpectrum256(float* dst) const noexcept override;

private:
    SpectrumTap spectrumTap_;
};

} // namespace razumov::graph
