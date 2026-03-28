#include "FlexGraphSerialization.h"

namespace razumov::graph
{
namespace
{

juce::ValueTree slotToTree(const FlexSlotDesc& s)
{
    juce::ValueTree t(flexSlotValueTreeType);
    t.setProperty("id", (int) s.slotId, nullptr);
    t.setProperty("bypass", s.bypassed ? 1 : 0, nullptr);
    t.setProperty("label", s.uiLabel, nullptr);

    if (s.descType == FlexSlotDescType::Module)
    {
        t.setProperty("shape", "module", nullptr);
        t.setProperty("kind", (int) s.kind, nullptr);
        t.setProperty("gainLinear", s.gainLinear, nullptr);
        t.setProperty("filterCutoffHz", s.filterCutoffHz, nullptr);
        t.setProperty("latencySamples", s.latencySamples, nullptr);
        return t;
    }

    t.setProperty("shape", "split", nullptr);
    for (const auto& branch : s.branches)
    {
        juce::ValueTree b(flexBranchValueTreeType);
        for (const auto& child : branch)
            b.addChild(slotToTree(child), -1, nullptr);
        t.addChild(b, -1, nullptr);
    }
    return t;
}

bool treeToSlot(const juce::ValueTree& t, FlexSlotDesc& s) noexcept
{
    if (!t.hasType(flexSlotValueTreeType))
        return false;

    s.slotId = (uint32_t)(int) t.getProperty("id", 0);
    s.bypassed = ((int) t.getProperty("bypass", 0)) != 0;
    s.uiLabel = t.getProperty("label").toString();

    const auto shape = t.getProperty("shape").toString();
    if (shape == "module")
    {
        s.descType = FlexSlotDescType::Module;
        s.kind = (AudioNodeKind)(int) t.getProperty("kind", (int) AudioNodeKind::Gain);
        s.gainLinear = (float) t.getProperty("gainLinear", 1.0);
        s.filterCutoffHz = (float) t.getProperty("filterCutoffHz", 20000.0);
        s.latencySamples = (int) t.getProperty("latencySamples", 0);
        s.branches.clear();
        return true;
    }

    if (shape != "split")
        return false;

    s.descType = FlexSlotDescType::Split;
    s.kind = AudioNodeKind::Gain;
    s.branches.clear();

    for (int i = 0; i < t.getNumChildren(); ++i)
    {
        const auto b = t.getChild(i);
        if (!b.hasType(flexBranchValueTreeType))
            continue;
        std::vector<FlexSlotDesc> branchSlots;
        for (int j = 0; j < b.getNumChildren(); ++j)
        {
            FlexSlotDesc ch;
            if (treeToSlot(b.getChild(j), ch))
                branchSlots.push_back(std::move(ch));
        }
        s.branches.push_back(std::move(branchSlots));
    }
    return true;
}

bool needsIdsInSlot(const FlexSlotDesc& s) noexcept
{
    if (s.slotId == 0)
        return true;
    if (s.descType != FlexSlotDescType::Split)
        return false;
    for (const auto& br : s.branches)
        for (const auto& c : br)
            if (needsIdsInSlot(c))
                return true;
    return false;
}

void assignIdsInSlot(FlexSlotDesc& s, uint32_t& nextId) noexcept
{
    if (s.slotId == 0)
        s.slotId = nextId++;

    if (s.descType != FlexSlotDescType::Split)
        return;

    for (auto& br : s.branches)
        for (auto& c : br)
            assignIdsInSlot(c, nextId);
}

} // namespace

juce::ValueTree flexSegmentDescToValueTree(const FlexSegmentDesc& seg, uint32_t nextSlotCounter)
{
    juce::ValueTree root(flexGraphValueTreeType);
    root.setProperty("version", 1, nullptr);
    root.setProperty("nextSlotCounter", (int) nextSlotCounter, nullptr);
    for (const auto& s : seg)
        root.addChild(slotToTree(s), -1, nullptr);
    return root;
}

bool valueTreeToFlexSegmentDesc(const juce::ValueTree& tree, FlexSegmentDesc& out, uint32_t* nextSlotCounterOut) noexcept
{
    if (!tree.hasType(flexGraphValueTreeType))
        return false;

    if (nextSlotCounterOut != nullptr)
        *nextSlotCounterOut = (uint32_t)(int) tree.getProperty("nextSlotCounter", 1);

    out.clear();
    for (int i = 0; i < tree.getNumChildren(); ++i)
    {
        FlexSlotDesc s;
        if (treeToSlot(tree.getChild(i), s))
            out.push_back(std::move(s));
    }
    return true;
}

uint32_t maxSlotIdInSlot(const FlexSlotDesc& s) noexcept
{
    uint32_t m = s.slotId;
    if (s.descType != FlexSlotDescType::Split)
        return m;
    for (const auto& br : s.branches)
        for (const auto& c : br)
            m = juce::jmax(m, maxSlotIdInSlot(c));
    return m;
}

uint32_t maxSlotIdInSegment(const FlexSegmentDesc& seg) noexcept
{
    uint32_t m = 0;
    for (const auto& s : seg)
        m = juce::jmax(m, maxSlotIdInSlot(s));
    return m;
}

bool flexGraphNeedsSlotIdAssignment(const FlexSegmentDesc& seg) noexcept
{
    for (const auto& s : seg)
        if (needsIdsInSlot(s))
            return true;
    return false;
}

void assignUniqueSlotIds(FlexSegmentDesc& seg, uint32_t& nextId) noexcept
{
    for (auto& s : seg)
        assignIdsInSlot(s, nextId);
}

void assignSlotIdsForSubtree(FlexSlotDesc& slot, uint32_t& nextId) noexcept
{
    assignIdsInSlot(slot, nextId);
}

} // namespace razumov::graph
