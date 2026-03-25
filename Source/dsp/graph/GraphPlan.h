#pragma once

#include "AudioNode.h"
#include <memory>
#include <variant>
#include <vector>

namespace razumov::graph
{

struct SerialStep
{
    std::vector<std::unique_ptr<AudioNode>> nodes;
};

struct ParallelStep
{
    std::vector<std::unique_ptr<AudioNode>> left;
    std::vector<std::unique_ptr<AudioNode>> right;
};

using GraphStep = std::variant<SerialStep, ParallelStep>;

/** Неизменяемый план (после сборки): владение узлами, топология serial / parallel. */
class GraphPlan
{
public:
    GraphPlan() = default;

    explicit GraphPlan(std::vector<GraphStep> steps)
        : steps_(std::move(steps))
    {
    }

    const std::vector<GraphStep>& getSteps() const noexcept { return steps_; }
    std::vector<GraphStep>& getSteps() noexcept { return steps_; }

    void prepare(double sampleRate, int maxBlockSize, int numChannels);
    void reset();

    /** Суммарная задержка плагина для хоста: serial — сумма; parallel — max(веток). */
    int computePluginLatencySamples() const;

private:
    std::vector<GraphStep> steps_;
};

int chainLatencySamples(const std::vector<std::unique_ptr<AudioNode>>& chain) noexcept;

} // namespace razumov::graph
