#include "PluginEditor.h"
#include "PluginProcessor.h"

RazumovVocalChainAudioProcessorEditor::RazumovVocalChainAudioProcessorEditor(
    RazumovVocalChainAudioProcessor& p)
    : AudioProcessorEditor(&p)
    , processor(p)
{
    setSize(560, 420);
    setResizable(true, true);
    setResizeLimits(400, 300, 2000, 1600);
}

void RazumovVocalChainAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1a1d23));
    g.setColour(juce::Colours::white);
    g.setFont(juce::FontOptions(20.0f, juce::Font::bold));
    g.drawText("Razumov Vocal Chain", getLocalBounds().reduced(16), juce::Justification::topLeft);
    g.setFont(juce::FontOptions(14.0f));
    g.setColour(juce::Colour(0xffaab4c0));
    g.drawText("v0.2.0 graph: serial + parallel merge (DSP path active)", getLocalBounds().reduced(16),
               juce::Justification::centred);
}

void RazumovVocalChainAudioProcessorEditor::resized() {}
