#include <dsp/graph/AudioNode.h>
#include <dsp/graph/FlexGraphDesc.h>
#include <dsp/graph/FlexGraphSerialization.h>
#include <dsp/graph/GraphPlanFactory.h>

using razumov::graph::graphContainsAnySplit;

#include <cassert>
#include <cmath>

namespace
{

using namespace razumov::graph;

void testRoundTripPreservesValueTree()
{
    const auto desc = GraphPlanFactory::makeNestedParallelMismatchedLatencyDescForTests();
    const uint32_t nextIn = 1000;
    const juce::ValueTree vt = flexSegmentDescToValueTree(desc, nextIn);

    FlexSegmentDesc out;
    uint32_t counterOut = 0;
    assert(valueTreeToFlexSegmentDesc(vt, out, &counterOut));

    const juce::ValueTree vt2 = flexSegmentDescToValueTree(out, counterOut);
    assert(vt.isEquivalentTo(vt2));
}

void testRoundTripPassthroughEmpty()
{
    const auto desc = GraphPlanFactory::makePassthroughDesc();
    const juce::ValueTree vt = flexSegmentDescToValueTree(desc, 1u);

    FlexSegmentDesc out;
    uint32_t counterOut = 0;
    assert(valueTreeToFlexSegmentDesc(vt, out, &counterOut));

    const juce::ValueTree vt2 = flexSegmentDescToValueTree(out, counterOut);
    assert(vt.isEquivalentTo(vt2));
}

void testRoundTripPhaseAlignHalves()
{
    const auto desc = GraphPlanFactory::makeParallelHalvesPhaseAlignDesc(17, 4);
    const juce::ValueTree vt = flexSegmentDescToValueTree(desc, 500u);

    FlexSegmentDesc out;
    uint32_t counterOut = 0;
    assert(valueTreeToFlexSegmentDesc(vt, out, &counterOut));

    const juce::ValueTree vt2 = flexSegmentDescToValueTree(out, counterOut);
    assert(vt.isEquivalentTo(vt2));
}

void testMaxSlotIdMatchesAfterAssign()
{
    auto desc = GraphPlanFactory::makeParallelThirdsDesc();
    const uint32_t m = maxSlotIdInSegment(desc);
    assert(m > 0);

    uint32_t next = m + 50;
    FlexSlotDesc extra;
    extra.descType = FlexSlotDescType::Module;
    extra.kind = AudioNodeKind::Gain;
    extra.gainLinear = 1.0f;
    extra.slotId = 0;
    desc.push_back(extra);

    assert(flexGraphNeedsSlotIdAssignment(desc));
    next = 1;
    assignUniqueSlotIds(desc, next);
    assert(!flexGraphNeedsSlotIdAssignment(desc));
    assert(maxSlotIdInSegment(desc) >= m);
}

void testSwapDirectRootModulesSwapsBeyondProtectedSlots()
{
    auto d = GraphPlanFactory::makeDefaultVocalChainPhase3Desc(48000.0);
    uint32_t next = 1;
    assignUniqueSlotIds(d, next);
    const auto kGain = d[2].slotId;
    const auto kFilter = d[3].slotId;
    const auto kindAt2 = d[2].kind;
    const auto kindAt3 = d[3].kind;
    assert(trySwapDirectRootModuleSlots(d, kGain, kFilter));
    assert(d[2].kind == kindAt3);
    assert(d[3].kind == kindAt2);
}

void testSwapDirectRootModulesRejectsMicOrRoom()
{
    auto d = GraphPlanFactory::makeDefaultVocalChainPhase3Desc(48000.0);
    uint32_t next = 1;
    assignUniqueSlotIds(d, next);
    const auto kMic = d[0].slotId;
    const auto kGain = d[2].slotId;
    assert(!trySwapDirectRootModuleSlots(d, kMic, kGain));
}

void testGraphContainsAnySplit()
{
    const auto serial = GraphPlanFactory::makeDefaultVocalChainPhase3Desc(48000.0);
    assert(!graphContainsAnySplit(serial));
    const auto par = GraphPlanFactory::makeParallelHalvesDesc();
    assert(graphContainsAnySplit(par));
}

void testParallelModuleSplitShape()
{
    const auto sp = GraphPlanFactory::makeSplitDryBranchAndParallelModule(AudioNodeKind::OptoCompressor);
    assert(sp.descType == FlexSlotDescType::Split);
    assert(sp.branches.size() == 2u);
    assert(sp.branches[0].empty());
    assert(sp.branches[1].size() == 1u);
    assert(sp.branchMixLinear.size() == 2u);
    assert(std::abs(sp.branchMixLinear[0] - 0.5f) < 1e-5f);
    assert(std::abs(sp.branchMixLinear[1] - 0.5f) < 1e-5f);
}

} // namespace

void runFlexGraphSerializationTests()
{
    testRoundTripPreservesValueTree();
    testRoundTripPassthroughEmpty();
    testRoundTripPhaseAlignHalves();
    testMaxSlotIdMatchesAfterAssign();
    testSwapDirectRootModulesSwapsBeyondProtectedSlots();
    testSwapDirectRootModulesRejectsMicOrRoom();
    testGraphContainsAnySplit();
    testParallelModuleSplitShape();
}
