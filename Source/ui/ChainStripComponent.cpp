#include "ChainStripComponent.h"
#include "DesignTokens.h"
#include "EditorVisualAssets.h"
#include "PluginProcessor.h"
#include "dsp/graph/FlexGraphDesc.h"
#include <cmath>

namespace razumov::ui
{
namespace
{
void drawWire(juce::Graphics& g, juce::Point<float> a, juce::Point<float> b, juce::Colour c)
{
    g.setColour(c);
    if (std::abs(a.y - b.y) < 2.0f && b.x > a.x + 3.0f)
        g.drawArrow(juce::Line<float>(a.x, a.y, b.x, b.y), 1.75f, 4.5f, 4.5f);
    else
        g.drawLine(juce::Line<float>(a.x, a.y, b.x, b.y), 1.65f);
}

void drawDeleteAffordance(juce::Graphics& g, juce::Rectangle<float> r, juce::Colour ink)
{
    g.setColour(juce::Colour(0x18000000));
    g.fillEllipse(r);
    g.setColour(ink.withAlpha(0.82f));
    g.drawEllipse(r.reduced(0.5f), 1.0f);
    const auto c = r.getCentre();
    const float h = r.getWidth() * 0.28f;
    g.drawLine(c.x - h, c.y - h, c.x + h, c.y + h, 1.45f);
    g.drawLine(c.x - h, c.y + h, c.x + h, c.y - h, 1.45f);
}

void drawPlusAffordance(juce::Graphics& g, juce::Rectangle<float> r, juce::Colour rim, juce::Colour fill)
{
    g.setColour(fill);
    g.fillRoundedRectangle(r, 4.5f);
    g.setColour(rim);
    g.drawRoundedRectangle(r.reduced(0.5f), 4.5f, 1.0f);
    g.setColour(rim.brighter(0.25f));
    const auto ctr = r.getCentre();
    const float half = 4.2f;
    g.drawLine(ctr.x - half, ctr.y, ctr.x + half, ctr.y, 1.65f);
    g.drawLine(ctr.x, ctr.y - half, ctr.x, ctr.y + half, 1.65f);
}

void drawSimpleMicIcon(juce::Graphics& g, juce::Rectangle<float> r, juce::Colour ink)
{
    const float cx = r.getCentreX();
    const float w = juce::jmin(r.getWidth(), r.getHeight()) * 0.42f;
    const float top = r.getY() + 2.0f;
    juce::Path body;
    body.addRoundedRectangle(cx - w * 0.5f, top, w, w * 1.35f, w * 0.48f, w * 0.48f);
    g.setColour(ink.withAlpha(0.85f));
    g.fillPath(body);
    g.setColour(ink.brighter(0.55f).withAlpha(0.4f));
    for (int i = 0; i < 3; ++i)
    {
        const float yy = top + w * 0.32f + (float) i * w * 0.22f;
        g.drawLine(cx - w * 0.28f, yy, cx + w * 0.28f, yy, 1.1f);
    }
    const float uY = top + w * 1.35f + 2.0f;
    g.setColour(ink.withAlpha(0.72f));
    g.drawLine(cx - w * 0.48f, uY, cx - w * 0.48f, uY + w * 0.32f, 2.0f);
    g.drawLine(cx + w * 0.48f, uY, cx + w * 0.48f, uY + w * 0.32f, 2.0f);
    g.drawLine(cx - w * 0.58f, uY + w * 0.32f, cx + w * 0.58f, uY + w * 0.32f, 2.0f);
}

void drawBypassPill(juce::Graphics& g, juce::Rectangle<float> pill, bool bypassed, bool accentGlow,
                    juce::Colour accent, juce::Colour textPri, juce::Colour textSec)
{
    const auto ctr = pill.getCentre();
    const float r = pill.getWidth() * 0.5f;
    if (!bypassed && accentGlow)
    {
        for (int i = 4; i >= 1; --i)
        {
            g.setColour(accent.withAlpha(0.045f + 0.028f * (float) i));
            g.fillEllipse(ctr.x - r - (float) i * 1.5f, ctr.y - r - (float) i * 1.5f,
                          (r + (float) i * 1.5f) * 2.0f, (r + (float) i * 1.5f) * 2.0f);
        }
    }
    g.setColour(bypassed ? juce::Colour(0x28000000) : juce::Colour(0x14000000));
    g.fillEllipse(pill);
    g.setColour(bypassed ? textSec.withAlpha(0.55f) : textPri.withAlpha(0.85f));
    g.setFont(juce::FontOptions(8.5f, juce::Font::bold));
    g.drawText("b", pill, juce::Justification::centred);
    g.setColour(bypassed ? textSec.withAlpha(0.55f) : accent.withAlpha(0.85f));
    g.drawEllipse(pill.reduced(0.5f), 1.0f);
}

bool firstSelectableSlot(const ChainStripLayout& layout, uint32_t& outId) noexcept
{
    for (const auto& c : layout.cards)
    {
        if (c.selectable && c.slotId != 0)
        {
            outId = c.slotId;
            return true;
        }
    }
    return false;
}

bool containsSlot(const ChainStripLayout& layout, uint32_t id) noexcept
{
    if (id == 0)
        return false;
    for (const auto& c : layout.cards)
    {
        if (c.selectable && c.slotId == id)
            return true;
    }
    return false;
}
} // namespace

ChainStripComponent::ChainStripComponent(RazumovVocalChainAudioProcessor& processor)
    : processor_(processor)
{
    setInterceptsMouseClicks(true, true);
    syncFromProcessor();
}

ChainStripComponent::~ChainStripComponent() = default;

juce::Rectangle<float> ChainStripComponent::bypassPillBounds(juce::Rectangle<float> card) noexcept
{
    const float s = 13.f;
    return { card.getX() + 4.f, card.getBottom() - s - 4.f, s, s };
}

uint32_t ChainStripComponent::hitTestCardSlotAt(juce::Point<float> p) const noexcept
{
    for (auto it = layout_.cards.rbegin(); it != layout_.cards.rend(); ++it)
    {
        const auto& c = *it;
        if (!c.selectable || c.slotId == 0)
            continue;
        if (c.bounds.expanded(1.0f).contains(p))
            return c.slotId;
    }
    return 0;
}

void ChainStripComponent::syncFromProcessor()
{
    rebuildLayout();

    if (selectedSlotId_ == 0)
    {
        uint32_t first = 0;
        if (firstSelectableSlot(layout_, first))
            selectedSlotId_ = first;
    }
    else if (!containsSlot(layout_, selectedSlotId_))
    {
        uint32_t first = 0;
        if (firstSelectableSlot(layout_, first))
            selectedSlotId_ = first;
        else
            selectedSlotId_ = 0;
    }

    repaint();
}

void ChainStripComponent::setSelectedSlotId(uint32_t id) noexcept
{
    selectedSlotId_ = id;
    repaint();
}

void ChainStripComponent::resized()
{
    rebuildLayout();
    repaint();
}

void ChainStripComponent::rebuildLayout()
{
    layout_ = computeChainStripLayout(processor_.getGraphDesc(), (float) getWidth(), (float) getHeight());
    const float ds = 14.f;
    for (auto& c : layout_.cards)
    {
        c.showDeleteButton = false;
        if (c.selectable && c.slotId != 0 && !c.isMergeNode
            && !razumov::graph::isProtectedFrontRootModuleSlot(processor_.getGraphDesc(), c.slotId))
        {
            c.showDeleteButton = true;
            c.deleteButtonBounds = { c.bounds.getRight() - ds - 4.f, c.bounds.getY() + 4.f, ds, ds };
        }
    }
}

void ChainStripComponent::paint(juce::Graphics& g)
{
    using namespace razumov::ui::tokens::argb;
    const juce::Colour panelBg(backgroundChainStrip);
    const juce::Colour cardFill(backgroundNode);
    const juce::Colour accent(accentSignal);
    const juce::Colour textPri(textPrimary);
    const juce::Colour textSec(textSecondary);
    const juce::Colour selOutline(accentSelection);

    g.fillAll(panelBg);

    if (layout_.cards.empty())
    {
        auto titleBar = getLocalBounds().reduced(12, 8).removeFromTop(18);
        g.setColour(juce::Colour(backgroundNode).withAlpha(0.92f));
        g.fillRoundedRectangle(titleBar.toFloat(), 4.f);
        g.setColour(textSec);
        g.setFont(juce::FontOptions(11.5f, juce::Font::bold));
        g.drawText("Signal path", titleBar, juce::Justification::centredLeft);
        return;
    }

    const juce::Colour wireCol = accent.withAlpha(0.88f);
    for (const auto& w : layout_.wires)
        drawWire(g, w.a, w.b, wireCol);

    const float corner = 10.0f;

    for (const auto& c : layout_.cards)
    {
        const bool sel = (c.slotId != 0 && c.slotId == selectedSlotId_);
        if (sel && c.selectable)
            drawRoundedRectGlow(g, c.bounds, corner, selOutline, 5);
    }

    for (const auto& c : layout_.cards)
    {
        const bool sel = (c.slotId != 0 && c.slotId == selectedSlotId_);
        const juce::Rectangle<float> card = c.bounds;

        g.setColour(juce::Colour(shadowElevated));
        g.fillRoundedRectangle(card.translated(0.0f, 3.0f), corner + 1.0f);
        g.setColour(juce::Colour(0x42000000));
        g.fillRoundedRectangle(card.translated(0.0f, 2.0f).reduced(0.5f), corner);

        g.setColour(c.bypassed ? cardFill.brighter(0.08f) : cardFill);
        g.fillRoundedRectangle(card, corner);

        if (sel)
        {
            g.setColour(selOutline.withAlpha(0.95f));
            g.drawRoundedRectangle(card.reduced(0.5f), corner, 2.0f);
        }
        else
        {
            g.setColour(accent.withAlpha(0.45f));
            g.drawRoundedRectangle(card.reduced(0.5f), corner, 1.0f);
        }

        g.setColour(c.bypassed ? textSec : textPri);
        const float fs = juce::jmin(11.5f, juce::jmax(8.5f, card.getWidth() * 0.09f));
        const bool micCard = (!c.isMergeNode && c.label.containsIgnoreCase("Mic correction"));
        auto labelArea = card.reduced(5.0f);
        labelArea.removeFromBottom(16.f);
        if (micCard)
        {
            auto inner = labelArea;
            const float iconH = inner.getHeight() * 0.58f;
            auto iconR = inner.withHeight(iconH);
            drawSimpleMicIcon(g, iconR, c.bypassed ? textSec : textPri);
            auto cap = inner.withTrimmedTop(iconH + 2.0f);
            g.setFont(juce::FontOptions(fs * 0.78f, juce::Font::bold));
            g.drawText("Mic correction", cap, juce::Justification::centred);
        }
        else
        {
            g.setFont(juce::FontOptions(fs, juce::Font::bold));
            g.drawText(c.label, labelArea, juce::Justification::centred);
        }

        if (c.slotId != 0 && c.selectable)
        {
            const auto pill = bypassPillBounds(card);
            drawBypassPill(g, pill, c.bypassed, true, accent, textPri, textSec);
        }

        if (c.showDeleteButton && c.deleteButtonBounds.getWidth() > 0.5f)
        {
            const bool delHover = deleteHoverSlotId_ == c.slotId;
            drawDeleteAffordance(g, c.deleteButtonBounds, delHover ? accent : textSec);
        }
    }

    const juce::Colour plusFill = juce::Colour(backgroundNode).brighter(0.12f);
    const juce::Colour plusRim = accent.withAlpha(0.75f);
    for (const auto& c : layout_.cards)
    {
        if (c.showSerialPlus && c.serialPlusBounds.getWidth() > 0.5f)
            drawPlusAffordance(g, c.serialPlusBounds, plusRim, plusFill.withAlpha(0.92f));
        if (c.showParallelPlus && c.parallelPlusBounds.getWidth() > 0.5f)
            drawPlusAffordance(g, c.parallelPlusBounds, plusRim, plusFill.withAlpha(0.85f));
    }

    auto titleBar = getLocalBounds().reduced(12, 8).removeFromTop(18);
    g.setColour(juce::Colour(backgroundNode).withAlpha(0.94f));
    g.fillRoundedRectangle(titleBar.toFloat(), 5.f);
    g.setColour(juce::Colour(borderModulePanel).withAlpha(0.65f));
    g.drawRoundedRectangle(titleBar.toFloat().reduced(0.5f), 5.f, 1.0f);
    g.setColour(textSec);
    g.setFont(juce::FontOptions(11.5f, juce::Font::bold));
    g.drawText("Signal path", titleBar.reduced(6, 0), juce::Justification::centredLeft);

    if (dragging_ && dragSlotId_ != 0)
    {
        for (const auto& c : layout_.cards)
        {
            if (c.slotId != dragSlotId_)
                continue;
            const float dx = dragLast_.x - dragStart_.x;
            const float dy = dragLast_.y - dragStart_.y;
            auto ghost = c.bounds.translated(dx, dy);
            g.setColour(cardFill.withAlpha(0.48f));
            g.fillRoundedRectangle(ghost, corner);
            g.setColour(accent.withAlpha(0.72f));
            g.drawRoundedRectangle(ghost.reduced(0.5f), corner, 1.5f);
            g.setColour(textPri.withAlpha(0.78f));
            const float fs = juce::jmin(11.5f, juce::jmax(8.5f, ghost.getWidth() * 0.09f));
            g.setFont(juce::FontOptions(fs, juce::Font::bold));
            g.drawText(c.label, ghost.reduced(6.f), juce::Justification::centred);
            break;
        }
    }
}

void ChainStripComponent::mouseDown(const juce::MouseEvent& e)
{
    const auto p = e.position;
    dragging_ = false;
    dragSlotId_ = 0;

    if (e.mods.isPopupMenu())
    {
        const auto screen = e.getScreenPosition();
        for (auto it = layout_.cards.rbegin(); it != layout_.cards.rend(); ++it)
        {
            const auto& c = *it;
            if (c.slotId == 0)
                continue;
            if (c.showSerialPlus && c.serialPlusBounds.getWidth() > 0.5f && c.serialPlusBounds.contains(p))
            {
                if (onChainContextMenu)
                    onChainContextMenu(ChainContextTarget::SerialPlus, c.slotId, screen);
                return;
            }
            if (c.showParallelPlus && c.parallelPlusBounds.getWidth() > 0.5f && c.parallelPlusBounds.contains(p))
            {
                if (onChainContextMenu)
                    onChainContextMenu(ChainContextTarget::ParallelPlus, c.slotId, screen);
                return;
            }
        }
        for (auto it = layout_.cards.rbegin(); it != layout_.cards.rend(); ++it)
        {
            const auto& c = *it;
            if (!c.selectable || c.slotId == 0)
                continue;
            if (c.bounds.expanded(1.0f).contains(p))
            {
                if (onChainContextMenu)
                    onChainContextMenu(ChainContextTarget::ModuleCard, c.slotId, screen);
                return;
            }
        }
        return;
    }

    for (auto it = layout_.cards.rbegin(); it != layout_.cards.rend(); ++it)
    {
        const auto& c = *it;
        if (!c.showDeleteButton)
            continue;
        if (c.deleteButtonBounds.contains(p))
        {
            if (onRequestRemoveSlot)
                onRequestRemoveSlot(c.slotId);
            syncFromProcessor();
            return;
        }
    }

    for (auto it = layout_.cards.rbegin(); it != layout_.cards.rend(); ++it)
    {
        const auto& c = *it;
        if (c.slotId == 0 || !c.selectable)
            continue;
        if (bypassPillBounds(c.bounds).contains(p))
        {
            processor_.setSlotBypassForId(c.slotId, !c.bypassed);
            syncFromProcessor();
            if (onSlotSelected)
                onSlotSelected(c.slotId);
            return;
        }
    }

    for (auto it = layout_.cards.rbegin(); it != layout_.cards.rend(); ++it)
    {
        const auto& c = *it;
        if (c.slotId == 0)
            continue;
        if (c.showParallelPlus && c.parallelPlusBounds.getWidth() > 0.5f && c.parallelPlusBounds.contains(p))
        {
            if (onRequestParallelBranch)
                onRequestParallelBranch(c.slotId);
            return;
        }
        if (c.showSerialPlus && c.serialPlusBounds.getWidth() > 0.5f && c.serialPlusBounds.contains(p))
        {
            if (onRequestAddSerialAfter)
                onRequestAddSerialAfter(c.slotId);
            return;
        }
    }

    for (const auto& c : layout_.cards)
    {
        if (!c.selectable || c.slotId == 0)
            continue;
        if (c.bounds.expanded(1.0f).contains(p))
        {
            dragSlotId_ = c.slotId;
            dragStart_ = p;
            dragLast_ = p;
            selectedSlotId_ = c.slotId;
            if (onSlotSelected)
                onSlotSelected(selectedSlotId_);
            repaint();
            return;
        }
    }
}

void ChainStripComponent::mouseDrag(const juce::MouseEvent& e)
{
    if (dragSlotId_ == 0)
        return;
    dragLast_ = e.position;
    if (dragStart_.getDistanceFrom(e.position) > 6.f)
        dragging_ = true;
    if (dragging_)
        repaint();
}

void ChainStripComponent::mouseUp(const juce::MouseEvent& e)
{
    if (dragging_ && dragSlotId_ != 0 && onRequestSwapRootModules)
    {
        const uint32_t target = hitTestCardSlotAt(e.position);
        if (target != 0 && target != dragSlotId_)
            onRequestSwapRootModules(dragSlotId_, target);
    }
    dragging_ = false;
    dragSlotId_ = 0;
    syncFromProcessor();
    repaint();
}

void ChainStripComponent::mouseMove(const juce::MouseEvent& e)
{
    const auto p = e.position;
    uint32_t delHover = 0;
    for (const auto& c : layout_.cards)
    {
        if (c.showDeleteButton && c.deleteButtonBounds.contains(p))
        {
            delHover = c.slotId;
            break;
        }
    }
    if (delHover != deleteHoverSlotId_)
    {
        deleteHoverSlotId_ = delHover;
        repaint();
    }

    for (const auto& c : layout_.cards)
    {
        if (c.slotId == 0)
            continue;
        if (c.showDeleteButton && c.deleteButtonBounds.contains(p))
        {
            setMouseCursor(juce::MouseCursor::PointingHandCursor);
            return;
        }
        if (bypassPillBounds(c.bounds).contains(p))
        {
            setMouseCursor(juce::MouseCursor::PointingHandCursor);
            return;
        }
        if (c.showParallelPlus && c.parallelPlusBounds.contains(p))
        {
            setMouseCursor(juce::MouseCursor::PointingHandCursor);
            return;
        }
        if (c.showSerialPlus && c.serialPlusBounds.contains(p))
        {
            setMouseCursor(juce::MouseCursor::PointingHandCursor);
            return;
        }
        if (c.selectable && c.bounds.contains(p))
        {
            setMouseCursor(juce::MouseCursor::PointingHandCursor);
            return;
        }
    }
    setMouseCursor(juce::MouseCursor::NormalCursor);
}

} // namespace razumov::ui
