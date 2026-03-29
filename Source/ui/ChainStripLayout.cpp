#include "ChainStripLayout.h"
#include <cmath>
#include <optional>

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
    if (from.getDistanceFrom(to) > 1.5f)
        out.wires.push_back({ from, to });
}

/** Fork on main row down to first module on a parallel row (orthogonal). */
void pushOrthogonalForkToFirst(ChainStripLayout& out, juce::Point<float> fork, juce::Point<float> firstCenter) noexcept
{
    juce::Point<float> elbow(fork.x, firstCenter.y);
    pushSerialWire(out, fork, elbow);
    pushSerialWire(out, elbow, firstCenter);
}

/** Last module on branch row up to merge on main row (orthogonal). */
void pushOrthogonalBranchToMerge(ChainStripLayout& out, juce::Point<float> from, juce::Point<float> mergeOnMain) noexcept
{
    juce::Point<float> elbow(from.x, mergeOnMain.y);
    pushSerialWire(out, from, elbow);
    pushSerialWire(out, elbow, mergeOnMain);
}

struct RootModuleCounter
{
    int nextRootModuleIndex { 0 };
};

bool shouldHidePlaceholderInStrip(const FlexSlotDesc& s, bool inBranch) noexcept
{
    if (!inBranch)
        return false;
    if (s.descType != FlexSlotDescType::Module)
        return false;
    if (s.kind != AudioNodeKind::Gain)
        return false;
    if (std::abs(s.gainLinear - 1.0f) > 1e-3f)
        return false;
    if (s.uiLabel.isNotEmpty())
        return false;
    return true;
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
    ChainStripLayout& out,
    RootModuleCounter* rootCounter,
    bool inBranch,
    const std::optional<juce::Point<float>>& branchFeed)
{
    SegmentBounds block;
    if (seg.empty())
    {
        block.endX = x;
        return block;
    }

    float xCursor = x;
    juce::Point<float> lastCenter { 0, 0 };
    bool haveLast = branchFeed.has_value();
    if (branchFeed.has_value())
        lastCenter = *branchFeed;
    bool branchFirstWireDone = false;

    for (const auto& slot : seg)
    {
        if (slot.descType == FlexSlotDescType::Module)
        {
            if (shouldHidePlaceholderInStrip(slot, inBranch))
                continue;

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

            if (haveLast)
            {
                if (branchFeed.has_value() && !branchFirstWireDone)
                {
                    pushOrthogonalForkToFirst(out, lastCenter, curCenter);
                    branchFirstWireDone = true;
                }
                else
                    pushSerialWire(out, lastCenter, curCenter);
            }

            out.cards.push_back(c);

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

        // Split: no Split/Merge cards; compact fork/join; schematic wires only.
        const float splitZone = 28.f;
        const float mergeZone = 28.f;
        const auto splitForkPoint = juce::Point<float>(xCursor + splitZone * 0.5f, yLine + cardH * 0.5f);

        if (haveLast)
            pushSerialWire(out, lastCenter, splitForkPoint);

        if (!block.hadContent)
        {
            block.firstCenter = splitForkPoint;
            block.hadContent = true;
        }

        const float branchY = yLine + rowPitch;
        float branchCursorX = xCursor + splitZone + hGap;
        float maxBranchEnd = branchCursorX;
        std::vector<SegmentBounds> branchInfo;
        branchInfo.reserve(slot.branches.size());

        for (const auto& br : slot.branches)
        {
            const auto bb = layoutSegment(br,
                                          branchCursorX,
                                          branchY,
                                          rowPitch,
                                          cardW,
                                          cardH,
                                          hGap,
                                          branchGap,
                                          out,
                                          rootCounter,
                                          true,
                                          splitForkPoint);
            branchInfo.push_back(bb);
            maxBranchEnd = juce::jmax(maxBranchEnd, bb.endX);
            branchCursorX = bb.endX + branchGap;
        }

        const float mergeX = maxBranchEnd + hGap;
        const auto mergeJunction = juce::Point<float>(mergeX + mergeZone * 0.5f, yLine + cardH * 0.5f);

        for (const auto& bb : branchInfo)
        {
            if (bb.hadContent)
                pushOrthogonalBranchToMerge(out, bb.lastCenter, mergeJunction);
        }

        block.lastCenter = mergeJunction;
        lastCenter = mergeJunction;
        haveLast = true;
        branchFirstWireDone = false;

        xCursor = mergeX + mergeZone + hGap;
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
    const float padY = 30.f;
    const float rowPitch = 68.f;
    const float cardH = 56.f;
    const float cardW = 124.f;
    const float hGap = 16.f;
    const float branchGap = 16.f;

    const float innerW = juce::jmax(80.f, availableWidth - 2.f * padX);
    const float yLine = 0.f;

    RootModuleCounter rootCounter;
    layoutSegment(root, 0.f, yLine, rowPitch, cardW, cardH, hGap, branchGap, layout, &rootCounter, false, {});

    const float maxR = computeMaxRight(layout);
    const float maxB = computeMaxBottom(layout);
    const float scale = (maxR > 0.f && maxR > innerW) ? (innerW / maxR) : 1.f;

    scaleLayout(layout, padX, padY, scale);

    layout.totalWidth = computeMaxRight(layout) + padX;
    layout.totalHeight = juce::jmax(computeMaxBottom(layout) + padY, stripHeight);

    return layout;
}

} // namespace razumov::ui
