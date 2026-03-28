#include "ChainStripComponent.h"
#include "PluginProcessor.h"
#include "params/ParamIDs.h"

namespace razumov::ui
{

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
    items_ = processor_.getChainStripItems();
    if (selectedSlotId_ == 0 && !items_.empty())
        selectedSlotId_ = items_.front().slotId;
    else
    {
        bool found = false;
        for (const auto& it : items_)
        {
            if (it.slotId == selectedSlotId_)
            {
                found = true;
                break;
            }
        }
        if (!found && !items_.empty())
            selectedSlotId_ = items_.front().slotId;
    }
    rebuildLayout();
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
}

void ChainStripComponent::rebuildLayout()
{
    hitRects_.clear();
    if (items_.empty())
        return;

    auto area = getLocalBounds().reduced(10, 6).withTrimmedTop(14);
    const int n = (int) items_.size();
    if (n <= 0)
        return;

    int maxRow = 0;
    for (const auto& it : items_)
        maxRow = juce::jmax(maxRow, it.row);

    const float gap = 10.0f;
    const float arrowW = 12.0f;
    const float totalArrows = (float)(n - 1) * arrowW;
    const float avail = (float) area.getWidth() - totalArrows;
    float cardW = juce::jmax(48.0f, avail / (float) n);
    cardW = juce::jmin(cardW, 96.0f);

    float x = (float) area.getX();
    const float midY = (float) area.getCentreY() + 4.0f;
    const float cardH = 40.0f;
    const float rowPitch = 44.0f;
    const float centerOffset = -0.5f * (float) maxRow * rowPitch;

    for (int i = 0; i < n; ++i)
    {
        const int r = juce::jlimit(0, maxRow, items_[(size_t) i].row);
        const float yTop = midY + centerOffset + (float) r * rowPitch - cardH * 0.5f;
        auto card = juce::Rectangle<float>(x, yTop, cardW, cardH);
        hitRects_.push_back(card.toNearestInt());
        x = card.getRight();
        if (i < n - 1)
            x += arrowW;
    }
}

void ChainStripComponent::paint(juce::Graphics& g)
{
    const juce::Colour panelBg(0xff252830);
    const juce::Colour cardFill(0xff1e2229);
    const juce::Colour accent(0xff6c9fd2);
    const juce::Colour textPri(0xffe8eaed);
    const juce::Colour textSec(0xff8892a0);
    const juce::Colour selOutline(0xff9fd2ff);

    g.fillAll(panelBg);

    auto area = getLocalBounds().reduced(10, 6);
    g.setColour(textSec);
    g.setFont(juce::FontOptions(11.0f));
    g.drawText("Signal path", area.removeFromTop(14), juce::Justification::centredLeft);

    const int n = (int) items_.size();
    if (n <= 0)
        return;

    int maxRow = 0;
    for (const auto& it : items_)
        maxRow = juce::jmax(maxRow, it.row);

    const float gap = 10.0f;
    const float arrowW = 12.0f;
    const float totalArrows = (float)(n - 1) * arrowW;
    const float avail = (float) area.getWidth() - totalArrows;
    float cardW = juce::jmax(48.0f, avail / (float) n);
    cardW = juce::jmin(cardW, 96.0f);

    float x = (float) area.getX();
    const float midY = (float) area.getCentreY() + 4.0f;
    const float cardH = 40.0f;
    const float rowPitch = 44.0f;
    const float centerOffset = -0.5f * (float) maxRow * rowPitch;
    const float corner = 6.0f;

    for (int i = 0; i < n; ++i)
    {
        const int r = juce::jlimit(0, maxRow, items_[(size_t) i].row);
        const float yTop = midY + centerOffset + (float) r * rowPitch - cardH * 0.5f;
        auto card = juce::Rectangle<float>(x, yTop, cardW, cardH);
        const bool sel = (items_[(size_t) i].slotId == selectedSlotId_);
        const bool byp = items_[(size_t) i].bypassed;

        g.setColour(byp ? cardFill.brighter(0.08f) : cardFill);
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

        g.setColour(byp ? textSec : textPri);
        g.setFont(juce::FontOptions(juce::jmin(13.0f, cardW * 0.22f), juce::Font::bold));
        g.drawText(items_[(size_t) i].label, card.reduced(4.0f), juce::Justification::centred);

        x = card.getRight();

        if (i < n - 1)
        {
            g.setColour(accent.withAlpha(0.75f));
            g.drawArrow(juce::Line<float>(x + 4.0f, midY, x + arrowW - 2.0f, midY), 2.2f, 5.0f, 5.0f);
            x += arrowW;
        }
    }
}

void ChainStripComponent::mouseDown(const juce::MouseEvent& e)
{
    const juce::Point<int> p = e.getPosition();
    for (int i = 0; i < (int) hitRects_.size(); ++i)
    {
        if (hitRects_[(size_t) i].contains(p) && i < (int) items_.size())
        {
            selectedSlotId_ = items_[(size_t) i].slotId;
            if (onSlotSelected)
                onSlotSelected(selectedSlotId_);
            repaint();
            return;
        }
    }
}

} // namespace razumov::ui
