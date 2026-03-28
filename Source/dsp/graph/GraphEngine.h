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

private:
    void swapAndPreparePendingPlan();
    void ensureBranchPool(int breadth);
    void processSegment(FlexSegment& seg, juce::AudioBuffer<float>& buffer);
    void processSplit(FlexSlot& slot, juce::AudioBuffer<float>& buffer);

    std::function<void(int)> onLatency_;

    std::mutex mutex_;
    std::shared_ptr<FlexGraphPlan> pendingPlan_;
    std::shared_ptr<FlexGraphPlan> activePlan_;

    double sampleRate_ { 44100.0 };
    int maxBlockSize_ { 512 };
    int numChannels_ { 2 };

    int reportedLatency_ { 0 };

    int maxDelayStorage_ { 0 };

    std::vector<juce::AudioBuffer<float>> branchBuffers_;
    std::vector<std::unique_ptr<MergeDelayPad>> mergePads_;
};

} // namespace razumov::graph
