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
        const float radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f - 2.0f;
        const auto centre = bounds.getCentre();
        const float a0 = rotaryStartAngle;
        const float a1 = rotaryEndAngle;
        const float aVal = a0 + sliderPos * (a1 - a0);

        g.setColour(juce::Colour(0x22000000));
        g.fillEllipse(centre.x - radius - 1.5f, centre.y - radius + 2.5f, (radius + 1.5f) * 2.0f, (radius + 1.5f) * 2.0f);

        const juce::Colour trackDark = slider.findColour(juce::Slider::rotarySliderOutlineColourId).darker(0.35f);
        const juce::Colour trackLight = slider.findColour(juce::Slider::rotarySliderOutlineColourId).brighter(0.55f);
        const int segments = 40;
        for (int i = 0; i < segments; ++i)
        {
            const float t0 = a0 + (a1 - a0) * (float) i / (float) segments;
            const float t1 = a0 + (a1 - a0) * (float) (i + 1) / (float) segments;
            const float mid = (t0 + t1) * 0.5f;
            const float shade = 0.5f + 0.5f * std::sin(mid - juce::MathConstants<float>::halfPi);
            juce::Path seg;
            seg.addCentredArc(centre.x, centre.y, radius, radius, 0.0f, t0, t1, true);
            g.setColour(trackLight.interpolatedWith(trackDark, juce::jlimit(0.0f, 1.0f, shade)));
            g.strokePath(seg, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }

        juce::Path val;
        val.addCentredArc(centre.x, centre.y, radius, radius, 0.0f, a0, aVal, true);
        g.setColour(slider.findColour(juce::Slider::rotarySliderFillColourId));
        g.strokePath(val, juce::PathStrokeType(4.4f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        const float dotR = 4.2f;
        const auto thumb = centre.getPointOnCircumference(radius - 1.0f, aVal);
        g.setColour(slider.findColour(juce::Slider::thumbColourId));
        g.fillEllipse(thumb.x - dotR, thumb.y - dotR, dotR * 2.0f, dotR * 2.0f);
        g.setColour(juce::Colour(0x55000000));
        g.drawEllipse(thumb.x - dotR, thumb.y - dotR, dotR * 2.0f, dotR * 2.0f, 1.0f);
    }

    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour&,
                              bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        auto r = button.getLocalBounds().toFloat().reduced(1.0f);
        juce::Colour top = juce::Colour(tkn::argb::controlButtonFace);
        if (shouldDrawButtonAsDown)
            top = top.darker(0.14f);
        else if (shouldDrawButtonAsHighlighted)
            top = top.brighter(0.05f);
        const juce::Colour bottom = top.darker(0.2f);
        juce::ColourGradient cg(top, r.getX(), r.getY(), bottom, r.getX(), r.getBottom(), false);
        g.setGradientFill(cg);
        g.fillRoundedRectangle(r, 6.0f);
        g.setColour(juce::Colour(tkn::argb::controlButtonBorder));
        g.drawRoundedRectangle(r.reduced(0.5f), 6.0f, 1.0f);
        if (!shouldDrawButtonAsDown)
        {
            g.setColour(juce::Colour(0x14000000));
            g.fillRoundedRectangle(r.removeFromBottom(1.5f), 4.0f);
        }
    }
};

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

void RazumovVocalChainAudioProcessorEditor::styleRotary(juce::Slider& s, int textBoxW, int textBoxH)
{
    s.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, textBoxW, textBoxH);
    s.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(tkn::argb::controlRotaryOutline));
    s.setColour(juce::Slider::thumbColourId, juce::Colour(tkn::argb::textPrimary));
}

void RazumovVocalChainAudioProcessorEditor::refreshRotaryStyles()
{
    const int tw = scaled(58);
    const int th = scaled(18);
    auto one = [this, tw, th](juce::Slider& s) { styleRotary(s, tw, th); };
    one(macroGlueSlider);
    one(macroAirSlider);
    one(macroSibilanceSlider);
    one(macroPresenceSlider);
    one(macroPunchSlider);
    one(macroBodySlider);
    one(macroSmoothSlider);
    one(macroDensitySlider);
    one(micAmountSlider);
    one(gainSlider);
    one(lowpassSlider);
    one(deessCrossSlider);
    one(deessThreshSlider);
    one(deessRatioSlider);
    one(optoThreshSlider);
    one(optoRatioSlider);
    one(optoMakeupSlider);
    one(fetThreshSlider);
    one(fetRatioSlider);
    one(fetMakeupSlider);
    one(vcaThreshSlider);
    one(vcaRatioSlider);
    one(vcaMakeupSlider);
    one(exciterDriveSlider);
    one(exciterMixSlider);
    one(spectralMixSlider);
    one(spectralThreshSlider);
    one(spectralRatioSlider);
}

void RazumovVocalChainAudioProcessorEditor::populateComboBoxes()
{
    auto& apvts = processor.getAPVTS();
    if (auto* p = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter(razumov::params::micProfile)))
        micProfileCombo.addItemList(p->choices, 1);
}

void RazumovVocalChainAudioProcessorEditor::showAddModuleMenuForSlot(uint32_t referenceSlotId)
{
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

    m.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&chainStrip),
                    [this, referenceSlotId](int r) {
                        if (r <= 0)
                            return;
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
                        processor.insertPaletteModuleAfterSlot(referenceSlotId, k);
                        syncChainStripAfterGraphEdit();
                    });
}

void RazumovVocalChainAudioProcessorEditor::showParallelSplitMenuForSlot(uint32_t referenceSlotId)
{
    if (!processor.canInsertParallelSplitAfterSlot(referenceSlotId))
        return;
    juce::PopupMenu m;
    m.addItem(20, "Split x2");
    m.addItem(21, "Split x3");
    m.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&chainStrip),
                    [this, referenceSlotId](int r) {
                        if (r == 20)
                        {
                            processor.insertSplitAfterSlot(referenceSlotId, 2);
                            syncChainStripAfterGraphEdit();
                            return;
                        }
                        if (r == 21)
                        {
                            processor.insertSplitAfterSlot(referenceSlotId, 3);
                            syncChainStripAfterGraphEdit();
                        }
                    });
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
            {
                bool isRoomSlot = false;
                for (const auto& it : processor.getChainStripItems())
                {
                    if (it.slotId == selectedSlotId_ && it.label.containsIgnoreCase("Room"))
                    {
                        isRoomSlot = true;
                        break;
                    }
                }
                if (isRoomSlot)
                {
                    moduleTitleLabel.setText("Room correction", juce::dontSendNotification);
                    moduleHintLabel.setText("Pass-through when no room profile is active.", juce::dontSendNotification);
                    moduleHintLabel.setVisible(true);
                }
                else
                    moduleTitleLabel.setText("Gain", juce::dontSendNotification);
                break;
            }
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

    micProfileCombo.setVisible(showMic);
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

    const bool canRemoveOrMove = selectedSlotId_ != 0
                                 && !razumov::graph::isProtectedFrontRootModuleSlot(processor.getGraphDesc(),
                                                                                     selectedSlotId_);
    removeSlotBtn.setEnabled(canRemoveOrMove);
    moveLeftBtn.setEnabled(canRemoveOrMove);
    moveRightBtn.setEnabled(canRemoveOrMove);

    reloadModuleParamsFromProcessor();
    resized();
}

void RazumovVocalChainAudioProcessorEditor::layoutMacroHeroRow(juce::Rectangle<int> area)
{
    const int pad = scaled(12);
    const int kw = scaled(92);
    const int mkh = scaled(96);
    const int mlab = scaled(15);
    const int colGap = scaled(6);
    macroSectionLabel.setBounds(area.removeFromTop(scaled(22)).reduced(pad, 0));

    const int totalW = 8 * kw + 7 * colGap;
    int x = area.getX() + (area.getWidth() - totalW) / 2;
    const int y = area.getY() + pad;

    auto place = [&](juce::Label& lbl, juce::Slider& s) {
        lbl.setBounds(x, y, kw, mlab);
        s.setBounds(x, y + mlab, kw, mkh);
        x += kw + colGap;
    };

    place(macroGlueLabel, macroGlueSlider);
    place(macroAirLabel, macroAirSlider);
    place(macroSibilanceLabel, macroSibilanceSlider);
    place(macroPresenceLabel, macroPresenceSlider);
    place(macroPunchLabel, macroPunchSlider);
    place(macroBodyLabel, macroBodySlider);
    place(macroSmoothLabel, macroSmoothSlider);
    place(macroDensityLabel, macroDensitySlider);
}

void RazumovVocalChainAudioProcessorEditor::layoutModuleViewport(int viewportWidth)
{
    const int W = juce::jmax(scaled(760), viewportWidth);
    const int pad = scaled(12);
    const int kw = scaled(80);
    const int kh = scaled(100);
    const int gap = scaled(10);
    int x = pad;
    int y = pad;

    moduleTitleLabel.setBounds(x, y, W - 2 * pad, scaled(22));
    y += scaled(26);

    if (moduleHintLabel.isVisible())
    {
        moduleHintLabel.setBounds(x, y, W - 2 * pad, scaled(36));
        y += scaled(42);
    }

    if (micBypassBtn.isVisible())
    {
        micProfileCombo.setBounds(x, y, juce::jmin(scaled(420), W - 2 * pad), scaled(28));
        y += scaled(34);
        micBypassBtn.setBounds(x, y, scaled(120), scaled(28));
        micAmountSlider.setBounds(x + scaled(132), y, kw, kh);
        y += kh + gap + scaled(18);
    }
    else if (gainSlider.isVisible())
    {
        gainSlider.setBounds(x, y, kw, kh);
        y += kh + gap + scaled(18);
    }

    auto row3 = [&](juce::Slider& a, juce::Slider& b, juce::Slider& c) {
        if (!a.isVisible())
            return;
        a.setBounds(x, y, kw, kh);
        b.setBounds(x + (kw + gap), y, kw, kh);
        c.setBounds(x + 2 * (kw + gap), y, kw, kh);
        y += kh + gap + scaled(18);
    };

    row3(deessCrossSlider, deessThreshSlider, deessRatioSlider);
    row3(optoThreshSlider, optoRatioSlider, optoMakeupSlider);
    row3(fetThreshSlider, fetRatioSlider, fetMakeupSlider);
    row3(vcaThreshSlider, vcaRatioSlider, vcaMakeupSlider);

    if (exciterDriveSlider.isVisible())
    {
        exciterDriveSlider.setBounds(x, y, kw, kh);
        exciterMixSlider.setBounds(x + (kw + gap), y, kw, kh);
        y += kh + gap + scaled(18);
    }

    if (spectralBypassBtn.isVisible())
    {
        spectralBypassBtn.setBounds(x, y, scaled(140), scaled(28));
        y += scaled(36);
        row3(spectralMixSlider, spectralThreshSlider, spectralRatioSlider);
    }

    if (lowpassSlider.isVisible())
    {
        lowpassSlider.setBounds(x, y, kw, kh);
        y += kh + gap + scaled(18);
    }

    const int bottom = y + pad;
    const int h = juce::jmax(bottom, scaled(120));
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

    setSize(1280, 920);
    setResizable(true, true);
    setResizeLimits(1180, 780, 2800, 2200);

    auto styleCombo = [](juce::ComboBox& c) {
        using namespace juce;
        c.setColour(ComboBox::backgroundColourId, juce::Colour(tkn::argb::backgroundNode));
        c.setColour(ComboBox::outlineColourId, juce::Colour(tkn::argb::borderModulePanel));
        c.setColour(ComboBox::textColourId, juce::Colour(tkn::argb::textPrimary));
        c.setColour(ComboBox::arrowColourId, juce::Colour(tkn::argb::textSecondary));
    };

    presetLabel.setText("Preset", juce::dontSendNotification);
    presetLabel.setJustificationType(juce::Justification::centred);
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
    styleCombo(presetCombo);
    presetCombo.setTooltip("Built-in factory presets (macros and defaults). The graph is edited in the strip.");

    content.addAndMakeVisible(micProfileCombo);
    styleCombo(micProfileCombo);
    micProfileCombo.setTooltip("Mic profile for correction (Mic correction module below).");

    populateComboBoxes();

    addAndMakeVisible(chainStrip);
    chainStrip.onSlotSelected = [this](uint32_t id) {
        selectedSlotId_ = id;
        refreshModulePanelVisibility();
    };
    chainStrip.onRequestAddSerialAfter = [this](uint32_t id) { showAddModuleMenuForSlot(id); };
    chainStrip.onRequestParallelBranch = [this](uint32_t id) { showParallelSplitMenuForSlot(id); };

    addAndMakeVisible(bypassSlotBtn);
    addAndMakeVisible(removeSlotBtn);
    addAndMakeVisible(moveLeftBtn);
    addAndMakeVisible(moveRightBtn);
    addAndMakeVisible(addModuleBtn);
    bypassSlotBtn.setTooltip("Bypass the selected module in the strip (or double-click a module).");
    removeSlotBtn.setTooltip("Remove the selected module from the graph.");
    moveLeftBtn.setTooltip("Move the root segment left.");
    moveRightBtn.setTooltip("Move the root segment right.");
    addModuleBtn.setTooltip("Insert a module after the selected block (same as + between blocks).");

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
        const uint32_t ref = referenceSlotForInsert(processor, selectedSlotId_);
        if (ref != 0)
            showAddModuleMenuForSlot(ref);
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

    auto addKnob = [this](juce::Slider& s, juce::Colour c) {
        styleRotary(s, 58, 16);
        s.setColour(juce::Slider::rotarySliderFillColourId, c);
        content.addAndMakeVisible(s);
    };

    auto setupMacro = [&](juce::Slider& s, juce::Label& lbl, const char* name, juce::Colour c) {
        lbl.setText(name, juce::dontSendNotification);
        lbl.setJustificationType(juce::Justification::centred);
        lbl.setColour(juce::Label::textColourId, juce::Colour(tkn::argb::textLabel));
        addAndMakeVisible(lbl);
        styleRotary(s, 58, 16);
        s.setColour(juce::Slider::rotarySliderFillColourId, c);
        addAndMakeVisible(s);
    };

    macroSectionLabel.setText("Macros", juce::dontSendNotification);
    macroSectionLabel.setJustificationType(juce::Justification::centred);
    macroSectionLabel.setColour(juce::Label::textColourId, juce::Colour(tkn::macro::sectionLabel));
    macroSectionLabel.setFont(juce::FontOptions(15.0f, juce::Font::bold));
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
    uiScale_ = juce::jlimit(0.88f, 1.5f, (float) getWidth() / 1280.f);

    auto bounds = getLocalBounds().reduced(scaled(16));
    bounds.removeFromTop(scaled(40));

    const int presetRowH = scaled(52);
    auto presetRow = bounds.removeFromTop(presetRowH);
    const int pw = juce::jmin(scaled(440), presetRow.getWidth() - scaled(40));
    auto presetBox = presetRow.withSizeKeepingCentre(pw, presetRowH);
    presetLabel.setBounds(presetBox.removeFromTop(scaled(14)));
    presetCombo.setBounds(presetBox.removeFromTop(scaled(32)).reduced(0, 2));

    const int macroBlockH = scaled(178);
    layoutMacroHeroRow(bounds.removeFromTop(macroBlockH));

    chainStrip.setBounds(bounds.removeFromTop(scaled(248)));

    auto toolRow = bounds.removeFromTop(scaled(34));
    const int tw = scaled(88);
    int tx = toolRow.getX() + scaled(10);
    bypassSlotBtn.setBounds(tx, toolRow.getY() + 2, tw, scaled(28));
    tx += tw + scaled(6);
    removeSlotBtn.setBounds(tx, toolRow.getY() + 2, tw, scaled(28));
    tx += tw + scaled(6);
    moveLeftBtn.setBounds(tx, toolRow.getY() + 2, scaled(36), scaled(28));
    tx += scaled(42);
    moveRightBtn.setBounds(tx, toolRow.getY() + 2, scaled(36), scaled(28));
    tx += scaled(48);
    addModuleBtn.setBounds(tx, toolRow.getY() + 2, scaled(100), scaled(28));

    viewport.setBounds(bounds);
    layoutModuleViewport(viewport.getWidth());
    refreshRotaryStyles();
}

RazumovVocalChainAudioProcessorEditor::~RazumovVocalChainAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}
