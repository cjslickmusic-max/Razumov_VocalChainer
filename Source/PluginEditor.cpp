#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "dsp/graph/FlexGraphDesc.h"
#include "params/ParamIDs.h"

namespace
{
uint32_t referenceSlotForInsert(RazumovVocalChainAudioProcessor& proc, uint32_t selectedSlotId)
{
    if (selectedSlotId != 0)
        return selectedSlotId;
    const auto items = proc.getChainStripItems();
    if (!items.empty())
        return items.front().slotId;
    return 0;
}
} // namespace

void RazumovVocalChainAudioProcessorEditor::styleRotary(juce::Slider& s)
{
    s.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 72, 16);
}

void RazumovVocalChainAudioProcessorEditor::populateComboBoxes()
{
    auto& apvts = processor.getAPVTS();
    if (auto* p = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter(razumov::params::chainProfile)))
        chainCombo.addItemList(p->choices, 1);
    if (auto* p = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter(razumov::params::micProfile)))
        micProfileCombo.addItemList(p->choices, 1);
}

void RazumovVocalChainAudioProcessorEditor::syncChainStripAfterGraphEdit()
{
    chainStrip.syncFromProcessor();
    selectedSlotId_ = chainStrip.getSelectedSlotId();
    refreshModulePanelVisibility();
}

void RazumovVocalChainAudioProcessorEditor::reloadModuleParamsFromProcessor()
{
    using namespace razumov::params;
    const auto& desc = processor.getGraphDesc();
    if (razumov::graph::queryIsParallelSplitSlot(desc, selectedSlotId_))
        return;
    const auto kind = razumov::graph::queryModuleKindForSlotId(desc, selectedSlotId_);
    if (!kind.has_value())
        return;

    const uint32_t id = selectedSlotId_;
    micBypassBtn.setToggleState(processor.getModuleBoolParam(id, micBypass), juce::dontSendNotification);
    spectralBypassBtn.setToggleState(processor.getModuleBoolParam(id, spectralBypass), juce::dontSendNotification);
    micAmountSlider.setValue(processor.getModuleFloatParam(id, micAmount), juce::dontSendNotification);
    gainSlider.setValue(processor.getModuleFloatParam(id, gainDb), juce::dontSendNotification);
    lowpassSlider.setValue(processor.getModuleFloatParam(id, lowpassHz), juce::dontSendNotification);
    deessCrossSlider.setValue(processor.getModuleFloatParam(id, deessCrossoverHz), juce::dontSendNotification);
    deessThreshSlider.setValue(processor.getModuleFloatParam(id, deessThresholdDb), juce::dontSendNotification);
    deessRatioSlider.setValue(processor.getModuleFloatParam(id, deessRatio), juce::dontSendNotification);
    optoThreshSlider.setValue(processor.getModuleFloatParam(id, optoThresholdDb), juce::dontSendNotification);
    optoRatioSlider.setValue(processor.getModuleFloatParam(id, optoRatio), juce::dontSendNotification);
    optoMakeupSlider.setValue(processor.getModuleFloatParam(id, optoMakeupDb), juce::dontSendNotification);
    fetThreshSlider.setValue(processor.getModuleFloatParam(id, fetThresholdDb), juce::dontSendNotification);
    fetRatioSlider.setValue(processor.getModuleFloatParam(id, fetRatio), juce::dontSendNotification);
    fetMakeupSlider.setValue(processor.getModuleFloatParam(id, fetMakeupDb), juce::dontSendNotification);
    vcaThreshSlider.setValue(processor.getModuleFloatParam(id, vcaThresholdDb), juce::dontSendNotification);
    vcaRatioSlider.setValue(processor.getModuleFloatParam(id, vcaRatio), juce::dontSendNotification);
    vcaMakeupSlider.setValue(processor.getModuleFloatParam(id, vcaMakeupDb), juce::dontSendNotification);
    exciterDriveSlider.setValue(processor.getModuleFloatParam(id, exciterDrive), juce::dontSendNotification);
    exciterMixSlider.setValue(processor.getModuleFloatParam(id, exciterMix), juce::dontSendNotification);
    spectralMixSlider.setValue(processor.getModuleFloatParam(id, spectralMix), juce::dontSendNotification);
    spectralThreshSlider.setValue(processor.getModuleFloatParam(id, spectralThresholdDb), juce::dontSendNotification);
    spectralRatioSlider.setValue(processor.getModuleFloatParam(id, spectralRatio), juce::dontSendNotification);
}

void RazumovVocalChainAudioProcessorEditor::refreshModulePanelVisibility()
{
    using AK = razumov::graph::AudioNodeKind;
    const auto& desc = processor.getGraphDesc();
    const auto kind = razumov::graph::queryModuleKindForSlotId(desc, selectedSlotId_);
    const bool isSplit = razumov::graph::queryIsParallelSplitSlot(desc, selectedSlotId_);

    moduleHintLabel.setVisible(false);

    if (isSplit)
    {
        moduleTitleLabel.setText("Parallel split", juce::dontSendNotification);
        moduleHintLabel.setText("Parallel branches use the lower row in the strip.", juce::dontSendNotification);
        moduleHintLabel.setVisible(true);
    }
    else if (kind.has_value())
    {
        switch (*kind)
        {
            case AK::MicCorrection:
                moduleTitleLabel.setText("Mic correction", juce::dontSendNotification);
                moduleHintLabel.setText("Per-slot mic correction (bypass and amount below).", juce::dontSendNotification);
                moduleHintLabel.setVisible(true);
                break;
            case AK::Gain:
                moduleTitleLabel.setText("Gain", juce::dontSendNotification);
                break;
            case AK::Filter:
                moduleTitleLabel.setText("Lowpass", juce::dontSendNotification);
                break;
            case AK::Deesser:
                moduleTitleLabel.setText("De-esser", juce::dontSendNotification);
                break;
            case AK::OptoCompressor:
                moduleTitleLabel.setText("Opto compressor", juce::dontSendNotification);
                break;
            case AK::FetCompressor:
                moduleTitleLabel.setText("FET compressor", juce::dontSendNotification);
                break;
            case AK::VcaCompressor:
                moduleTitleLabel.setText("VCA compressor", juce::dontSendNotification);
                break;
            case AK::Exciter:
                moduleTitleLabel.setText("Exciter", juce::dontSendNotification);
                break;
            case AK::SpectralCompressor:
                moduleTitleLabel.setText("Spectral compressor", juce::dontSendNotification);
                break;
            case AK::Latency:
                moduleTitleLabel.setText("Latency", juce::dontSendNotification);
                moduleHintLabel.setText("Utility delay line (tests / alignment).", juce::dontSendNotification);
                moduleHintLabel.setVisible(true);
                break;
            default:
                moduleTitleLabel.setText("Module", juce::dontSendNotification);
                break;
        }
    }
    else
    {
        moduleTitleLabel.setText("Select a module", juce::dontSendNotification);
    }

    const bool showDeess = kind == AK::Deesser;
    const bool showOpto = kind == AK::OptoCompressor;
    const bool showFet = kind == AK::FetCompressor;
    const bool showVca = kind == AK::VcaCompressor;
    const bool showExc = kind == AK::Exciter;
    const bool showSpec = kind == AK::SpectralCompressor;
    const bool showLp = kind == AK::Filter;
    const bool showSplitPanel = isSplit;
    const bool showMic = kind == AK::MicCorrection;
    const bool showGainKnob = kind == AK::Gain;

    juce::ignoreUnused(showSplitPanel);

    micBypassBtn.setVisible(showMic);
    micAmountSlider.setVisible(showMic);
    gainSlider.setVisible(showGainKnob);

    deessCrossSlider.setVisible(showDeess);
    deessThreshSlider.setVisible(showDeess);
    deessRatioSlider.setVisible(showDeess);

    optoThreshSlider.setVisible(showOpto);
    optoRatioSlider.setVisible(showOpto);
    optoMakeupSlider.setVisible(showOpto);

    fetThreshSlider.setVisible(showFet);
    fetRatioSlider.setVisible(showFet);
    fetMakeupSlider.setVisible(showFet);

    vcaThreshSlider.setVisible(showVca);
    vcaRatioSlider.setVisible(showVca);
    vcaMakeupSlider.setVisible(showVca);

    exciterDriveSlider.setVisible(showExc);
    exciterMixSlider.setVisible(showExc);

    spectralBypassBtn.setVisible(showSpec);
    spectralMixSlider.setVisible(showSpec);
    spectralThreshSlider.setVisible(showSpec);
    spectralRatioSlider.setVisible(showSpec);

    lowpassSlider.setVisible(showLp);

    reloadModuleParamsFromProcessor();
    resized();
}

void RazumovVocalChainAudioProcessorEditor::layoutGlobalSection(juce::Rectangle<int> area)
{
    const int pad = 12;
    const int kw = 96;
    const int mkh = 88;
    const int mlab = 16;
    const int colGap = 10;
    const int rowGap = 10;
    int x = area.getX() + pad;
    int y = area.getY() + pad;

    auto place = [&](juce::Label& lbl, juce::Slider& s) {
        lbl.setBounds(x, y, kw, mlab);
        s.setBounds(x, y + mlab, kw, mkh);
        x += kw + colGap;
    };

    place(macroGlueLabel, macroGlueSlider);
    place(macroAirLabel, macroAirSlider);
    place(macroSibilanceLabel, macroSibilanceSlider);
    place(macroPresenceLabel, macroPresenceSlider);

    x = area.getX() + pad;
    y += mlab + mkh + rowGap;

    place(macroPunchLabel, macroPunchSlider);
    place(macroBodyLabel, macroBodySlider);
    place(macroSmoothLabel, macroSmoothSlider);
    place(macroDensityLabel, macroDensitySlider);
}

void RazumovVocalChainAudioProcessorEditor::layoutModuleViewport(int viewportWidth)
{
    const int W = juce::jmax(820, viewportWidth);
    const int pad = 12;
    const int kw = 96;
    const int kh = 112;
    const int gap = 10;
    int x = pad;
    int y = pad;

    moduleTitleLabel.setBounds(x, y, W - 2 * pad, 22);
    y += 26;

    if (moduleHintLabel.isVisible())
    {
        moduleHintLabel.setBounds(x, y, W - 2 * pad, 36);
        y += 42;
    }

    if (micBypassBtn.isVisible())
    {
        micBypassBtn.setBounds(x, y, 120, 28);
        micAmountSlider.setBounds(x + 132, y, kw, kh);
        y += kh + gap + 18;
    }
    else if (gainSlider.isVisible())
    {
        gainSlider.setBounds(x, y, kw, kh);
        y += kh + gap + 18;
    }

    auto row3 = [&](juce::Slider& a, juce::Slider& b, juce::Slider& c) {
        if (!a.isVisible())
            return;
        a.setBounds(x, y, kw, kh);
        b.setBounds(x + (kw + gap), y, kw, kh);
        c.setBounds(x + 2 * (kw + gap), y, kw, kh);
        y += kh + gap + 18;
    };

    row3(deessCrossSlider, deessThreshSlider, deessRatioSlider);
    row3(optoThreshSlider, optoRatioSlider, optoMakeupSlider);
    row3(fetThreshSlider, fetRatioSlider, fetMakeupSlider);
    row3(vcaThreshSlider, vcaRatioSlider, vcaMakeupSlider);

    if (exciterDriveSlider.isVisible())
    {
        exciterDriveSlider.setBounds(x, y, kw, kh);
        exciterMixSlider.setBounds(x + (kw + gap), y, kw, kh);
        y += kh + gap + 18;
    }

    if (spectralBypassBtn.isVisible())
    {
        spectralBypassBtn.setBounds(x, y, 140, 28);
        y += 36;
        row3(spectralMixSlider, spectralThreshSlider, spectralRatioSlider);
    }

    if (lowpassSlider.isVisible())
    {
        lowpassSlider.setBounds(x, y, kw, kh);
        y += kh + gap + 18;
    }

    const int bottom = y + pad;
    content.setSize(W, juce::jmax(bottom, 120));
}

RazumovVocalChainAudioProcessorEditor::RazumovVocalChainAudioProcessorEditor(RazumovVocalChainAudioProcessor& p)
    : AudioProcessorEditor(&p)
    , processor(p)
    , chainStrip(processor)
{
    setSize(920, 960);
    setResizable(true, true);
    setResizeLimits(680, 720, 2000, 1800);

    micProfileLabel.setText("Mic profile", juce::dontSendNotification);
    micProfileLabel.setJustificationType(juce::Justification::centredRight);
    micProfileLabel.setColour(juce::Label::textColourId, juce::Colour(0xffaab4c0));
    addAndMakeVisible(micProfileLabel);
    addAndMakeVisible(micProfileCombo);

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
        {
            processor.applyFactoryPreset(id - 1);
            reloadModuleParamsFromProcessor();
        }
    };
    addAndMakeVisible(presetCombo);

    chainLabel.setText("Chain", juce::dontSendNotification);
    chainLabel.setJustificationType(juce::Justification::centredRight);
    chainLabel.setColour(juce::Label::textColourId, juce::Colour(0xffaab4c0));
    addAndMakeVisible(chainLabel);
    addAndMakeVisible(chainCombo);

    populateComboBoxes();

    addAndMakeVisible(chainStrip);
    chainStrip.onSlotSelected = [this](uint32_t id) {
        selectedSlotId_ = id;
        refreshModulePanelVisibility();
    };

    addAndMakeVisible(bypassSlotBtn);
    addAndMakeVisible(removeSlotBtn);
    addAndMakeVisible(moveLeftBtn);
    addAndMakeVisible(moveRightBtn);
    addAndMakeVisible(addModuleBtn);

    bypassSlotBtn.onClick = [this] {
        const auto items = processor.getChainStripItems();
        bool bypass = false;
        bool found = false;
        for (const auto& it : items)
        {
            if (it.slotId == selectedSlotId_)
            {
                found = true;
                bypass = it.bypassed;
                break;
            }
        }
        if (found)
            processor.setSlotBypassForId(selectedSlotId_, !bypass);
        syncChainStripAfterGraphEdit();
    };

    removeSlotBtn.onClick = [this] {
        if (selectedSlotId_ != 0)
            processor.removeGraphSlotById(selectedSlotId_);
        syncChainStripAfterGraphEdit();
    };

    moveLeftBtn.onClick = [this] {
        if (selectedSlotId_ != 0)
            processor.moveRootSlotContainingId(selectedSlotId_, -1);
        syncChainStripAfterGraphEdit();
    };

    moveRightBtn.onClick = [this] {
        if (selectedSlotId_ != 0)
            processor.moveRootSlotContainingId(selectedSlotId_, 1);
        syncChainStripAfterGraphEdit();
    };

    addModuleBtn.onClick = [this] {
        juce::PopupMenu m;
        using AK = razumov::graph::AudioNodeKind;
        m.addItem(1, "Gain");
        m.addItem(2, "Lowpass");
        m.addItem(3, "De-esser");
        m.addItem(4, "Opto");
        m.addItem(5, "FET");
        m.addItem(6, "VCA");
        m.addItem(7, "Exciter");
        m.addItem(8, "Spectral");
        m.addSeparator();
        m.addItem(20, "Split x2");
        m.addItem(21, "Split x3");

        m.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&addModuleBtn),
                        [this](int r) {
                            if (r <= 0)
                                return;
                            const uint32_t ref = referenceSlotForInsert(processor, selectedSlotId_);
                            if (ref == 0)
                                return;
                            if (r == 20)
                            {
                                processor.insertSplitAfterSlot(ref, 2);
                                syncChainStripAfterGraphEdit();
                                return;
                            }
                            if (r == 21)
                            {
                                processor.insertSplitAfterSlot(ref, 3);
                                syncChainStripAfterGraphEdit();
                                return;
                            }
                            using AK = razumov::graph::AudioNodeKind;
                            AK k = AK::Gain;
                            switch (r)
                            {
                                case 1:
                                    k = AK::Gain;
                                    break;
                                case 2:
                                    k = AK::Filter;
                                    break;
                                case 3:
                                    k = AK::Deesser;
                                    break;
                                case 4:
                                    k = AK::OptoCompressor;
                                    break;
                                case 5:
                                    k = AK::FetCompressor;
                                    break;
                                case 6:
                                    k = AK::VcaCompressor;
                                    break;
                                case 7:
                                    k = AK::Exciter;
                                    break;
                                case 8:
                                    k = AK::SpectralCompressor;
                                    break;
                                default:
                                    return;
                            }
                            processor.insertPaletteModuleAfterSlot(ref, k);
                            syncChainStripAfterGraphEdit();
                        });
    };

    moduleTitleLabel.setJustificationType(juce::Justification::centredLeft);
    moduleTitleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    moduleTitleLabel.setFont(juce::FontOptions(15.0f, juce::Font::bold));
    content.addAndMakeVisible(moduleTitleLabel);

    moduleHintLabel.setJustificationType(juce::Justification::centredLeft);
    moduleHintLabel.setColour(juce::Label::textColourId, juce::Colour(0xff8892a0));
    moduleHintLabel.setFont(juce::FontOptions(12.0f));
    content.addAndMakeVisible(moduleHintLabel);

    addAndMakeVisible(viewport);
    viewport.setScrollBarsShown(true, false);

    auto& apvts = processor.getAPVTS();

    micProfileAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvts, razumov::params::micProfile, micProfileCombo);

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
        addAndMakeVisible(lbl);
        styleRotary(s);
        s.setColour(juce::Slider::rotarySliderFillColourId, c);
        addAndMakeVisible(s);
    };

    setupMacro(macroGlueSlider, macroGlueLabel, "Glue", juce::Colour(0xffc080a0));
    setupMacro(macroAirSlider, macroAirLabel, "Air", juce::Colour(0xff80c0c8));
    setupMacro(macroSibilanceSlider, macroSibilanceLabel, "Sibil", juce::Colour(0xffd0a868));
    setupMacro(macroPresenceSlider, macroPresenceLabel, "Presence", juce::Colour(0xffa8d080));
    setupMacro(macroPunchSlider, macroPunchLabel, "Punch", juce::Colour(0xffe88860));
    setupMacro(macroBodySlider, macroBodyLabel, "Body", juce::Colour(0xffb89870));
    setupMacro(macroSmoothSlider, macroSmoothLabel, "Smooth", juce::Colour(0xff8898d0));
    setupMacro(macroDensitySlider, macroDensityLabel, "Density", juce::Colour(0xff78b0a0));

    macroGlueAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, razumov::params::macroGlue, macroGlueSlider);
    macroAirAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, razumov::params::macroAir, macroAirSlider);
    macroSibilanceAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, razumov::params::macroSibilance, macroSibilanceSlider);
    macroPresenceAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, razumov::params::macroPresence, macroPresenceSlider);
    macroPunchAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, razumov::params::macroPunch, macroPunchSlider);
    macroBodyAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, razumov::params::macroBody, macroBodySlider);
    macroSmoothAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, razumov::params::macroSmooth, macroSmoothSlider);
    macroDensityAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, razumov::params::macroDensity, macroDensitySlider);

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

    micAmountSlider.setRange(0.0, 1.0, 0.01);
    gainSlider.setRange(-24.0, 12.0, 0.1);
    gainSlider.setTextValueSuffix(" dB");
    lowpassSlider.setRange(400.0, 20000.0, 1.0);
    lowpassSlider.setSkewFactorFromMidPoint(2500.0);
    lowpassSlider.setTextValueSuffix(" Hz");
    deessCrossSlider.setRange(3000.0, 10000.0, 1.0);
    deessThreshSlider.setRange(-60.0, 0.0, 0.1);
    deessRatioSlider.setRange(1.0, 10.0, 0.1);
    optoThreshSlider.setRange(-60.0, 0.0, 0.1);
    optoRatioSlider.setRange(1.0, 20.0, 0.1);
    optoMakeupSlider.setRange(0.0, 24.0, 0.1);
    fetThreshSlider.setRange(-60.0, 0.0, 0.1);
    fetRatioSlider.setRange(1.0, 20.0, 0.1);
    fetMakeupSlider.setRange(0.0, 24.0, 0.1);
    vcaThreshSlider.setRange(-60.0, 0.0, 0.1);
    vcaRatioSlider.setRange(1.0, 20.0, 0.1);
    vcaMakeupSlider.setRange(0.0, 24.0, 0.1);
    exciterDriveSlider.setRange(0.1, 8.0, 0.01);
    exciterMixSlider.setRange(0.0, 1.0, 0.01);
    spectralMixSlider.setRange(0.0, 1.0, 0.01);
    spectralThreshSlider.setRange(-60.0, 0.0, 0.1);
    spectralRatioSlider.setRange(1.0, 20.0, 0.1);

    auto wireFloat = [this](juce::Slider& s, const char* paramId) {
        s.onValueChange = [this, &s, paramId] {
            processor.setModuleFloatParam(selectedSlotId_, paramId, (float) s.getValue());
        };
    };

    wireFloat(micAmountSlider, razumov::params::micAmount);
    wireFloat(gainSlider, razumov::params::gainDb);
    wireFloat(lowpassSlider, razumov::params::lowpassHz);
    wireFloat(deessCrossSlider, razumov::params::deessCrossoverHz);
    wireFloat(deessThreshSlider, razumov::params::deessThresholdDb);
    wireFloat(deessRatioSlider, razumov::params::deessRatio);
    wireFloat(optoThreshSlider, razumov::params::optoThresholdDb);
    wireFloat(optoRatioSlider, razumov::params::optoRatio);
    wireFloat(optoMakeupSlider, razumov::params::optoMakeupDb);
    wireFloat(fetThreshSlider, razumov::params::fetThresholdDb);
    wireFloat(fetRatioSlider, razumov::params::fetRatio);
    wireFloat(fetMakeupSlider, razumov::params::fetMakeupDb);
    wireFloat(vcaThreshSlider, razumov::params::vcaThresholdDb);
    wireFloat(vcaRatioSlider, razumov::params::vcaRatio);
    wireFloat(vcaMakeupSlider, razumov::params::vcaMakeupDb);
    wireFloat(exciterDriveSlider, razumov::params::exciterDrive);
    wireFloat(exciterMixSlider, razumov::params::exciterMix);
    wireFloat(spectralMixSlider, razumov::params::spectralMix);
    wireFloat(spectralThreshSlider, razumov::params::spectralThresholdDb);
    wireFloat(spectralRatioSlider, razumov::params::spectralRatio);

    micBypassBtn.onClick = [this] {
        processor.setModuleBoolParam(selectedSlotId_, razumov::params::micBypass, micBypassBtn.getToggleState());
    };
    spectralBypassBtn.onClick = [this] {
        processor.setModuleBoolParam(selectedSlotId_, razumov::params::spectralBypass, spectralBypassBtn.getToggleState());
    };

    viewport.setViewedComponent(&content, false);

    chainStrip.syncFromProcessor();
    selectedSlotId_ = chainStrip.getSelectedSlotId();
    chainStrip.setSelectedSlotId(selectedSlotId_);
    refreshModulePanelVisibility();
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
    g.drawText("v0.9.1", ver, juce::Justification::centredRight);
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

    auto micRow = bounds.removeFromTop(32);
    micProfileCombo.setBounds(micRow.removeFromRight(320).reduced(4, 4));
    micProfileLabel.setBounds(micRow.removeFromRight(88).reduced(4, 6));

    chainStrip.setBounds(bounds.removeFromTop(100));

    auto toolRow = bounds.removeFromTop(32);
    const int tw = 88;
    int tx = toolRow.getX() + 10;
    bypassSlotBtn.setBounds(tx, toolRow.getY() + 2, tw, 28);
    tx += tw + 6;
    removeSlotBtn.setBounds(tx, toolRow.getY() + 2, tw, 28);
    tx += tw + 6;
    moveLeftBtn.setBounds(tx, toolRow.getY() + 2, 36, 28);
    tx += 42;
    moveRightBtn.setBounds(tx, toolRow.getY() + 2, 36, 28);
    tx += 48;
    addModuleBtn.setBounds(tx, toolRow.getY() + 2, 100, 28);

    auto globalArea = bounds.removeFromTop(260);
    layoutGlobalSection(globalArea);

    viewport.setBounds(bounds);
    layoutModuleViewport(viewport.getWidth());
}
