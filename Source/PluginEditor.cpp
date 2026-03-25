#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "params/ParamIDs.h"

RazumovVocalChainAudioProcessorEditor::RazumovVocalChainAudioProcessorEditor(
    RazumovVocalChainAudioProcessor& p)
    : AudioProcessorEditor(&p)
    , processor(p)
{
    setSize(560, 420);
    setResizable(true, true);
    setResizeLimits(400, 300, 2000, 1600);

    gainSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    gainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    gainSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff6c9fd2));
    addAndMakeVisible(gainSlider);

    lowpassSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    lowpassSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    lowpassSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff8fd28c));
    addAndMakeVisible(lowpassSlider);

    gainLabel.setText("Gain (dB)", juce::dontSendNotification);
    gainLabel.setJustificationType(juce::Justification::centred);
    gainLabel.attachToComponent(&gainSlider, false);
    addAndMakeVisible(gainLabel);

    lowpassLabel.setText("Lowpass (Hz)", juce::dontSendNotification);
    lowpassLabel.setJustificationType(juce::Justification::centred);
    lowpassLabel.attachToComponent(&lowpassSlider, false);
    addAndMakeVisible(lowpassLabel);

    auto& apvts = processor.getAPVTS();
    gainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, razumov::params::gainDb, gainSlider);
    lowpassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, razumov::params::lowpassHz, lowpassSlider);
}

void RazumovVocalChainAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1a1d23));
    g.setColour(juce::Colours::white);
    g.setFont(juce::FontOptions(20.0f, juce::Font::bold));
    g.drawText("Razumov Vocal Chain", getLocalBounds().reduced(16), juce::Justification::topLeft);
    g.setFont(juce::FontOptions(14.0f));
    g.setColour(juce::Colour(0xffaab4c0));
    g.drawText("v0.3.0 APVTS + graph", getLocalBounds().reduced(16), juce::Justification::centred);
}

void RazumovVocalChainAudioProcessorEditor::resized()
{
    auto r = getLocalBounds().reduced(16);
    r.removeFromTop(44);

    auto row = r.removeFromTop(160);
    gainSlider.setBounds(row.removeFromLeft(160).reduced(8));
    row.removeFromLeft(24);
    lowpassSlider.setBounds(row.removeFromLeft(160).reduced(8));
}
