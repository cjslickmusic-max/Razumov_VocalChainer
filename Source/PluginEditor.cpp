#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "dsp/graph/FlexGraphDesc.h"
#include "params/ParamIDs.h"
#include "ui/DesignTokens.h"
#include "ui/EditorVisualAssets.h"

namespace tkn = razumov::ui::tokens;

struct VocalChainerLookAndFeel : juce::LookAndFeel_V4
{
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                          float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider) override
    {
        juce::ignoreUnused(slider);
        auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(5.0f);
        const float radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f - 1.5f;
        const auto centre = bounds.getCentre();
        const float a0 = rotaryStartAngle;
        const float a1 = rotaryEndAngle;
        const float aVal = a0 + sliderPos * (a1 - a0);

        juce::Path track;
        track.addCentredArc(centre.x, centre.y, radius, radius, 0.0f, a0, a1, true);
        g.setColour(slider.findColour(juce::Slider::rotarySliderOutlineColourId));
        g.strokePath(track, juce::PathStrokeType(4.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        juce::Path val;
        val.addCentredArc(centre.x, centre.y, radius, radius, 0.0f, a0, aVal, true);
        g.setColour(slider.findColour(juce::Slider::rotarySliderFillColourId));
        g.strokePath(val, juce::PathStrokeType(4.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        const float dotR = 4.0f;
        const auto thumb = centre.getPointOnCircumference(radius - 1.0f, aVal);
        g.setColour(slider.findColour(juce::Slider::thumbColourId));
        g.fillEllipse(thumb.x - dotR, thumb.y - dotR, dotR * 2.0f, dotR * 2.0f);
    }
};

void MicProfilePanel::resized()
{
    auto r = getLocalBounds().reduced(8, 8);
    const int side = juce::jmin(96, juce::jmin(r.getWidth(), r.getHeight()));
    previewBounds_ = r.withSizeKeepingCentre(side, side);
}

void MicProfilePanel::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(tkn::argb::backgroundNode));
    auto frame = previewBounds_.toFloat();
    g.setColour(juce::Colour(tkn::argb::surfaceMicPreviewInner));
    g.fillRoundedRectangle(frame, 7.0f);
    g.setColour(juce::Colour(tkn::argb::borderMicPreview));
    g.drawRoundedRectangle(frame.reduced(0.5f), 7.0f, 1.0f);
    auto caption = frame.removeFromBottom(20.0f);
    g.setColour(juce::Colour(tkn::argb::textSecondary));
    g.setFont(juce::FontOptions(22.0f, juce::Font::bold));
    g.drawText("Mic", frame, juce::Justification::centred);
    g.setFont(juce::FontOptions(10.0f));
    g.setColour(juce::Colour(tkn::argb::textCaption));
    g.drawText("Tap to choose profile", caption, juce::Justification::centred);
}

void MicProfilePanel::mouseDown(const juce::MouseEvent& e)
{
    if (previewBounds_.contains(e.getPosition()) && onPreviewClicked)
        onPreviewClicked();
}

void ModuleSectionBackdrop::paint(juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat().reduced(1.0f);
    g.setColour(juce::Colour(tkn::argb::surfaceModuleBackdrop));
    g.fillRoundedRectangle(r, 8.0f);
    g.setColour(juce::Colour(tkn::argb::borderModulePanel));
    g.drawRoundedRectangle(r, 8.0f, 1.0f);
}

namespace
{
/** Время компиляции этого translation unit (обновляется при пересборке редактора). */
static const char* const kBuildDateTime = __DATE__ " " __TIME__;

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
    s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 58, 16);
    s.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(tkn::argb::controlRotaryOutline));
    s.setColour(juce::Slider::thumbColourId, juce::Colour(tkn::argb::textPrimary));
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
    const int kw = 80;
    const int mkh = 72;
    const int mlab = 14;
    const int colGap = 10;
    const int rowGap = 10;
    macroSectionLabel.setBounds(area.removeFromTop(18).reduced(pad, 0));
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
    const int W = juce::jmax(760, viewportWidth);
    const int pad = 12;
    const int kw = 80;
    const int kh = 100;
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
    const int h = juce::jmax(bottom, 120);
    content.setSize(W, h);
    moduleSectionBackdrop.setBounds(0, 0, W, h);
}

RazumovVocalChainAudioProcessorEditor::RazumovVocalChainAudioProcessorEditor(RazumovVocalChainAudioProcessor& p)
    : AudioProcessorEditor(&p)
    , processor(p)
    , chainStrip(processor)
    , laf(std::make_unique<VocalChainerLookAndFeel>())
{
    setLookAndFeel(laf.get());

    setSize(1000, 900);
    setResizable(true, true);
    setResizeLimits(720, 680, 2000, 1800);

    addAndMakeVisible(micProfilePanel);
    addAndMakeVisible(micProfileCombo);
    micProfilePanel.onPreviewClicked = [this] { micProfileCombo.showPopup(); };
    micProfileCombo.setTooltip("Mic correction profile. Per-slot amount and bypass are in the Mic module below.");

    presetLabel.setText("Factory preset", juce::dontSendNotification);
    presetLabel.setJustificationType(juce::Justification::centredLeft);
    presetLabel.setColour(juce::Label::textColourId, juce::Colour(tkn::argb::textLabel));
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
    presetCombo.setTooltip("Built-in factory presets (macros and defaults). The graph is edited separately.");

    chainLabel.setText("Graph template", juce::dontSendNotification);
    chainLabel.setJustificationType(juce::Justification::centredLeft);
    chainLabel.setColour(juce::Label::textColourId, juce::Colour(tkn::argb::textLabel));
    addAndMakeVisible(chainLabel);
    addAndMakeVisible(chainCombo);

    populateComboBoxes();
    chainCombo.setTooltip("Starting module order and routing. Use Add / Split and the strip below to change the path.");

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
    bypassSlotBtn.setTooltip("Bypass the selected module in the strip (or double-click a module).");
    removeSlotBtn.setTooltip("Remove the selected module from the graph.");
    moveLeftBtn.setTooltip("Move the root segment left.");
    moveRightBtn.setTooltip("Move the root segment right.");
    addModuleBtn.setTooltip("Insert a module or a parallel split after the selection.");

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

    content.addAndMakeVisible(moduleSectionBackdrop);
    moduleTitleLabel.setJustificationType(juce::Justification::centredLeft);
    moduleTitleLabel.setColour(juce::Label::textColourId, juce::Colour(tkn::argb::textTitle));
    moduleTitleLabel.setFont(juce::FontOptions(15.0f, juce::Font::bold));
    content.addAndMakeVisible(moduleTitleLabel);

    moduleHintLabel.setJustificationType(juce::Justification::centredLeft);
    moduleHintLabel.setColour(juce::Label::textColourId, juce::Colour(tkn::argb::textSecondary));
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
        lbl.setColour(juce::Label::textColourId, juce::Colour(tkn::argb::textLabel));
        addAndMakeVisible(lbl);
        styleRotary(s);
        s.setColour(juce::Slider::rotarySliderFillColourId, c);
        addAndMakeVisible(s);
    };

    macroSectionLabel.setText("Macros", juce::dontSendNotification);
    macroSectionLabel.setJustificationType(juce::Justification::centredLeft);
    macroSectionLabel.setColour(juce::Label::textColourId, juce::Colour(tkn::macro::sectionLabel));
    macroSectionLabel.setFont(juce::FontOptions(13.0f, juce::Font::bold));
    addAndMakeVisible(macroSectionLabel);

    setupMacro(macroGlueSlider, macroGlueLabel, "Glue", juce::Colour(tkn::macro::glue));
    setupMacro(macroAirSlider, macroAirLabel, "Air", juce::Colour(tkn::macro::air));
    setupMacro(macroSibilanceSlider, macroSibilanceLabel, "Sibil", juce::Colour(tkn::macro::sibilance));
    setupMacro(macroPresenceSlider, macroPresenceLabel, "Presence", juce::Colour(tkn::macro::presence));
    setupMacro(macroPunchSlider, macroPunchLabel, "Punch", juce::Colour(tkn::macro::punch));
    setupMacro(macroBodySlider, macroBodyLabel, "Body", juce::Colour(tkn::macro::body));
    setupMacro(macroSmoothSlider, macroSmoothLabel, "Smooth", juce::Colour(tkn::macro::smooth));
    setupMacro(macroDensitySlider, macroDensityLabel, "Density", juce::Colour(tkn::macro::density));

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

    addKnob(micAmountSlider, juce::Colour(tkn::knob::micAmount));
    addKnob(gainSlider, juce::Colour(tkn::knob::gain));
    addKnob(lowpassSlider, juce::Colour(tkn::knob::lowpass));
    addKnob(deessCrossSlider, juce::Colour(tkn::knob::deess));
    addKnob(deessThreshSlider, juce::Colour(tkn::knob::deess));
    addKnob(deessRatioSlider, juce::Colour(tkn::knob::deess));
    addKnob(optoThreshSlider, juce::Colour(tkn::knob::opto));
    addKnob(optoRatioSlider, juce::Colour(tkn::knob::opto));
    addKnob(optoMakeupSlider, juce::Colour(tkn::knob::opto));
    addKnob(fetThreshSlider, juce::Colour(tkn::knob::fet));
    addKnob(fetRatioSlider, juce::Colour(tkn::knob::fet));
    addKnob(fetMakeupSlider, juce::Colour(tkn::knob::fet));
    addKnob(vcaThreshSlider, juce::Colour(tkn::knob::vca));
    addKnob(vcaRatioSlider, juce::Colour(tkn::knob::vca));
    addKnob(vcaMakeupSlider, juce::Colour(tkn::knob::vca));
    addKnob(exciterDriveSlider, juce::Colour(tkn::knob::exciter));
    addKnob(exciterMixSlider, juce::Colour(tkn::knob::exciter));
    addKnob(spectralMixSlider, juce::Colour(tkn::knob::spectral));
    addKnob(spectralThreshSlider, juce::Colour(tkn::knob::spectral));
    addKnob(spectralRatioSlider, juce::Colour(tkn::knob::spectral));

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
    g.fillAll(juce::Colour(tkn::argb::backgroundEditor));
    razumov::ui::drawEditorBackgroundLayer(g, getLocalBounds());
    razumov::ui::drawEditorCornerAccents(g, getLocalBounds());
    auto r = getLocalBounds().reduced(16);
    auto titleRow = r.removeFromTop(40);
    g.setColour(juce::Colour(tkn::argb::textTitle));
    g.setFont(juce::FontOptions(18.0f, juce::Font::bold));
    auto titleLeft = titleRow;
    auto versionBlock = titleRow.removeFromRight(300);
    g.drawText("Razumov VocalChainer", titleLeft, juce::Justification::centredLeft);

    const juce::String versionLine = juce::String("v") + JucePlugin_VersionString;
    const juce::String buildLine = juce::String("build ") + kBuildDateTime;

    g.setFont(juce::FontOptions(12.0f));
    g.setColour(juce::Colour(tkn::argb::textLabel));
    auto v1 = versionBlock.removeFromTop(17);
    g.drawText(versionLine, v1, juce::Justification::centredRight);
    g.setFont(juce::FontOptions(10.0f));
    g.setColour(juce::Colour(tkn::argb::textTertiary));
    g.drawText(buildLine, versionBlock, juce::Justification::centredRight);
}

void RazumovVocalChainAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced(16);
    bounds.removeFromTop(40);

    const int ctrlRowH = 56;
    auto ctrlRow = bounds.removeFromTop(ctrlRowH);
    const int colGap = 14;
    const int colW = (ctrlRow.getWidth() - colGap) / 2;
    auto leftPreset = ctrlRow.removeFromLeft(colW);
    auto rightChain = ctrlRow;

    presetLabel.setBounds(leftPreset.removeFromTop(14));
    presetCombo.setBounds(leftPreset.removeFromTop(32).reduced(0, 2));

    chainLabel.setBounds(rightChain.removeFromTop(14));
    chainCombo.setBounds(rightChain.removeFromTop(32).reduced(0, 2));

    const int micH = 112;
    auto micRow = bounds.removeFromTop(micH);
    micProfilePanel.setBounds(micRow.removeFromLeft(112));
    {
        auto comboArea = micRow.reduced(8, 8);
        const int ch = 30;
        micProfileCombo.setBounds(comboArea.withHeight(ch).withY(comboArea.getCentreY() - ch / 2));
    }

    chainStrip.setBounds(bounds.removeFromTop(132));

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

    auto globalArea = bounds.removeFromTop(238);
    layoutGlobalSection(globalArea);

    viewport.setBounds(bounds);
    layoutModuleViewport(viewport.getWidth());
}

RazumovVocalChainAudioProcessorEditor::~RazumovVocalChainAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}
