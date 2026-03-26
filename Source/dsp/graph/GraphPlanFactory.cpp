#include "GraphPlanFactory.h"
#include "CompressorArchetypeNode.h"
#include "DeesserNode.h"
#include "ExciterNode.h"
#include "FilterNode.h"
#include "GainNode.h"
#include "LatencyNode.h"
#include "MicCorrectionNode.h"
#include "SpectralCompressorNode.h"

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
    (void) sampleRate;
    filt->setCutoffHz(20000.0f);

    SerialStep serial;
    serial.nodes.push_back(std::move(gain));
    serial.nodes.push_back(std::move(filt));

    std::vector<GraphStep> steps;
    steps.emplace_back(std::move(serial));
    return std::make_unique<GraphPlan>(std::move(steps));
}

std::unique_ptr<GraphPlan> GraphPlanFactory::makeDefaultVocalChainPhase3(double sampleRate)
{
    (void) sampleRate;

    SerialStep serial;

    serial.nodes.push_back(std::make_unique<MicCorrectionNode>());

    serial.nodes.push_back(std::make_unique<GainNode>(1.0f));

    auto filt = std::make_unique<FilterNode>();
    filt->setCutoffHz(20000.0f);
    serial.nodes.push_back(std::move(filt));

    serial.nodes.push_back(std::make_unique<DeesserNode>());

    serial.nodes.push_back(
        std::make_unique<CompressorArchetypeNode>(CompressorArchetypeNode::Archetype::Opto));
    serial.nodes.push_back(
        std::make_unique<CompressorArchetypeNode>(CompressorArchetypeNode::Archetype::Fet));
    serial.nodes.push_back(
        std::make_unique<CompressorArchetypeNode>(CompressorArchetypeNode::Archetype::Vca));

    serial.nodes.push_back(std::make_unique<ExciterNode>());
    serial.nodes.push_back(std::make_unique<SpectralCompressorNode>());

    std::vector<GraphStep> steps;
    steps.emplace_back(std::move(serial));
    return std::make_unique<GraphPlan>(std::move(steps));
}

std::unique_ptr<GraphPlan> GraphPlanFactory::makeCompactVocalChainPhase3(double sampleRate)
{
    (void) sampleRate;

    SerialStep serial;

    serial.nodes.push_back(std::make_unique<MicCorrectionNode>());
    serial.nodes.push_back(std::make_unique<GainNode>(1.0f));

    auto filt = std::make_unique<FilterNode>();
    filt->setCutoffHz(20000.0f);
    serial.nodes.push_back(std::move(filt));

    serial.nodes.push_back(
        std::make_unique<CompressorArchetypeNode>(CompressorArchetypeNode::Archetype::Opto));

    serial.nodes.push_back(std::make_unique<ExciterNode>());
    serial.nodes.push_back(std::make_unique<SpectralCompressorNode>());

    std::vector<GraphStep> steps;
    steps.emplace_back(std::move(serial));
    return std::make_unique<GraphPlan>(std::move(steps));
}

std::unique_ptr<GraphPlan> GraphPlanFactory::makeFetForwardVocalChainPhase3(double sampleRate)
{
    (void) sampleRate;

    SerialStep serial;

    serial.nodes.push_back(std::make_unique<MicCorrectionNode>());
    serial.nodes.push_back(std::make_unique<GainNode>(1.0f));

    auto filt = std::make_unique<FilterNode>();
    filt->setCutoffHz(20000.0f);
    serial.nodes.push_back(std::move(filt));

    serial.nodes.push_back(std::make_unique<DeesserNode>());

    serial.nodes.push_back(
        std::make_unique<CompressorArchetypeNode>(CompressorArchetypeNode::Archetype::Fet));
    serial.nodes.push_back(
        std::make_unique<CompressorArchetypeNode>(CompressorArchetypeNode::Archetype::Opto));
    serial.nodes.push_back(
        std::make_unique<CompressorArchetypeNode>(CompressorArchetypeNode::Archetype::Vca));

    serial.nodes.push_back(std::make_unique<ExciterNode>());
    serial.nodes.push_back(std::make_unique<SpectralCompressorNode>());

    std::vector<GraphStep> steps;
    steps.emplace_back(std::move(serial));
    return std::make_unique<GraphPlan>(std::move(steps));
}

std::unique_ptr<GraphPlan> GraphPlanFactory::makeStartupChainForIndex(int index, double sampleRate)
{
    switch (index)
    {
        case 1:
            return makeCompactVocalChainPhase3(sampleRate);
        case 2:
            return makeFetForwardVocalChainPhase3(sampleRate);
        case 0:
        default:
            return makeDefaultVocalChainPhase3(sampleRate);
    }
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
