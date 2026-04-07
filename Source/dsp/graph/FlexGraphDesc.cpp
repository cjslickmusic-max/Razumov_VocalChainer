#include "FlexGraphDesc.h"
#include "FlexGraphSerialization.h"

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
        case AudioNodeKind::ParametricEq:
            return "EQ";
        case AudioNodeKind::SpectrumAnalyzer:
            return "SpecAn";
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

void walkSegmentForStrip(const FlexSegmentDesc& seg, std::vector<ChainStripItem>& out, int depth)
{
    for (const auto& slot : seg)
    {
        if (slot.descType == FlexSlotDescType::Module)
        {
            ChainStripItem it;
            it.slotId = slot.slotId;
            it.bypassed = slot.bypassed;
            it.row = depth;
            it.label = slot.uiLabel.isNotEmpty() ? slot.uiLabel : juce::String(shortNameForKind(slot.kind));
            out.push_back(it);
            continue;
        }

        ChainStripItem splitIt;
        splitIt.slotId = slot.slotId;
        splitIt.bypassed = slot.bypassed;
        splitIt.row = depth;
        splitIt.label = "Par" + juce::String((int) slot.branches.size());
        out.push_back(splitIt);

        for (const auto& br : slot.branches)
            walkSegmentForStrip(br, out, depth + 1);
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
    walkSegmentForStrip(root, out, 0);
    return out;
}

bool graphContainsAnySplit(const FlexSegmentDesc& seg) noexcept
{
    for (const auto& s : seg)
    {
        if (s.descType == FlexSlotDescType::Split)
        {
            for (const auto& br : s.branches)
                if (graphContainsAnySplit(br))
                    return true;
            return true;
        }
    }
    return false;
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

bool insertFlexSlotAfterModuleSlotId(FlexSegmentDesc& seg, uint32_t moduleSlotId, const FlexSlotDesc& templateSlot,
                                     uint32_t& nextId) noexcept
{
    for (size_t i = 0; i < seg.size(); ++i)
    {
        auto& s = seg[i];
        if (s.descType == FlexSlotDescType::Module && s.slotId == moduleSlotId)
        {
            FlexSlotDesc copy = templateSlot;
            assignSlotIdsForSubtree(copy, nextId);
            seg.insert(seg.begin() + i + 1, std::move(copy));
            return true;
        }
        if (s.descType == FlexSlotDescType::Split)
        {
            for (auto& br : s.branches)
                if (insertFlexSlotAfterModuleSlotId(br, moduleSlotId, templateSlot, nextId))
                    return true;
        }
    }
    return false;
}

namespace
{
void walkModuleSlotIds(const FlexSegmentDesc& seg, std::vector<uint32_t>& out)
{
    for (const auto& s : seg)
    {
        if (s.descType == FlexSlotDescType::Module)
            out.push_back(s.slotId);
        else if (s.descType == FlexSlotDescType::Split)
        {
            for (const auto& br : s.branches)
                walkModuleSlotIds(br, out);
        }
    }
}
} // namespace

std::vector<uint32_t> collectModuleSlotIds(const FlexSegmentDesc& root)
{
    std::vector<uint32_t> out;
    walkModuleSlotIds(root, out);
    return out;
}

std::optional<uint32_t> getRootModuleSlotIdAtIndex(const FlexSegmentDesc& root, int moduleIndex) noexcept
{
    int mi = 0;
    for (const auto& s : root)
    {
        if (s.descType != FlexSlotDescType::Module)
            continue;
        if (mi == moduleIndex)
            return s.slotId;
        ++mi;
    }
    return std::nullopt;
}

bool isProtectedFrontRootModuleSlot(const FlexSegmentDesc& root, uint32_t slotId) noexcept
{
    int mi = 0;
    for (const auto& s : root)
    {
        if (s.descType != FlexSlotDescType::Module)
            continue;
        if (s.slotId == slotId)
            return mi < 2;
        ++mi;
    }
    return false;
}

bool trySwapDirectRootModuleSlots(FlexSegmentDesc& root, uint32_t slotIdA, uint32_t slotIdB) noexcept
{
    if (slotIdA == 0 || slotIdB == 0 || slotIdA == slotIdB)
        return false;
    int ia = -1;
    int ib = -1;
    for (int i = 0; i < (int) root.size(); ++i)
    {
        const auto& s = root[(size_t) i];
        if (s.descType != FlexSlotDescType::Module)
            continue;
        if (s.slotId == slotIdA)
            ia = i;
        if (s.slotId == slotIdB)
            ib = i;
    }
    if (ia < 0 || ib < 0)
        return false;
    if (ia <= 1 || ib <= 1)
        return false;
    std::swap(root[(size_t) ia], root[(size_t) ib]);
    return true;
}

} // namespace razumov::graph
