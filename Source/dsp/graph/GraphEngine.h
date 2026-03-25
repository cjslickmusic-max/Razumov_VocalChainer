#pragma once

#include "GraphPlan.h"
#include "MergeDelayPad.h"
#include <functional>
#include <memory>
#include <mutex>

namespace razumov::graph
{

/** Выполнение плана: смена графа через submitPlan (message thread), process — audio thread. */
class GraphEngine
{
public:
    GraphEngine() = default;

    void setLatencyCallback(std::function<void(int)> callback) { onLatency_ = std::move(callback); }

    void prepare(double sampleRate, int maxBlockSize, int numChannels);
    void releaseResources();

    /** Audio thread: прогон активного плана; пустой/null план — прозрачный проход. */
    void process(juce::AudioBuffer<float>& buffer);

    /** Message / UI thread: новый план подменится в начале ближайшего process (без аллокаций в audio). */
    void submitPlan(std::shared_ptr<GraphPlan> plan);

    int getReportedLatencySamples() const noexcept { return reportedLatency_; }

private:
    void swapAndPreparePendingPlan();
    void runSteps(juce::AudioBuffer<float>& buffer);
    void processParallel(ParallelStep& par, juce::AudioBuffer<float>& buffer);

    std::function<void(int)> onLatency_;

    std::mutex mutex_;
    std::shared_ptr<GraphPlan> pendingPlan_;
    std::shared_ptr<GraphPlan> activePlan_;

    double sampleRate_ { 44100.0 };
    int maxBlockSize_ { 512 };
    int numChannels_ { 2 };

    int reportedLatency_ { 0 };

    juce::AudioBuffer<float> workL_;
    juce::AudioBuffer<float> workR_;

    std::unique_ptr<MergeDelayPad> padL_;
    std::unique_ptr<MergeDelayPad> padR_;

    int maxDelayStorage_ { 0 };
};

} // namespace razumov::graph
