#include "GraphPlanFactory.h"
#include "CompressorArchetypeNode.h"

namespace razumov::graph
{
namespace
{

FlexSlotDesc module(AudioNodeKind kind)
{
    FlexSlotDesc d;
    d.descType = FlexSlotDescType::Module;
    d.kind = kind;
    return d;
}

FlexSlotDesc gainModule(float linear)
{
    auto d = module(AudioNodeKind::Gain);
    d.gainLinear = linear;
    return d;
}

FlexSlotDesc filterModule(float cutoffHz)
{
    auto d = module(AudioNodeKind::Filter);
    d.filterCutoffHz = cutoffHz;
    return d;
}

FlexSlotDesc latencyModule(int samples)
{
    auto d = module(AudioNodeKind::Latency);
    d.latencySamples = samples;
    return d;
}

FlexSlotDesc compressorModule(CompressorArchetypeNode::Archetype arch)
{
    FlexSlotDesc d;
    d.descType = FlexSlotDescType::Module;
    switch (arch)
    {
        case CompressorArchetypeNode::Archetype::Opto:
            d.kind = AudioNodeKind::OptoCompressor;
            break;
        case CompressorArchetypeNode::Archetype::Fet:
            d.kind = AudioNodeKind::FetCompressor;
            break;
        case CompressorArchetypeNode::Archetype::Vca:
            d.kind = AudioNodeKind::VcaCompressor;
            break;
    }
    return d;
}

} // namespace

FlexSegmentDesc GraphPlanFactory::makePassthroughDesc()
{
    return {};
}

FlexSegmentDesc GraphPlanFactory::makeSerialGainAndWideFilterDesc(double sampleRate)
{
    (void) sampleRate;
    FlexSegmentDesc d;
    d.push_back(gainModule(1.0f));
    d.push_back(filterModule(20000.0f));
    return d;
}

FlexSegmentDesc GraphPlanFactory::makeDefaultVocalChainPhase3Desc(double sampleRate)
{
    (void) sampleRate;
    FlexSegmentDesc d;
    d.push_back(module(AudioNodeKind::MicCorrection));
    d.push_back(gainModule(1.0f));
    d.push_back(filterModule(20000.0f));
    d.push_back(module(AudioNodeKind::Deesser));
    d.push_back(compressorModule(CompressorArchetypeNode::Archetype::Opto));
    d.push_back(compressorModule(CompressorArchetypeNode::Archetype::Fet));
    d.push_back(compressorModule(CompressorArchetypeNode::Archetype::Vca));
    d.push_back(module(AudioNodeKind::Exciter));
    d.push_back(module(AudioNodeKind::SpectralCompressor));
    return d;
}

FlexSegmentDesc GraphPlanFactory::makeCompactVocalChainPhase3Desc(double sampleRate)
{
    (void) sampleRate;
    FlexSegmentDesc d;
    d.push_back(module(AudioNodeKind::MicCorrection));
    d.push_back(gainModule(1.0f));
    d.push_back(filterModule(20000.0f));
    d.push_back(compressorModule(CompressorArchetypeNode::Archetype::Opto));
    d.push_back(module(AudioNodeKind::Exciter));
    d.push_back(module(AudioNodeKind::SpectralCompressor));
    return d;
}

FlexSegmentDesc GraphPlanFactory::makeFetForwardVocalChainPhase3Desc(double sampleRate)
{
    (void) sampleRate;
    FlexSegmentDesc d;
    d.push_back(module(AudioNodeKind::MicCorrection));
    d.push_back(gainModule(1.0f));
    d.push_back(filterModule(20000.0f));
    d.push_back(module(AudioNodeKind::Deesser));
    d.push_back(compressorModule(CompressorArchetypeNode::Archetype::Fet));
    d.push_back(compressorModule(CompressorArchetypeNode::Archetype::Opto));
    d.push_back(compressorModule(CompressorArchetypeNode::Archetype::Vca));
    d.push_back(module(AudioNodeKind::Exciter));
    d.push_back(module(AudioNodeKind::SpectralCompressor));
    return d;
}

FlexSegmentDesc GraphPlanFactory::makeStartupDescForIndex(int index, double sampleRate)
{
    switch (index)
    {
        case 1:
            return makeCompactVocalChainPhase3Desc(sampleRate);
        case 2:
            return makeFetForwardVocalChainPhase3Desc(sampleRate);
        case 0:
        default:
            return makeDefaultVocalChainPhase3Desc(sampleRate);
    }
}

FlexSegmentDesc GraphPlanFactory::makeParallelHalvesDesc()
{
    FlexSlotDesc split;
    split.descType = FlexSlotDescType::Split;
    split.branches.resize(2);
    split.branches[0].push_back(gainModule(0.5f));
    split.branches[1].push_back(gainModule(0.5f));
    return FlexSegmentDesc { split };
}

FlexSegmentDesc GraphPlanFactory::makeParallelMismatchedLatencyDescForTests()
{
    FlexSlotDesc split;
    split.descType = FlexSlotDescType::Split;
    split.branches.resize(2);
    split.branches[0].push_back(gainModule(0.5f));
    split.branches[1].push_back(latencyModule(64));
    split.branches[1].push_back(gainModule(0.5f));
    return FlexSegmentDesc { split };
}

std::unique_ptr<FlexGraphPlan> GraphPlanFactory::makePlanFromDesc(const FlexSegmentDesc& desc)
{
    return std::make_unique<FlexGraphPlan>(FlexGraphPlan::buildFromDesc(desc));
}

std::unique_ptr<FlexGraphPlan> GraphPlanFactory::makeStartupChainForIndex(int index, double sampleRate)
{
    return makePlanFromDesc(makeStartupDescForIndex(index, sampleRate));
}

} // namespace razumov::graph
