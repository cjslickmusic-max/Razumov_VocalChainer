#pragma once

#include "AudioNode.h"

namespace razumov::graph
{

/**
 * Слот коррекции микрофона (линеаризация / профиль).
 * Сейчас: полный pass-through (блок остаётся в цепочке и в UI). Профиль EQ/IR — отдельная доработка.
 * bypass/amount зарезервированы под будущую DSP-логику.
 */
class MicCorrectionNode final : public AudioNode
{
public:
    MicCorrectionNode() = default;

    AudioNodeKind getKind() const noexcept override { return AudioNodeKind::MicCorrection; }

    int getLatencySamples() const noexcept override { return 0; }

    void setBypass(bool b) noexcept { bypass_ = b; }
    bool getBypass() const noexcept { return bypass_; }

    /** 0..1: будущая доля коррекции (сейчас не влияет на DSP). */
    void setAmount(float a) noexcept { amount_ = juce::jlimit(0.0f, 1.0f, a); }
    float getAmount() const noexcept { return amount_; }

    void prepare(double sampleRate, int maxBlockSize, int numChannels) override;
    void reset() override;
    void process(juce::AudioBuffer<float>& buffer) override;

private:
    bool bypass_ { false };
    float amount_ { 1.0f };
};

} // namespace razumov::graph
