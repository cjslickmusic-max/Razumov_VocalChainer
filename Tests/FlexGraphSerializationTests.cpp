#include <dsp/graph/AudioNode.h>
#include <dsp/graph/FlexGraphDesc.h>
#include <dsp/graph/FlexGraphSerialization.h>
#include <dsp/graph/GraphPlanFactory.h>

#include <cassert>
#include <cmath>

using razumov::graph::graphContainsAnySplit;

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

void testInsertFlexSlotAfterRootModule()
{
    FlexSegmentDesc d;
    FlexSlotDesc a = GraphPlanFactory::makeModulePaletteSlot(AudioNodeKind::Gain);
    FlexSlotDesc b = GraphPlanFactory::makeModulePaletteSlot(AudioNodeKind::Gain);
    FlexSlotDesc c = GraphPlanFactory::makeModulePaletteSlot(AudioNodeKind::Gain);
    uint32_t n = 1;
    assignSlotIdsForSubtree(a, n);
    assignSlotIdsForSubtree(b, n);
    assignSlotIdsForSubtree(c, n);
    const uint32_t bId = b.slotId;
    d.push_back(std::move(a));
    d.push_back(std::move(b));
    d.push_back(std::move(c));
    FlexSlotDesc sp = GraphPlanFactory::makeSplitDryBranchAndParallelModule(AudioNodeKind::FetCompressor);
    uint32_t next = n;
    assert(insertFlexSlotAfterModuleSlotId(d, bId, sp, next));
    assert(d.size() == 4u);
    assert(d[1].slotId == bId);
    assert(d[2].descType == FlexSlotDescType::Split);
}

void testInsertFlexSlotAfterModuleInNestedBranch()
{
    auto d = GraphPlanFactory::makeNestedParallelMismatchedLatencyDescForTests();
    uint32_t latId = 0;
    for (const auto id : collectModuleSlotIds(d))
    {
        if (queryModuleKindForSlotId(d, id) == AudioNodeKind::Latency)
        {
            latId = id;
            break;
        }
    }
    assert(latId != 0u);
    FlexSlotDesc sp = GraphPlanFactory::makeSplitDryBranchAndParallelModule(AudioNodeKind::Gain);
    uint32_t next = 5000;
    assert(insertFlexSlotAfterModuleSlotId(d, latId, sp, next));
    assert(d.size() == 1u);
    assert(d[0].descType == FlexSlotDescType::Split);
    const auto& innerSplit = d[0].branches[0][0];
    assert(innerSplit.descType == FlexSlotDescType::Split);
    assert(innerSplit.branches[0].size() == 3u);
    assert(innerSplit.branches[0][0].descType == FlexSlotDescType::Module);
    assert(innerSplit.branches[0][1].descType == FlexSlotDescType::Split);
}

void testMigrateSerialAfterSplitIntoBranchZero()
{
    FlexSegmentDesc root;
    FlexSlotDesc g = GraphPlanFactory::makeModulePaletteSlot(AudioNodeKind::Gain);
    FlexSlotDesc sp = GraphPlanFactory::makeSplitDryBranchAndParallelModule(AudioNodeKind::Deesser);
    FlexSlotDesc lp = GraphPlanFactory::makeModulePaletteSlot(AudioNodeKind::Filter);
    uint32_t n = 1;
    assignSlotIdsForSubtree(g, n);
    assignSlotIdsForSubtree(sp, n);
    assignSlotIdsForSubtree(lp, n);
    const uint32_t lpId = lp.slotId;
    root.push_back(std::move(g));
    root.push_back(std::move(sp));
    root.push_back(std::move(lp));
    migrateSerialModulesAfterSplitIntoBranchZero(root);
    assert(root.size() == 2u);
    assert(root[1].descType == FlexSlotDescType::Split);
    assert(!root[1].branches[0].empty());
    assert(queryModuleKindForSlotId(root, lpId) == AudioNodeKind::Filter);
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
    testInsertFlexSlotAfterRootModule();
    testInsertFlexSlotAfterModuleInNestedBranch();
    testMigrateSerialAfterSplitIntoBranchZero();
}
