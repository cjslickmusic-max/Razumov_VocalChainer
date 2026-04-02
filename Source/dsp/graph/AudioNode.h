#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

namespace razumov::graph
{

class GainNode;
class FilterNode;
class ISpectrumSource;

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
    ParametricEq,
    SpectrumAnalyzer,
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

    /** Привязка к slotId из плана (опционально; для отладки / UI). */
    virtual void bindSlot(uint32_t slotId) noexcept { juce::ignoreUnused(slotId); }

    /** Узел с тапом спектра для панели (иначе nullptr). */
    virtual ISpectrumSource* asSpectrumSource() noexcept { return nullptr; }

    virtual void prepare(double sampleRate, int maxBlockSize, int numChannels) = 0;
    virtual void reset() = 0;

    /** Обработка in-place; numChannels >= 2 для стерео. */
    virtual void process(juce::AudioBuffer<float>& buffer) = 0;

    /** Дополнительная задержка узла (lookahead, FIR, спектр и т.д.). */
    virtual int getLatencySamples() const noexcept { return 0; }

    /** Opto/FET/VCA: сглаженный gain reduction (dB), 0 = нет. Чтение с message/UI thread. */
    virtual float getGainReductionDbForUi() const noexcept { return 0.f; }

    /**
     * Spectral compressor: входной спектр 0...1 и величина «снятия» 0...1 на бин (как вычитание по полосам).
     * false если узел не поддерживает. dst массивы по 256 элементов.
     */
    virtual bool copySpectralCompressionDisplay256(float* inNorm256, float* redNorm256) const noexcept
    {
        juce::ignoreUnused(inNorm256, redNorm256);
        return false;
    }
};

} // namespace razumov::graph
