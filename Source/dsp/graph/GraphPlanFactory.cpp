#include "GraphPlanFactory.h"
#include "FilterNode.h"
#include "GainNode.h"
#include "LatencyNode.h"

namespace razumov::graph
{

std::unique_ptr<GraphPlan> GraphPlanFactory::makePassthrough()
{
    return std::make_unique<GraphPlan>();
}

std::unique_ptr<GraphPlan> GraphPlanFactory::makeSerialGainAndWideFilter(double sampleRate)
{
    auto gain = std::make_unique<GainNode>(1.0f);
    auto filt = std::make_unique<FilterNode>();
    filt->setCutoffHz((float) (sampleRate * 0.49));

    SerialStep serial;
    serial.nodes.push_back(std::move(gain));
    serial.nodes.push_back(std::move(filt));

    std::vector<GraphStep> steps;
    steps.emplace_back(std::move(serial));
    return std::make_unique<GraphPlan>(std::move(steps));
}

std::unique_ptr<GraphPlan> GraphPlanFactory::makeParallelHalves()
{
    auto gl = std::make_unique<GainNode>(0.5f);
    auto gr = std::make_unique<GainNode>(0.5f);

    ParallelStep par;
    par.left.push_back(std::move(gl));
    par.right.push_back(std::move(gr));

    std::vector<GraphStep> steps;
    steps.emplace_back(std::move(par));
    return std::make_unique<GraphPlan>(std::move(steps));
}

std::unique_ptr<GraphPlan> GraphPlanFactory::makeParallelMismatchedLatencyForTests()
{
    auto gl = std::make_unique<GainNode>(0.5f);
    auto dr = std::make_unique<LatencyNode>(64);
    auto gr = std::make_unique<GainNode>(0.5f);

    ParallelStep par;
    par.left.push_back(std::move(gl));
    par.right.push_back(std::move(dr));
    par.right.push_back(std::move(gr));

    std::vector<GraphStep> steps;
    steps.emplace_back(std::move(par));
    return std::make_unique<GraphPlan>(std::move(steps));
}

} // namespace razumov::graph
