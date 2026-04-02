#pragma once

#include "FlexGraphPlan.h"
#include "MergeDelayPad.h"
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

namespace razumov::params
{
struct MacroAudioState;
class ModuleParamsRuntime;
}

namespace razumov::graph
{

/** Выполнение FlexGraphPlan: смена графа через submitPlan (message thread), process — audio thread. */
class GraphEngine
{
public:
    GraphEngine() = default;

    void setLatencyCallback(std::function<void(int)> callback) { onLatency_ = std::move(callback); }

    void prepare(double sampleRate, int maxBlockSize, int numChannels);
    void releaseResources();

    /** Audio thread: swap плана, применение параметров по slotId, затем DSP. */
    void process(juce::AudioBuffer<float>& buffer,
                 const razumov::params::MacroAudioState& macros,
                 const razumov::params::ModuleParamsRuntime& moduleParams);

    /** Message / UI thread: новый план подменится в начале ближайшего process (без аллокаций в audio). */
    void submitPlan(std::shared_ptr<FlexGraphPlan> plan);

    int getReportedLatencySamples() const noexcept { return reportedLatency_; }

    /** UI / message thread: копия 256 нормализованных бинов для slotId (false если нет ISpectrumSource). */
    bool copySpectrumForSlot(uint32_t slotId, float* dst256) const;

    /** UI: сглаженный GR (dB) для Opto/FET/VCA по slotId. */
    float getGainReductionDbForSlot(uint32_t slotId) const;

    /** UI: спектральный компрессор — вход 0...1 и величина снятия 0...1 на бин. */
    bool copySpectralCompressionDisplayForSlot(uint32_t slotId, float* inNorm256, float* redNorm256) const;

    /** UI: уровень сайдчейна (dB) после A/R. */
    float getSpectralSidechainEnvDbForSlot(uint32_t slotId) const;

private:
    void swapAndPreparePendingPlan();
    void ensureBranchPool(int breadth);
    void processSegment(FlexSegment& seg, juce::AudioBuffer<float>& buffer, int splitDepth);
    void processSplit(FlexSlot& slot, juce::AudioBuffer<float>& buffer, int splitDepth);

    static bool walkCopySpectrum(const FlexSegment& seg, uint32_t slotId, float* dst256) noexcept;
    static bool walkGainReduction(const FlexSegment& seg, uint32_t slotId, float& outDb) noexcept;
    static bool walkSpectralCompressionDisplay(const FlexSegment& seg,
                                               uint32_t slotId,
                                               float* in256,
                                               float* red256) noexcept;
    static bool walkSpectralSidechainEnvDb(const FlexSegment& seg, uint32_t slotId, float& outDb) noexcept;

    std::function<void(int)> onLatency_;

    mutable std::mutex mutex_;
    std::shared_ptr<FlexGraphPlan> pendingPlan_;
    std::shared_ptr<FlexGraphPlan> activePlan_;

    double sampleRate_ { 44100.0 };
    int maxBlockSize_ { 512 };
    int numChannels_ { 2 };

    int reportedLatency_ { 0 };

    int maxDelayStorage_ { 0 };
    int maxSplitBreadthForPads_ { 2 };

    std::vector<juce::AudioBuffer<float>> branchBuffers_;
    /** Phase align на ветке (перед PDC merge). */
    std::vector<std::unique_ptr<MergeDelayPad>> phaseAlignPads_;
    std::vector<std::unique_ptr<MergeDelayPad>> mergePads_;
    /** Задержки веток при split; размер >= maxSplitBreadth, без аллокаций в processSplit. */
    std::vector<int> latPerBranchScratch_;
};

} // namespace razumov::graph
