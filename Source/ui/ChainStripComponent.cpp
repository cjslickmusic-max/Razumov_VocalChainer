#include "ChainStripComponent.h"
#include "PluginProcessor.h"
#include "params/ParamIDs.h"
#include <cmath>

namespace razumov::ui
{
namespace
{
void drawWire(juce::Graphics& g, juce::Point<float> a, juce::Point<float> b, juce::Colour c)
{
    g.setColour(c);
    if (std::abs(a.y - b.y) < 2.0f && b.x > a.x + 3.0f)
        g.drawArrow(juce::Line<float>(a.x, a.y, b.x, b.y), 1.5f, 4.5f, 4.5f);
    else
        g.drawLine(juce::Line<float>(a.x, a.y, b.x, b.y), 1.35f);
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
    , apvts_(processor.getAPVTS())
{
    apvts_.addParameterListener(razumov::params::chainProfile, this);
    setInterceptsMouseClicks(true, true);
    syncFromProcessor();
}

ChainStripComponent::~ChainStripComponent()
{
    apvts_.removeParameterListener(razumov::params::chainProfile, this);
}

void ChainStripComponent::parameterChanged(const juce::String& parameterID, float newValue)
{
    juce::ignoreUnused(newValue);
    if (parameterID == razumov::params::chainProfile)
        syncFromProcessor();
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
}

void ChainStripComponent::paint(juce::Graphics& g)
{
    const juce::Colour panelBg(0xff252830);
    const juce::Colour cardFill(0xff1e2229);
    const juce::Colour mergeFill(0xff222630);
    const juce::Colour accent(0xff6c9fd2);
    const juce::Colour textPri(0xffe8eaed);
    const juce::Colour textSec(0xff8892a0);
    const juce::Colour selOutline(0xff9fd2ff);

    g.fillAll(panelBg);

    auto header = getLocalBounds().reduced(10, 6);
    g.setColour(textSec);
    g.setFont(juce::FontOptions(11.0f));
    g.drawText("Signal path", header.removeFromTop(14), juce::Justification::centredLeft);

    if (layout_.cards.empty())
        return;

    const juce::Colour wireCol = accent.withAlpha(0.72f);
    for (const auto& w : layout_.wires)
        drawWire(g, w.a, w.b, wireCol);

    const float corner = 6.0f;

    for (const auto& c : layout_.cards)
    {
        const bool sel = (c.slotId != 0 && c.slotId == selectedSlotId_);
        const juce::Rectangle<float> card = c.bounds;

        g.setColour(c.isMergeNode ? mergeFill : (c.bypassed ? cardFill.brighter(0.08f) : cardFill));
        g.fillRoundedRectangle(card, corner);

        if (sel)
        {
            g.setColour(selOutline.withAlpha(0.95f));
            g.drawRoundedRectangle(card.reduced(0.5f), corner, 2.0f);
        }
        else
        {
            g.setColour(accent.withAlpha(0.55f));
            g.drawRoundedRectangle(card.reduced(0.5f), corner, 1.0f);
        }

        g.setColour(c.bypassed ? textSec : textPri);
        const float fs = juce::jmin(12.5f, juce::jmax(8.5f, card.getWidth() * 0.18f));
        g.setFont(juce::FontOptions(fs, juce::Font::bold));
        g.drawText(c.label, card.reduced(3.0f), juce::Justification::centred);

        if (c.bypassed)
        {
            g.setColour(juce::Colour(0xffe89868));
            g.setFont(juce::FontOptions(9.0f, juce::Font::bold));
            g.drawText("BYP", card.reduced(4.0f).removeFromBottom(11.0f), juce::Justification::bottomRight);
        }
    }
}

void ChainStripComponent::mouseDown(const juce::MouseEvent& e)
{
    const auto p = e.position;
    for (const auto& c : layout_.cards)
    {
        if (!c.selectable || c.slotId == 0)
            continue;
        if (c.bounds.expanded(1.0f).contains(p))
        {
            if (e.getNumberOfClicks() == 2)
            {
                processor_.setSlotBypassForId(c.slotId, !c.bypassed);
                syncFromProcessor();
                repaint();
                return;
            }
            selectedSlotId_ = c.slotId;
            if (onSlotSelected)
                onSlotSelected(selectedSlotId_);
            repaint();
            return;
        }
    }
}

} // namespace razumov::ui
