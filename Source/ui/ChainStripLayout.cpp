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

struct RootModuleCounter
{
    int nextRootModuleIndex { 0 };
};

SegmentBounds layoutSegment(
    const FlexSegmentDesc& seg,
    float x,
    float yLine,
    float rowPitch,
    float cardW,
    float cardH,
    float hGap,
    float branchGap,
    ChainStripLayout& out,
    RootModuleCounter* rootCounter)
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
            if (rootCounter != nullptr)
            {
                const float p = 15.0f;
                if (yLine == 0.0f)
                {
                    const int ridx = rootCounter->nextRootModuleIndex++;
                    c.showSerialPlus = (ridx >= 1);
                    c.showParallelPlus = (ridx >= 2);
                }
                else
                {
                    c.showSerialPlus = true;
                    c.showParallelPlus = true;
                }
                if (c.showSerialPlus)
                {
                    c.serialPlusBounds = { c.bounds.getRight() + hGap * 0.5f - p * 0.5f,
                                            c.bounds.getCentreY() - p * 0.5f,
                                            p,
                                            p };
                }
                if (c.showParallelPlus)
                {
                    c.parallelPlusBounds = { c.bounds.getCentreX() - p * 0.5f,
                                              c.bounds.getBottom() + 6.0f,
                                              p,
                                              p };
                }
            }
            const auto curCenter = centerOf(c.bounds);
            out.cards.push_back(c);
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
            const auto bb = layoutSegment(br, branchX, branchY, rowPitch, cardW, cardH, hGap, branchGap, out, rootCounter);
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
        if (c.serialPlusBounds.getWidth() > 0.5f)
        {
            c.serialPlusBounds = juce::Rectangle<float>(
                c.serialPlusBounds.getX() * scale + originX,
                c.serialPlusBounds.getY() * scale + originY,
                c.serialPlusBounds.getWidth() * scale,
                c.serialPlusBounds.getHeight() * scale);
        }
        if (c.parallelPlusBounds.getWidth() > 0.5f)
        {
            c.parallelPlusBounds = juce::Rectangle<float>(
                c.parallelPlusBounds.getX() * scale + originX,
                c.parallelPlusBounds.getY() * scale + originY,
                c.parallelPlusBounds.getWidth() * scale,
                c.parallelPlusBounds.getHeight() * scale);
        }
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
    {
        m = juce::jmax(m, c.bounds.getBottom());
        if (c.parallelPlusBounds.getWidth() > 0.5f)
            m = juce::jmax(m, c.parallelPlusBounds.getBottom());
    }
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
    const float padY = 16.f;
    const float rowPitch = 56.f;
    const float cardH = 46.f;
    const float cardW = 104.f;
    const float hGap = 14.f;
    const float branchGap = 14.f;

    const float innerW = juce::jmax(80.f, availableWidth - 2.f * padX);
    const float yLine = 0.f;

    RootModuleCounter rootCounter;
    layoutSegment(root, 0.f, yLine, rowPitch, cardW, cardH, hGap, branchGap, layout, &rootCounter);

    const float maxR = computeMaxRight(layout);
    const float maxB = computeMaxBottom(layout);
    const float scale = (maxR > 0.f && maxR > innerW) ? (innerW / maxR) : 1.f;

    scaleLayout(layout, padX, padY, scale);

    layout.totalWidth = computeMaxRight(layout) + padX;
    layout.totalHeight = juce::jmax(computeMaxBottom(layout) + padY, stripHeight);

    return layout;
}

} // namespace razumov::ui
