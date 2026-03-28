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
        moduleHintLabel.setText("Branches appear as cards to the right in the strip.", juce::dontSendNotification);
        moduleHintLabel.setVisible(true);
    }
    else if (kind.has_value())
    {
        switch (*kind)
        {
            case AK::MicCorrection:
                moduleTitleLabel.setText("Mic correction", juce::dontSendNotification);
                moduleHintLabel.setText("Global Mic bypass and Mic amount are above.", juce::dontSendNotification);
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

    resized();
}

void RazumovVocalChainAudioProcessorEditor::layoutGlobalSection(juce::Rectangle<int> area)
{
    const int pad = 12;
    const int kw = 96;
    const int mkh = 96;
    const int mlab = 16;
    int x = area.getX() + pad;
    int y = area.getY() + pad;

    macroGlueLabel.setBounds(x, y, kw, mlab);
    macroGlueSlider.setBounds(x, y + mlab, kw, mkh);
    x += kw + 10;
    macroAirLabel.setBounds(x, y, kw, mlab);
    macroAirSlider.setBounds(x, y + mlab, kw, mkh);
    x += kw + 10;
    macroSibilanceLabel.setBounds(x, y, kw, mlab);
    macroSibilanceSlider.setBounds(x, y + mlab, kw, mkh);
    x += kw + 10;
    macroPresenceLabel.setBounds(x, y, kw, mlab);
    macroPresenceSlider.setBounds(x, y + mlab, kw, mkh);

    y += mlab + mkh + 14;
    x = area.getX() + pad;

    micBypassBtn.setBounds(x, y, 120, 28);
    micAmountSlider.setBounds(x + 132, y, kw, 112);
    gainSlider.setBounds(x + 132 + kw + 10, y, kw, 112);
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
    setSize(920, 840);
    setResizable(true, true);
    setResizeLimits(680, 560, 2000, 1600);

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
            processor.applyFactoryPreset(id - 1);
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
    addAndMakeVisible(micBypassBtn);

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
    g.drawText("v0.8.3", ver, juce::Justification::centredRight);
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

    chainStrip.setBounds(bounds.removeFromTop(58));

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

    auto globalArea = bounds.removeFromTop(230);
    layoutGlobalSection(globalArea);

    viewport.setBounds(bounds);
    layoutModuleViewport(viewport.getWidth());
}
