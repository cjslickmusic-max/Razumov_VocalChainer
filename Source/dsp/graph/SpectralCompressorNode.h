#pragma once

#include "AudioNode.h"
#include "MergeDelayPad.h"

#include <juce_dsp/juce_dsp.h>

#include <memory>
#include <vector>

namespace razumov::graph
{

/**
 * Спектральный компрессор: STFT, динамика по модулю с сохранением фазы входа (без лишнего
 * фазового искажения от динамики). Dry задержан на ту же задержку, что и wet (MergeDelayPad),
 * затем dry/wet mix — выравнивание как у параллельных веток с PDC.
 */
class SpectralCompressorNode final : public AudioNode
{
public:
    SpectralCompressorNode();
    ~SpectralCompressorNode() override;

    SpectralCompressorNode(const SpectralCompressorNode&) = delete;
    SpectralCompressorNode& operator=(const SpectralCompressorNode&) = delete;
    SpectralCompressorNode(SpectralCompressorNode&&) = delete;
    SpectralCompressorNode& operator=(SpectralCompressorNode&&) = delete;

    AudioNodeKind getKind() const noexcept override { return AudioNodeKind::SpectralCompressor; }

    int getLatencySamples() const noexcept override;

    void setBypass(bool b) noexcept { bypass_ = b; }
    void setMix(float m) noexcept { mix_ = juce::jlimit(0.0f, 1.0f, m); }
    void setThresholdDb(float dB) noexcept { thresholdDb_ = dB; }
    void setRatio(float r) noexcept { ratio_ = juce::jmax(1.0f, r); }

    void prepare(double sampleRate, int maxBlockSize, int numChannels) override;
    void reset() override;
    void process(juce::AudioBuffer<float>& buffer) override;

private:
    struct ChannelData;

    static constexpr int fftOrder_ = 10;
    static constexpr int fftSize_ = 1 << fftOrder_;
    static constexpr int hop_ = fftSize_ / 2;
    static constexpr int kLatencySamples_ = fftSize_;

    bool bypass_ { false };
    float mix_ { 0.75f };
    float thresholdDb_ { -24.0f };
    float ratio_ { 3.0f };

    int numChannels_ { 2 };

    std::unique_ptr<juce::dsp::FFT> fft_;
    std::vector<float> window_;

    std::unique_ptr<MergeDelayPad> dryPad_;
    std::vector<std::unique_ptr<ChannelData>> channels_;

    juce::AudioBuffer<float> dryScratch_;
};

} // namespace razumov::graph
