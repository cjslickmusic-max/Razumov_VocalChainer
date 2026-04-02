#pragma once

#include "AudioNode.h"
#include "MergeDelayPad.h"

#include <array>
#include <atomic>
#include <juce_dsp/juce_dsp.h>

#include <memory>
#include <vector>

namespace razumov::graph
{

/**
 * Спектральный компрессор: STFT, динамика по модулю с сохранением фазы входа.
 * Детектор — взвешенная энергия в полосе (center + Q), огибающая Attack/Release,
 * порог и ratio задают общее снятие, применяемое к бинам с весом полосы.
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
    void setSidechainHz(float hz) noexcept { sidechainHz_ = juce::jlimit(20.0f, 20000.0f, hz); }
    void setSidechainQ(float q) noexcept { sidechainQ_ = juce::jlimit(0.25f, 16.0f, q); }
    void setAttackMs(float ms) noexcept { attackMs_ = juce::jlimit(0.5f, 500.0f, ms); }
    void setReleaseMs(float ms) noexcept { releaseMs_ = juce::jlimit(5.0f, 3000.0f, ms); }

    void prepare(double sampleRate, int maxBlockSize, int numChannels) override;
    void reset() override;
    void process(juce::AudioBuffer<float>& buffer) override;

    bool copySpectralCompressionDisplay256(float* inNorm256, float* redNorm256) const noexcept override;
    float getSpectralSidechainEnvDbForUi() const noexcept override;

private:
    struct ChannelData;
    friend struct ChannelData;

    static constexpr int kSpectralDisplayBins = 256;

    void commitSpectralMeterFrame(int half,
                                  const float* magIn,
                                  const float* weight,
                                  float gainLin,
                                  float thresholdDb,
                                  float ratio) noexcept;

    static constexpr int fftOrder_ = 10;
    static constexpr int fftSize_ = 1 << fftOrder_;
    static constexpr int hop_ = fftSize_ / 2;
    static constexpr int kLatencySamples_ = fftSize_;

    bool bypass_ { false };
    float mix_ { 0.75f };
    float thresholdDb_ { -24.0f };
    float ratio_ { 3.0f };
    float sidechainHz_ { 2000.0f };
    float sidechainQ_ { 1.2f };
    float attackMs_ { 50.0f };
    float releaseMs_ { 250.0f };

    double sampleRate_ { 44100.0 };

    int numChannels_ { 2 };

    std::unique_ptr<juce::dsp::FFT> fft_;
    std::vector<float> window_;

    std::unique_ptr<MergeDelayPad> dryPad_;
    std::vector<std::unique_ptr<ChannelData>> channels_;

    juce::AudioBuffer<float> dryScratch_;

    std::array<std::atomic<float>, kSpectralDisplayBins> specInNorm_ {};
    std::array<std::atomic<float>, kSpectralDisplayBins> specRedNorm_ {};
    std::atomic<float> scEnvDbUi_ { -120.0f };
};

} // namespace razumov::graph
