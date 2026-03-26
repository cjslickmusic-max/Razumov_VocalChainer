#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "params/ParamIDs.h"

void RazumovVocalChainAudioProcessorEditor::styleRotary(juce::Slider& s)
{
    s.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 72, 16);
}

RazumovVocalChainAudioProcessorEditor::RazumovVocalChainAudioProcessorEditor(
    RazumovVocalChainAudioProcessor& p)
    : AudioProcessorEditor(&p)
    , processor(p)
    , chainStrip(processor.getAPVTS())
{
    setSize(880, 700);
    setResizable(true, true);
    setResizeLimits(640, 480, 2000, 1600);

    presetLabel.setText("Preset", juce::dontSendNotification);
    presetLabel.setJustificationType(juce::Justification::centredRight);
    presetLabel.setColour(juce::Label::textColourId, juce::Colour(0xffaab4c0));
    addAndMakeVisible(presetLabel);

    for (int i = 0; i < processor.getNumPrograms(); ++i)
        presetCombo.addItem(processor.getProgramName(i), i + 1);
    presetCombo.setSelectedId(processor.getCurrentProgram() + 1, juce::dontSendNotification);
    presetCombo.onChange = [this] {
        const int id = presetCombo.getSelectedId();
        if (id > 0)
            processor.applyFactoryPreset(id - 1);
    };
    addAndMakeVisible(presetCombo);

    addAndMakeVisible(chainStrip);

    addAndMakeVisible(viewport);
    viewport.setScrollBarsShown(true, false);

    auto& apvts = processor.getAPVTS();

    chainLabel.setText("Chain", juce::dontSendNotification);
    chainLabel.setJustificationType(juce::Justification::centredRight);
    chainLabel.setColour(juce::Label::textColourId, juce::Colour(0xffaab4c0));
    addAndMakeVisible(chainLabel);
    addAndMakeVisible(chainCombo);
    chainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvts, razumov::params::chainProfile, chainCombo);

    auto addKnob = [this](juce::Slider& s, juce::Colour c) {
        styleRotary(s);
        s.setColour(juce::Slider::rotarySliderFillColourId, c);
        content.addAndMakeVisible(s);
    };

    auto setupMacro = [&](juce::Slider& s, juce::Label& lbl, const char* name, juce::Colour c) {
        lbl.setText(name, juce::dontSendNotification);
        lbl.setJustificationType(juce::Justification::centred);
        lbl.setColour(juce::Label::textColourId, juce::Colour(0xffaab4c0));
        content.addAndMakeVisible(lbl);
        addKnob(s, c);
    };

    setupMacro(macroGlueSlider, macroGlueLabel, "Glue", juce::Colour(0xffc080a0));
    setupMacro(macroAirSlider, macroAirLabel, "Air", juce::Colour(0xff80c0c8));
    setupMacro(macroSibilanceSlider, macroSibilanceLabel, "Sibil", juce::Colour(0xffd0a868));
    setupMacro(macroPresenceSlider, macroPresenceLabel, "Presence", juce::Colour(0xffa8d080));

    macroGlueAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, razumov::params::macroGlue, macroGlueSlider);
    macroAirAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, razumov::params::macroAir, macroAirSlider);
    macroSibilanceAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, razumov::params::macroSibilance, macroSibilanceSlider);
    macroPresenceAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, razumov::params::macroPresence, macroPresenceSlider);

    micBypassBtn.setButtonText("Mic bypass");
    micBypassBtn.setClickingTogglesState(true);
    content.addAndMakeVisible(micBypassBtn);

    spectralBypassBtn.setButtonText("Spectral bypass");
    spectralBypassBtn.setClickingTogglesState(true);
    content.addAndMakeVisible(spectralBypassBtn);

    addKnob(micAmountSlider, juce::Colour(0xff9a86d4));
    addKnob(gainSlider, juce::Colour(0xff6c9fd2));
    addKnob(lowpassSlider, juce::Colour(0xff8fd28c));
    addKnob(deessCrossSlider, juce::Colour(0xffd2a86c));
    addKnob(deessThreshSlider, juce::Colour(0xffd2a86c));
    addKnob(deessRatioSlider, juce::Colour(0xffd2a86c));
    addKnob(optoThreshSlider, juce::Colour(0xff6cb8d2));
    addKnob(optoRatioSlider, juce::Colour(0xff6cb8d2));
    addKnob(optoMakeupSlider, juce::Colour(0xff6cb8d2));
    addKnob(fetThreshSlider, juce::Colour(0xffd26c8c));
    addKnob(fetRatioSlider, juce::Colour(0xffd26c8c));
    addKnob(fetMakeupSlider, juce::Colour(0xffd26c8c));
    addKnob(vcaThreshSlider, juce::Colour(0xffb8d26c));
    addKnob(vcaRatioSlider, juce::Colour(0xffb8d26c));
    addKnob(vcaMakeupSlider, juce::Colour(0xffb8d26c));
    addKnob(exciterDriveSlider, juce::Colour(0xffe0c080));
    addKnob(exciterMixSlider, juce::Colour(0xffe0c080));
    addKnob(spectralMixSlider, juce::Colour(0xff9a9ae0));
    addKnob(spectralThreshSlider, juce::Colour(0xff9a9ae0));
    addKnob(spectralRatioSlider, juce::Colour(0xff9a9ae0));

    micBypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvts, razumov::params::micBypass, micBypassBtn);
    spectralBypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvts, razumov::params::spectralBypass, spectralBypassBtn);

    micAmountAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, razumov::params::micAmount, micAmountSlider);
    gainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, razumov::params::gainDb, gainSlider);
    lowpassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, razumov::params::lowpassHz, lowpassSlider);
    deessCrossAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, razumov::params::deessCrossoverHz, deessCrossSlider);
    deessThreshAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, razumov::params::deessThresholdDb, deessThreshSlider);
    deessRatioAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, razumov::params::deessRatio, deessRatioSlider);
    optoThreshAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, razumov::params::optoThresholdDb, optoThreshSlider);
    optoRatioAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, razumov::params::optoRatio, optoRatioSlider);
    optoMakeupAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, razumov::params::optoMakeupDb, optoMakeupSlider);
    fetThreshAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, razumov::params::fetThresholdDb, fetThreshSlider);
    fetRatioAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, razumov::params::fetRatio, fetRatioSlider);
    fetMakeupAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, razumov::params::fetMakeupDb, fetMakeupSlider);
    vcaThreshAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, razumov::params::vcaThresholdDb, vcaThreshSlider);
    vcaRatioAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, razumov::params::vcaRatio, vcaRatioSlider);
    vcaMakeupAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, razumov::params::vcaMakeupDb, vcaMakeupSlider);
    exciterDriveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, razumov::params::exciterDrive, exciterDriveSlider);
    exciterMixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, razumov::params::exciterMix, exciterMixSlider);
    spectralMixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, razumov::params::spectralMix, spectralMixSlider);
    spectralThreshAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, razumov::params::spectralThresholdDb, spectralThreshSlider);
    spectralRatioAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, razumov::params::spectralRatio, spectralRatioSlider);

    viewport.setViewedComponent(&content, false);
}

void RazumovVocalChainAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1a1d23));
    auto r = getLocalBounds().reduced(16);
    auto titleRow = r.removeFromTop(26);
    g.setColour(juce::Colours::white);
    g.setFont(juce::FontOptions(18.0f, juce::Font::bold));
    g.drawText("Razumov Vocal Chain", titleRow, juce::Justification::centredLeft);
    g.setFont(juce::FontOptions(12.0f));
    g.setColour(juce::Colour(0xffaab4c0));
    auto ver = titleRow;
    ver.removeFromRight(620);
    g.drawText("v0.8.1", ver, juce::Justification::centredRight);
}

void RazumovVocalChainAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    auto header = bounds.removeFromTop(44);
    auto bar = header.removeFromRight(580);
    chainCombo.setBounds(bar.removeFromRight(268).reduced(4, 8));
    chainLabel.setBounds(bar.removeFromRight(52).reduced(4, 10));
    presetCombo.setBounds(bar.removeFromRight(220).reduced(4, 8));
    presetLabel.setBounds(bar.removeFromRight(52).reduced(4, 10));
    chainStrip.setBounds(bounds.removeFromTop(58));
    viewport.setBounds(bounds);

    const int pad = 12;
    const int kw = 96;
    const int kh = 112;
    const int gap = 10;
    const int toggleH = 28;

    const int W = juce::jmax(820, viewport.getWidth());
    int x = pad;
    int y = pad;

    const int mlab = 16;
    const int mkh = 96;
    macroGlueLabel.setBounds(x, y, kw, mlab);
    macroGlueSlider.setBounds(x, y + mlab, kw, mkh);
    x += kw + gap;
    macroAirLabel.setBounds(x, y, kw, mlab);
    macroAirSlider.setBounds(x, y + mlab, kw, mkh);
    x += kw + gap;
    macroSibilanceLabel.setBounds(x, y, kw, mlab);
    macroSibilanceSlider.setBounds(x, y + mlab, kw, mkh);
    x += kw + gap;
    macroPresenceLabel.setBounds(x, y, kw, mlab);
    macroPresenceSlider.setBounds(x, y + mlab, kw, mkh);

    y += mlab + mkh + gap + 14;
    x = pad;

    micBypassBtn.setBounds(x, y, 120, toggleH);
    x += 128 + gap;
    spectralBypassBtn.setBounds(x, y, 140, toggleH);
    x += 148 + gap;
    micAmountSlider.setBounds(x, y, kw, kh);
    x += kw + gap;
    gainSlider.setBounds(x, y, kw, kh);
    x += kw + gap;
    lowpassSlider.setBounds(x, y, kw, kh);

    y += kh + gap + 18;
    x = pad;

    deessCrossSlider.setBounds(x, y, kw, kh);
    deessThreshSlider.setBounds(x + (kw + gap), y, kw, kh);
    deessRatioSlider.setBounds(x + 2 * (kw + gap), y, kw, kh);
    y += kh + gap + 18;

    optoThreshSlider.setBounds(x, y, kw, kh);
    optoRatioSlider.setBounds(x + (kw + gap), y, kw, kh);
    optoMakeupSlider.setBounds(x + 2 * (kw + gap), y, kw, kh);
    y += kh + gap + 18;

    fetThreshSlider.setBounds(x, y, kw, kh);
    fetRatioSlider.setBounds(x + (kw + gap), y, kw, kh);
    fetMakeupSlider.setBounds(x + 2 * (kw + gap), y, kw, kh);
    y += kh + gap + 18;

    vcaThreshSlider.setBounds(x, y, kw, kh);
    vcaRatioSlider.setBounds(x + (kw + gap), y, kw, kh);
    vcaMakeupSlider.setBounds(x + 2 * (kw + gap), y, kw, kh);
    y += kh + gap + 18;

    exciterDriveSlider.setBounds(x, y, kw, kh);
    exciterMixSlider.setBounds(x + (kw + gap), y, kw, kh);
    y += kh + gap + 18;
    x = pad;

    spectralMixSlider.setBounds(x, y, kw, kh);
    spectralThreshSlider.setBounds(x + (kw + gap), y, kw, kh);
    spectralRatioSlider.setBounds(x + 2 * (kw + gap), y, kw, kh);

    const int bottom = spectralRatioSlider.getBottom() + pad;
    content.setSize(W, juce::jmax(bottom, viewport.getHeight()));
}
