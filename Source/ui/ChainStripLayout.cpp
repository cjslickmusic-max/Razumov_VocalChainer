#include "ChainStripLayout.h"
#include <cmath>

namespace razumov::ui
{
namespace
{
using razumov::graph::AudioNodeKind;
using razumov::graph::FlexSegmentDesc;
using razumov::graph::FlexSlotDesc;
using razumov::graph::FlexSlotDescType;

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

juce::Point<float> centerOf(const juce::Rectangle<float>& r) noexcept
{
    return { r.getCentreX(), r.getCentreY() };
}

struct SegmentBounds
{
    float endX { 0 };
    juce::Point<float> firstCenter { 0, 0 };
    juce::Point<float> lastCenter { 0, 0 };
    bool hadContent { false };
};

void pushSerialWire(ChainStripLayout& out, juce::Point<float> from, juce::Point<float> to) noexcept
{
    if (from.x < to.x - 0.5f)
        out.wires.push_back({ from, to });
}

SegmentBounds layoutSegment(
    const FlexSegmentDesc& seg,
    float x,
    float yLine,
    float rowPitch,
    float cardW,
    float cardH,
    float hGap,
    float branchGap,
    ChainStripLayout& out)
{
    if (seg.empty())
    {
        SegmentBounds empty;
        empty.endX = x;
        empty.hadContent = false;
        return empty;
    }

    SegmentBounds block;
    float xCursor = x;
    juce::Point<float> lastCenter { 0, 0 };
    bool haveLast = false;

    for (const auto& slot : seg)
    {
        if (slot.descType == FlexSlotDescType::Module)
        {
            ChainStripLayoutCard c;
            c.bounds = { xCursor, yLine, cardW, cardH };
            c.slotId = slot.slotId;
            c.bypassed = slot.bypassed;
            c.label = slot.uiLabel.isNotEmpty() ? slot.uiLabel : juce::String(shortNameForKind(slot.kind));
            c.selectable = true;
            c.isMergeNode = false;
            out.cards.push_back(c);

            const auto curCenter = centerOf(c.bounds);
            if (haveLast)
                pushSerialWire(out, lastCenter, curCenter);

            if (!block.hadContent)
            {
                block.firstCenter = curCenter;
                block.hadContent = true;
            }
            block.lastCenter = curCenter;
            lastCenter = curCenter;
            haveLast = true;

            xCursor += cardW + hGap;
            block.endX = xCursor;
            continue;
        }

        // Split
        ChainStripLayoutCard splitCard;
        splitCard.bounds = { xCursor, yLine, cardW, cardH };
        splitCard.slotId = slot.slotId;
        splitCard.bypassed = slot.bypassed;
        splitCard.label = "Split x" + juce::String((int) slot.branches.size());
        splitCard.selectable = true;
        splitCard.isMergeNode = false;
        out.cards.push_back(splitCard);

        const auto splitCenter = centerOf(splitCard.bounds);
        if (haveLast)
            pushSerialWire(out, lastCenter, splitCenter);

        if (!block.hadContent)
        {
            block.firstCenter = splitCenter;
            block.hadContent = true;
        }

        const float branchY = yLine + rowPitch;
        float branchX = xCursor + cardW + hGap;
        float maxBranchEnd = 0;
        std::vector<SegmentBounds> branchInfo;
        branchInfo.reserve(slot.branches.size());

        for (const auto& br : slot.branches)
        {
            const auto bb = layoutSegment(br, branchX, branchY, rowPitch, cardW, cardH, hGap, branchGap, out);
            branchInfo.push_back(bb);
            maxBranchEnd = juce::jmax(maxBranchEnd, bb.endX);
            branchX = bb.endX + branchGap;
        }

        const float mergeX = maxBranchEnd + hGap;
        ChainStripLayoutCard mergeCard;
        mergeCard.bounds = { mergeX, yLine, cardW, cardH };
        mergeCard.slotId = 0;
        mergeCard.bypassed = false;
        mergeCard.label = "Merge";
        mergeCard.selectable = false;
        mergeCard.isMergeNode = true;
        out.cards.push_back(mergeCard);
        const auto mergeCenter = centerOf(mergeCard.bounds);

        for (const auto& bb : branchInfo)
        {
            if (bb.hadContent)
            {
                out.wires.push_back({ splitCenter, bb.firstCenter });
                out.wires.push_back({ bb.lastCenter, mergeCenter });
            }
        }

        block.lastCenter = mergeCenter;
        lastCenter = mergeCenter;
        haveLast = true;

        xCursor = mergeX + cardW + hGap;
        block.endX = xCursor;
    }

    return block;
}

void scaleLayout(ChainStripLayout& layout, float originX, float originY, float scale) noexcept
{
    for (auto& c : layout.cards)
    {
        c.bounds = juce::Rectangle<float>(
            c.bounds.getX() * scale + originX,
            c.bounds.getY() * scale + originY,
            c.bounds.getWidth() * scale,
            c.bounds.getHeight() * scale);
    }
    for (auto& w : layout.wires)
    {
        w.a.x = w.a.x * scale + originX;
        w.a.y = w.a.y * scale + originY;
        w.b.x = w.b.x * scale + originX;
        w.b.y = w.b.y * scale + originY;
    }
}

float computeMaxBottom(const ChainStripLayout& layout) noexcept
{
    float m = 0;
    for (const auto& c : layout.cards)
        m = juce::jmax(m, c.bounds.getBottom());
    return m;
}

float computeMaxRight(const ChainStripLayout& layout) noexcept
{
    float m = 0;
    for (const auto& c : layout.cards)
        m = juce::jmax(m, c.bounds.getRight());
    return m;
}

} // namespace

ChainStripLayout computeChainStripLayout(const razumov::graph::FlexSegmentDesc& root,
                                         float availableWidth,
                                         float stripHeight) noexcept
{
    ChainStripLayout layout;
    if (root.empty())
        return layout;

    const float padX = 10.f;
    const float padY = 14.f;
    const float rowPitch = 46.f;
    const float cardH = 34.f;
    const float cardW = 72.f;
    const float hGap = 10.f;
    const float branchGap = 12.f;

    const float innerW = juce::jmax(80.f, availableWidth - 2.f * padX);
    const float yLine = 0.f;

    layoutSegment(root, 0.f, yLine, rowPitch, cardW, cardH, hGap, branchGap, layout);

    const float maxR = computeMaxRight(layout);
    const float maxB = computeMaxBottom(layout);
    const float scale = (maxR > 0.f && maxR > innerW) ? (innerW / maxR) : 1.f;

    scaleLayout(layout, padX, padY, scale);

    layout.totalWidth = computeMaxRight(layout) + padX;
    layout.totalHeight = juce::jmax(computeMaxBottom(layout) + padY, stripHeight);

    return layout;
}

} // namespace razumov::ui
