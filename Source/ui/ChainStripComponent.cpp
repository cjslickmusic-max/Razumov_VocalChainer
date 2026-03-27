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
}

ChainStripComponent::~ChainStripComponent()
{
    apvts_.removeParameterListener(razumov::params::chainProfile, this);
}

void ChainStripComponent::parameterChanged(const juce::String& parameterID, float newValue)
{
    juce::ignoreUnused(newValue);
    if (parameterID == razumov::params::chainProfile)
        repaint();
}

void ChainStripComponent::paint(juce::Graphics& g)
{
    const juce::Colour panelBg(0xff252830);
    const juce::Colour cardFill(0xff1e2229);
    const juce::Colour accent(0xff6c9fd2);
    const juce::Colour textPri(0xffe8eaed);
    const juce::Colour textSec(0xff8892a0);

    g.fillAll(panelBg);

    auto area = getLocalBounds().reduced(10, 6);
    g.setColour(textSec);
    g.setFont(juce::FontOptions(11.0f));
    g.drawText("Signal path", area.removeFromTop(14), juce::Justification::centredLeft);

    const juce::StringArray labels = processor_.getChainStripLabelArray();
    const int n = labels.size();
    if (n <= 0)
        return;

    const float gap = 10.0f;
    const float arrowW = 12.0f;
    const float totalArrows = (float)(n - 1) * arrowW;
    const float avail = (float)area.getWidth() - totalArrows;
    float cardW = juce::jmax(48.0f, avail / (float)n);
    cardW = juce::jmin(cardW, 96.0f);

    float x = (float)area.getX();
    const float midY = (float)area.getCentreY() + 4.0f;
    const float cardH = 40.0f;
    const float corner = 6.0f;

    for (int i = 0; i < n; ++i)
    {
        auto card = juce::Rectangle<float>(x, midY - cardH * 0.5f, cardW, cardH);
        g.setColour(cardFill);
        g.fillRoundedRectangle(card, corner);
        g.setColour(accent.withAlpha(0.55f));
        g.drawRoundedRectangle(card.reduced(0.5f), corner, 1.0f);

        g.setColour(textPri);
        g.setFont(juce::FontOptions(juce::jmin(13.0f, cardW * 0.22f), juce::Font::bold));
        g.drawText(labels[i], card.reduced(4.0f), juce::Justification::centred);

        x = card.getRight();

        if (i < n - 1)
        {
            g.setColour(accent.withAlpha(0.75f));
            g.drawArrow(juce::Line<float>(x + 4.0f, midY, x + arrowW - 2.0f, midY), 2.2f, 5.0f, 5.0f);
            x += arrowW;
        }
    }
}

} // namespace razumov::ui
