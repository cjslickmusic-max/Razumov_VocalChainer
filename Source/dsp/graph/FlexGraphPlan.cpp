#include "FlexGraphPlan.h"

#include "AudioNodeFactory.h"

namespace razumov::graph
{

int slotLatencySamples(const FlexSlot& slot) noexcept
{
    if (slot.type == FlexSlot::Type::Module)
    {
        if (slot.bypassed || slot.node == nullptr)
            return 0;
        return slot.node->getLatencySamples();
    }

    int mx = 0;
    for (const auto& br : slot.branches)
        mx = juce::jmax(mx, segmentLatencySamples(br));
    return mx;
}

int segmentLatencySamples(const FlexSegment& seg) noexcept
{
    int sum = 0;
    for (const auto& s : seg)
        sum += slotLatencySamples(s);
    return sum;
}

void prepareSegment(FlexSegment& seg, double sampleRate, int maxBlockSize, int numChannels)
{
    for (auto& slot : seg)
    {
        if (slot.type == FlexSlot::Type::Module)
        {
            if (slot.node != nullptr)
                slot.node->prepare(sampleRate, maxBlockSize, numChannels);
        }
        else
        {
            for (auto& br : slot.branches)
                prepareSegment(br, sampleRate, maxBlockSize, numChannels);
        }
    }
}

void resetSegment(FlexSegment& seg)
{
    for (auto& slot : seg)
    {
        if (slot.type == FlexSlot::Type::Module)
        {
            if (slot.node != nullptr)
                slot.node->reset();
        }
        else
        {
            for (auto& br : slot.branches)
                resetSegment(br);
        }
    }
}

int FlexGraphPlan::computeMaxSplitBreadthRuntime(const FlexSegment& seg) noexcept
{
    int m = 1;
    for (const auto& s : seg)
    {
        if (s.type == FlexSlot::Type::Split)
        {
            m = juce::jmax(m, (int) s.branches.size());
            for (const auto& br : s.branches)
                m = juce::jmax(m, computeMaxSplitBreadthRuntime(br));
        }
    }
    return m;
}

FlexSlot FlexGraphPlan::buildSlotFromDesc(const FlexSlotDesc& d)
{
    FlexSlot slot;
    slot.slotId = d.slotId;
    slot.bypassed = d.bypassed;

    if (d.descType == FlexSlotDescType::Module)
    {
        slot.type = FlexSlot::Type::Module;
        slot.node = createAudioNodeFromModuleDesc(d);
        return slot;
    }

    slot.type = FlexSlot::Type::Split;
    for (const auto& branchDesc : d.branches)
    {
        FlexSegment branch;
        branch.reserve(branchDesc.size());
        for (const auto& child : branchDesc)
            branch.push_back(buildSlotFromDesc(child));
        slot.branches.push_back(std::move(branch));
    }
    return slot;
}

FlexGraphPlan FlexGraphPlan::buildFromDesc(const FlexSegmentDesc& desc)
{
    FlexSegment root;
    root.reserve(desc.size());
    for (const auto& d : desc)
        root.push_back(buildSlotFromDesc(d));

    const int breadthFromDesc = computeMaxSplitBreadth(desc);
    const int breadthRuntime = computeMaxSplitBreadthRuntime(root);
    const int breadth = juce::jmax(breadthFromDesc, breadthRuntime);

    return FlexGraphPlan(std::move(root), breadth);
}

void FlexGraphPlan::prepare(double sampleRate, int maxBlockSize, int numChannels)
{
    prepareSegment(root_, sampleRate, maxBlockSize, numChannels);
}

void FlexGraphPlan::reset()
{
    resetSegment(root_);
}

int FlexGraphPlan::computePluginLatencySamples() const noexcept
{
    return segmentLatencySamples(root_);
}

} // namespace razumov::graph
