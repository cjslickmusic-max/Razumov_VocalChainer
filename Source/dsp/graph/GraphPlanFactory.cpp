#include "GraphPlanFactory.h"
#include "CompressorArchetypeNode.h"
#include "FlexGraphSerialization.h"

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
    uint32_t n = 1;
    assignUniqueSlotIds(d, n);
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
    FlexSegmentDesc d;
    switch (index)
    {
        case 1:
            d = makeCompactVocalChainPhase3Desc(sampleRate);
            break;
        case 2:
            d = makeFetForwardVocalChainPhase3Desc(sampleRate);
            break;
        case 0:
        default:
            d = makeDefaultVocalChainPhase3Desc(sampleRate);
            break;
    }
    uint32_t n = 1;
    assignUniqueSlotIds(d, n);
    return d;
}

FlexSegmentDesc GraphPlanFactory::makeParallelHalvesDesc()
{
    FlexSlotDesc split;
    split.descType = FlexSlotDescType::Split;
    split.branches.resize(2);
    split.branches[0].push_back(gainModule(0.5f));
    split.branches[1].push_back(gainModule(0.5f));
    FlexSegmentDesc d { split };
    uint32_t n = 1;
    assignUniqueSlotIds(d, n);
    return d;
}

FlexSegmentDesc GraphPlanFactory::makeParallelHalvesPhaseAlignDesc(int branch0Align, int branch1Align)
{
    FlexSlotDesc split;
    split.descType = FlexSlotDescType::Split;
    split.branches.resize(2);
    split.branchPhaseAlignSamples.resize(2);
    split.branchPhaseAlignSamples[0] = juce::jlimit(0, kMaxBranchPhaseAlignSamples, branch0Align);
    split.branchPhaseAlignSamples[1] = juce::jlimit(0, kMaxBranchPhaseAlignSamples, branch1Align);
    split.branches[0].push_back(gainModule(0.5f));
    split.branches[1].push_back(gainModule(0.5f));
    FlexSegmentDesc d { split };
    uint32_t n = 1;
    assignUniqueSlotIds(d, n);
    return d;
}

FlexSegmentDesc GraphPlanFactory::makeParallelThirdsDesc()
{
    FlexSlotDesc split;
    split.descType = FlexSlotDescType::Split;
    split.branches.resize(3);
    split.branches[0].push_back(gainModule(1.0f / 3.0f));
    split.branches[1].push_back(gainModule(1.0f / 3.0f));
    split.branches[2].push_back(gainModule(1.0f / 3.0f));
    FlexSegmentDesc d { split };
    uint32_t n = 1;
    assignUniqueSlotIds(d, n);
    return d;
}

FlexSegmentDesc GraphPlanFactory::makeNestedParallelHalvesDesc()
{
    FlexSlotDesc inner;
    inner.descType = FlexSlotDescType::Split;
    inner.branches.resize(2);
    inner.branches[0].push_back(gainModule(0.25f));
    inner.branches[1].push_back(gainModule(0.25f));

    FlexSlotDesc outer;
    outer.descType = FlexSlotDescType::Split;
    outer.branches.resize(2);
    outer.branches[0].push_back(inner);
    outer.branches[1].push_back(gainModule(0.5f));
    FlexSegmentDesc d { outer };
    uint32_t n = 1;
    assignUniqueSlotIds(d, n);
    return d;
}

FlexSegmentDesc GraphPlanFactory::makeParallelMismatchedLatencyDescForTests()
{
    FlexSlotDesc split;
    split.descType = FlexSlotDescType::Split;
    split.branches.resize(2);
    split.branches[0].push_back(gainModule(0.5f));
    split.branches[1].push_back(latencyModule(64));
    split.branches[1].push_back(gainModule(0.5f));
    FlexSegmentDesc d { split };
    uint32_t n = 1;
    assignUniqueSlotIds(d, n);
    return d;
}

FlexSegmentDesc GraphPlanFactory::makeParallelThreeWayMismatchedLatencyDescForTests()
{
    FlexSlotDesc split;
    split.descType = FlexSlotDescType::Split;
    split.branches.resize(3);
    const float g = 1.0f / 3.0f;
    split.branches[0].push_back(gainModule(g));
    split.branches[1].push_back(latencyModule(32));
    split.branches[1].push_back(gainModule(g));
    split.branches[2].push_back(latencyModule(64));
    split.branches[2].push_back(gainModule(g));
    FlexSegmentDesc d { split };
    uint32_t n = 1;
    assignUniqueSlotIds(d, n);
    return d;
}

FlexSegmentDesc GraphPlanFactory::makeNestedParallelMismatchedLatencyDescForTests()
{
    FlexSlotDesc inner;
    inner.descType = FlexSlotDescType::Split;
    inner.branches.resize(2);
    inner.branches[0].push_back(latencyModule(32));
    inner.branches[0].push_back(gainModule(0.25f));
    inner.branches[1].push_back(gainModule(0.25f));

    FlexSlotDesc outer;
    outer.descType = FlexSlotDescType::Split;
    outer.branches.resize(2);
    outer.branches[0].push_back(inner);
    outer.branches[1].push_back(gainModule(0.5f));
    FlexSegmentDesc d { outer };
    uint32_t n = 1;
    assignUniqueSlotIds(d, n);
    return d;
}

FlexSlotDesc GraphPlanFactory::makeModulePaletteSlot(AudioNodeKind kind)
{
    switch (kind)
    {
        case AudioNodeKind::Gain:
            return gainModule(1.0f);
        case AudioNodeKind::Filter:
            return filterModule(20000.0f);
        case AudioNodeKind::MicCorrection:
            return module(AudioNodeKind::MicCorrection);
        case AudioNodeKind::Deesser:
            return module(AudioNodeKind::Deesser);
        case AudioNodeKind::OptoCompressor:
            return compressorModule(CompressorArchetypeNode::Archetype::Opto);
        case AudioNodeKind::FetCompressor:
            return compressorModule(CompressorArchetypeNode::Archetype::Fet);
        case AudioNodeKind::VcaCompressor:
            return compressorModule(CompressorArchetypeNode::Archetype::Vca);
        case AudioNodeKind::Exciter:
            return module(AudioNodeKind::Exciter);
        case AudioNodeKind::SpectralCompressor:
            return module(AudioNodeKind::SpectralCompressor);
        case AudioNodeKind::Latency:
            return latencyModule(0);
        default:
            return gainModule(1.0f);
    }
}

FlexSlotDesc GraphPlanFactory::makeSplitWithUnityBranches(int numBranches)
{
    const int n = juce::jmax(2, numBranches);
    FlexSlotDesc sp;
    sp.descType = FlexSlotDescType::Split;
    sp.branches.resize((size_t) n);
    for (int i = 0; i < n; ++i)
        sp.branches[(size_t) i].push_back(gainModule(1.0f));
    return sp;
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
