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

void appendSlotStripLabels(const FlexSlotDesc& slot, juce::StringArray& out)
{
    if (slot.descType == FlexSlotDescType::Module)
    {
        const juce::String lab =
            slot.uiLabel.isNotEmpty() ? slot.uiLabel : juce::String(shortNameForKind(slot.kind));
        out.add(lab);
        return;
    }

    out.add("Par" + juce::String((int) slot.branches.size()));

    for (const auto& br : slot.branches)
    {
        juce::String inner;
        for (size_t i = 0; i < br.size(); ++i)
        {
            if (i > 0)
                inner << " ";
            if (br[i].descType == FlexSlotDescType::Module)
            {
                const juce::String lab = br[i].uiLabel.isNotEmpty()
                                             ? br[i].uiLabel
                                             : juce::String(shortNameForKind(br[i].kind));
                inner << lab;
            }
            else
            {
                inner << "{Par}";
            }
        }
        out.add("(" + inner + ")");
    }
}

} // namespace

int computeMaxSplitBreadth(const FlexSegmentDesc& root) noexcept
{
    const int b = maxBreadthInSegment(root);
    return juce::jmax(2, b);
}

juce::StringArray segmentDescToChainStripLabels(const FlexSegmentDesc& root)
{
    juce::StringArray out;
    for (const auto& slot : root)
        appendSlotStripLabels(slot, out);
    return out;
}

} // namespace razumov::graph
