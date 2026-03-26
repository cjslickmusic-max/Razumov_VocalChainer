#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

namespace razumov::graph
{

class GainNode;
class FilterNode;

/** Тип узла для привязки параметров и отладки. */
enum class AudioNodeKind : uint8_t
{
    Unknown = 0,
    MicCorrection,
    Gain,
    Filter,
    OptoCompressor,
    FetCompressor,
    VcaCompressor,
    Deesser,
    Exciter,
    SpectralCompressor,
    Latency,
};

/** Базовый узел графа: стерео in-place, отчёт задержки в целых сэмплах. */
class AudioNode
{
public:
    virtual ~AudioNode() = default;

    virtual AudioNodeKind getKind() const noexcept { return AudioNodeKind::Unknown; }

    virtual GainNode* asGain() noexcept { return nullptr; }
    virtual FilterNode* asFilter() noexcept { return nullptr; }

    virtual void prepare(double sampleRate, int maxBlockSize, int numChannels) = 0;
    virtual void reset() = 0;

    /** Обработка in-place; numChannels >= 2 для стерео. */
    virtual void process(juce::AudioBuffer<float>& buffer) = 0;

    /** Дополнительная задержка узла (lookahead, FIR, спектр и т.д.). */
    virtual int getLatencySamples() const noexcept { return 0; }
};

} // namespace razumov::graph
