#include "FlexGraphDesc.h"

namespace razumov::graph
{
namespace
{
const char* shortNameForKind(AudioNodeKind k) noexcept
{
    switch (k)
    {
        case AudioNodeKind::MicCorrection:
            return "Mic";
        case AudioNodeKind::Gain:
            return "Gain";
        case AudioNodeKind::Filter:
            return "LP";
        case AudioNodeKind::Deesser:
            return "De-ess";
        case AudioNodeKind::OptoCompressor:
            return "Opto";
        case AudioNodeKind::FetCompressor:
            return "FET";
        case AudioNodeKind::VcaCompressor:
            return "VCA";
        case AudioNodeKind::Exciter:
            return "Exc";
        case AudioNodeKind::SpectralCompressor:
            return "Spec";
        case AudioNodeKind::Latency:
            return "Lat";
        default:
            return "?";
    }
}

int maxBreadthInSegment(const FlexSegmentDesc& seg) noexcept
{
    int m = 1;
    for (const auto& s : seg)
    {
        if (s.descType == FlexSlotDescType::Split)
        {
            m = juce::jmax(m, (int) s.branches.size());
            for (const auto& br : s.branches)
                m = juce::jmax(m, maxBreadthInSegment(br));
        }
    }
    return m;
}

void walkSegmentForStrip(const FlexSegmentDesc& seg, std::vector<ChainStripItem>& out)
{
    for (const auto& slot : seg)
    {
        if (slot.descType == FlexSlotDescType::Module)
        {
            ChainStripItem it;
            it.slotId = slot.slotId;
            it.bypassed = slot.bypassed;
            it.label = slot.uiLabel.isNotEmpty() ? slot.uiLabel : juce::String(shortNameForKind(slot.kind));
            out.push_back(it);
            continue;
        }

        ChainStripItem splitIt;
        splitIt.slotId = slot.slotId;
        splitIt.bypassed = slot.bypassed;
        splitIt.label = "Par" + juce::String((int) slot.branches.size());
        out.push_back(splitIt);

        for (const auto& br : slot.branches)
            walkSegmentForStrip(br, out);
    }
}

bool slotTreeContainsId(const FlexSlotDesc& s, uint32_t id) noexcept
{
    if (s.slotId == id)
        return true;
    if (s.descType != FlexSlotDescType::Split)
        return false;
    for (const auto& br : s.branches)
        for (const auto& c : br)
            if (slotTreeContainsId(c, id))
                return true;
    return false;
}

} // namespace

int computeMaxSplitBreadth(const FlexSegmentDesc& root) noexcept
{
    const int b = maxBreadthInSegment(root);
    return juce::jmax(2, b);
}

std::vector<ChainStripItem> segmentDescToChainStripItems(const FlexSegmentDesc& root)
{
    std::vector<ChainStripItem> out;
    walkSegmentForStrip(root, out);
    return out;
}

juce::StringArray segmentDescToChainStripLabels(const FlexSegmentDesc& root)
{
    juce::StringArray a;
    for (const auto& it : segmentDescToChainStripItems(root))
        a.add(it.label);
    return a;
}

bool setSlotBypassById(FlexSegmentDesc& root, uint32_t slotId, bool bypassed) noexcept
{
    for (auto& s : root)
    {
        if (s.slotId == slotId)
        {
            s.bypassed = bypassed;
            return true;
        }
        if (s.descType == FlexSlotDescType::Split)
        {
            for (auto& br : s.branches)
                if (setSlotBypassById(br, slotId, bypassed))
                    return true;
        }
    }
    return false;
}

bool removeSlotById(FlexSegmentDesc& root, uint32_t slotId) noexcept
{
    for (auto it = root.begin(); it != root.end(); ++it)
    {
        if (it->slotId == slotId)
        {
            root.erase(it);
            return true;
        }
        if (it->descType == FlexSlotDescType::Split)
        {
            for (auto& br : it->branches)
                if (removeSlotById(br, slotId))
                    return true;
        }
    }
    return false;
}

std::optional<AudioNodeKind> queryModuleKindForSlotId(const FlexSegmentDesc& root, uint32_t slotId) noexcept
{
    for (const auto& s : root)
    {
        if (s.slotId == slotId)
        {
            if (s.descType == FlexSlotDescType::Module)
                return s.kind;
            return std::nullopt;
        }
        if (s.descType == FlexSlotDescType::Split)
        {
            for (const auto& br : s.branches)
                if (auto k = queryModuleKindForSlotId(br, slotId))
                    return k;
        }
    }
    return std::nullopt;
}

bool queryIsParallelSplitSlot(const FlexSegmentDesc& seg, uint32_t slotId) noexcept
{
    for (const auto& s : seg)
    {
        if (s.slotId == slotId)
            return s.descType == FlexSlotDescType::Split;
        if (s.descType == FlexSlotDescType::Split)
        {
            for (const auto& br : s.branches)
                if (queryIsParallelSplitSlot(br, slotId))
                    return true;
        }
    }
    return false;
}

int findRootSlotIndexContainingId(const FlexSegmentDesc& root, uint32_t slotId) noexcept
{
    for (int i = 0; i < (int) root.size(); ++i)
        if (slotTreeContainsId(root[(size_t) i], slotId))
            return i;
    return -1;
}

} // namespace razumov::graph
