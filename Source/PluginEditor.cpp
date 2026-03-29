#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "dsp/graph/FlexGraphDesc.h"
#include "params/ParamIDs.h"
#include "ui/DesignTokens.h"
#include "ui/EditorVisualAssets.h"

namespace tkn = razumov::ui::tokens;

struct VocalChainerLookAndFeel : juce::LookAndFeel_V4
{
    /**
     * Rotary paint order follows Vital-style layering (see Ultimate Sampler docs/vital_design_reference.md):
     * shadow stack -> body -> inactive arc -> active arc (hover: thickness x1.4) -> thumb.
     */
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                          float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider) override
    {
        auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(5.0f);
        const float radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f - 2.0f;
        const auto centre = bounds.getCentre();
        const float a0 = rotaryStartAngle;
        const float a1 = rotaryEndAngle;
        const float aVal = a0 + sliderPos * (a1 - a0);
        const float trackW = 4.35f;
        const float bodyR = juce::jmax(2.0f, radius - trackW - 0.75f);
        const bool dis = !slider.isEnabled();
        const bool hover = slider.isEnabled() && (slider.isMouseOverOrDragging() || slider.isMouseButtonDown());
        const float hoverBoost = hover ? 1.4f : 1.0f;
        const float activeStroke = juce::jmin(6.2f, (trackW + 0.15f) * hoverBoost);
        const float inactiveStroke = trackW * (hover ? 1.08f : 1.0f);

        const auto applyDis = [dis](juce::Colour c) {
            return dis ? c.withMultipliedAlpha(0.52f) : c;
        };

        razumov::ui::drawKnobSoftShadowStack(g, centre, radius, dis ? 0.55f : 1.f);

        // Knob face: radial gradient (soft top highlight, bottom shade).
        juce::ColourGradient faceGrad(
            applyDis(juce::Colour(tkn::argb::backgroundNode).brighter(0.07f)),
            centre.x,
            centre.y - bodyR * 0.35f,
            applyDis(juce::Colour(tkn::argb::backgroundNode).darker(0.14f)),
            centre.x,
            centre.y + bodyR * 0.55f,
            true);
        g.setGradientFill(faceGrad);
        g.fillEllipse(centre.x - bodyR, centre.y - bodyR, bodyR * 2.0f, bodyR * 2.0f);
        if (hover && !dis)
        {
            g.setColour(juce::Colour(tkn::argb::rotaryHoverLighten));
            g.fillEllipse(centre.x - bodyR * 0.92f, centre.y - bodyR * 0.95f, bodyR * 1.84f, bodyR * 0.55f);
        }
        g.setColour(applyDis(juce::Colour(0x28ffffff)));
        g.drawEllipse(centre.x - bodyR + 0.75f, centre.y - bodyR + 0.75f, (bodyR - 0.75f) * 2.0f, (bodyR - 0.75f) * 2.0f, 1.0f);
        g.setColour(applyDis(juce::Colour(0x22000000)));
        g.drawEllipse(centre.x - bodyR + 0.5f, centre.y - bodyR + 0.5f, (bodyR - 0.5f) * 2.0f, (bodyR - 0.5f) * 2.0f, 1.0f);

        const juce::Colour inactiveTrack = applyDis(juce::Colour(tkn::argb::rotaryTrackInactive));
        juce::Path trackBg;
        trackBg.addCentredArc(centre.x, centre.y, radius, radius, 0.0f, a0, a1, true);
        g.setColour(inactiveTrack.withMultipliedAlpha(0.28f));
        g.strokePath(trackBg, juce::PathStrokeType(inactiveStroke + 2.4f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour(inactiveTrack);
        g.strokePath(trackBg, juce::PathStrokeType(inactiveStroke, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        juce::Path val;
        val.addCentredArc(centre.x, centre.y, radius, radius, 0.0f, a0, aVal, true);
        const juce::Colour accent = applyDis(slider.findColour(juce::Slider::rotarySliderFillColourId));
        g.setColour(accent.withMultipliedAlpha(0.35f));
        g.strokePath(val, juce::PathStrokeType(activeStroke + 2.4f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour(accent);
        g.strokePath(val, juce::PathStrokeType(activeStroke, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour(accent.brighter(0.35f).withAlpha(dis ? 0.22f : 0.45f));
        g.strokePath(val, juce::PathStrokeType(1.1f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        const float dotR = 4.0f;
        const auto thumb = centre.getPointOnCircumference(radius - 0.5f, aVal);
        g.setColour(accent.darker(0.22f));
        g.fillEllipse(thumb.x - dotR - 0.35f, thumb.y - dotR - 0.35f, (dotR + 0.35f) * 2.0f, (dotR + 0.35f) * 2.0f);
        g.setColour(accent.brighter(0.12f));
        g.fillEllipse(thumb.x - dotR, thumb.y - dotR, dotR * 2.0f, dotR * 2.0f);
        g.setColour(juce::Colour(0x78000000).withMultipliedAlpha(dis ? 0.45f : 1.0f));
        g.drawEllipse(thumb.x - dotR, thumb.y - dotR, dotR * 2.0f, dotR * 2.0f, 1.05f);
    }

    void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown, int, int, int, int,
                      juce::ComboBox& box) override
    {
        const float corner = box.findParentComponentOfClass<juce::ChoicePropertyComponent>() != nullptr ? 0.0f : 7.0f;
        auto boxBounds = juce::Rectangle<int>(0, 0, width, height);
        auto f = boxBounds.toFloat().reduced(0.5f);
        const bool hover = box.isEnabled() && (box.isMouseOver() || box.isMouseButtonDown());
        const bool dis = !box.isEnabled();

        if (corner > 0.5f && !isButtonDown)
        {
            g.setColour(juce::Colour(tkn::argb::shadowElevated));
            g.fillRoundedRectangle(f.translated(0.0f, 3.0f), corner + 1.0f);
        }

        juce::Colour fill = juce::Colour(tkn::argb::backgroundNode);
        if (dis)
            fill = fill.withMultipliedAlpha(0.65f);
        else if (isButtonDown)
            fill = fill.darker(0.08f);
        else if (hover)
            fill = fill.brighter(0.07f);
        g.setColour(fill);
        g.fillRoundedRectangle(f, corner);

        juce::Colour outline = juce::Colour(tkn::argb::borderModulePanel);
        if (hover && !dis)
            outline = juce::Colour(tkn::argb::accentSignal).withAlpha(0.55f);
        g.setColour(outline);
        g.drawRoundedRectangle(f.reduced(0.5f), corner, dis ? 0.9f : 1.15f);

        juce::Rectangle<int> arrowZone(width - 30, 0, 20, height);
        juce::Path path;
        path.startNewSubPath((float) arrowZone.getX() + 3.0f, (float) arrowZone.getCentreY() - 2.0f);
        path.lineTo((float) arrowZone.getCentreX(), (float) arrowZone.getCentreY() + 3.0f);
        path.lineTo((float) arrowZone.getRight() - 3.0f, (float) arrowZone.getCentreY() - 2.0f);
        g.setColour(box.findColour(juce::ComboBox::arrowColourId).withAlpha(dis ? 0.25f : (hover ? 1.0f : 0.9f)));
        g.strokePath(path, juce::PathStrokeType(2.0f));
    }

    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour&,
                              bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        auto r = button.getLocalBounds().toFloat().reduced(1.0f);
        const auto fullRect = r;
        const bool hover = button.isOver();
        juce::Colour top = juce::Colour(tkn::argb::controlButtonFace);
        if (shouldDrawButtonAsDown)
            top = top.darker(0.14f);
        else if (shouldDrawButtonAsHighlighted || hover)
            top = top.brighter(0.06f);
        const juce::Colour bottom = top.darker(0.2f);
        juce::ColourGradient cg(top, r.getX(), r.getY(), bottom, r.getX(), r.getBottom(), false);
        g.setGradientFill(cg);
        g.fillRoundedRectangle(r, 6.0f);
        juce::Colour border = juce::Colour(tkn::argb::controlButtonBorder);
        if (hover && !shouldDrawButtonAsDown)
            border = juce::Colour(tkn::argb::accentSignal).withAlpha(0.42f);
        g.setColour(border);
        g.drawRoundedRectangle(r.reduced(0.5f), 6.0f, 1.0f);
        if (!shouldDrawButtonAsDown)
        {
            auto shadowBody = fullRect;
            g.setColour(juce::Colour(0x5c000000));
            g.fillRoundedRectangle(shadowBody.removeFromBottom(3.0f), 4.0f);
            g.setColour(juce::Colour(tkn::argb::shadowElevated));
            g.fillRoundedRectangle(shadowBody.removeFromBottom(1.5f), 4.0f);
        }
        g.setColour(juce::Colour(0x18ffffff));
        g.drawLine(fullRect.getX() + 1.5f, fullRect.getY() + 1.0f, fullRect.getRight() - 1.5f, fullRect.getY() + 1.0f, 1.0f);
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

void RazumovVocalChainAudioProcessorEditor::showChainContextMenu(razumov::ui::ChainContextTarget target,
                                                                 uint32_t slotId,
                                                                 juce::Point<int> screenPos)
{
    const auto opts = juce::PopupMenu::Options()
                          .withTargetScreenArea(juce::Rectangle<int>(screenPos.x, screenPos.y, 1, 1))
                          .withParentComponent(this);

    if (target == razumov::ui::ChainContextTarget::SerialPlus)
    {
        juce::PopupMenu m;
        m.addItem(100, "Add module...");
        if (processor.canInsertParallelSplitAfterSlot(slotId))
            m.addItem(101, "Parallel split...");
        m.showMenuAsync(opts, [this, slotId](int r) {
            if (r == 100)
                showAddModuleMenuForSlot(slotId);
            else if (r == 101)
                showParallelSplitMenuForSlot(slotId);
        });
        return;
    }
    if (target == razumov::ui::ChainContextTarget::ParallelPlus)
    {
        if (processor.canInsertParallelSplitAfterSlot(slotId))
            showParallelSplitMenuForSlot(slotId);
        return;
    }

    bool bypassed = false;
    bool found = false;
    for (const auto& it : processor.getChainStripItems())
    {
        if (it.slotId == slotId)
        {
            found = true;
            bypassed = it.bypassed;
            break;
        }
    }
    const bool canEdit = slotId != 0
                         && !razumov::graph::isProtectedFrontRootModuleSlot(processor.getGraphDesc(), slotId);
    juce::PopupMenu m;
    m.addItem(10, bypassed ? "Unbypass" : "Bypass", true, false);
    m.addItem(11, "Remove", canEdit, false);
    m.addItem(12, "Move earlier", canEdit, false);
    m.addItem(13, "Move later", canEdit, false);
    m.addItem(14, "Duplicate", processor.canDuplicateRootModuleSlot(slotId), false);
    m.addSeparator();
    m.addItem(16, "Add module after...");
    if (processor.canInsertParallelSplitAfterSlot(slotId))
        m.addItem(17, "Parallel split...");
    m.showMenuAsync(opts, [this, slotId, bypassed, found](int r) {
        if (r <= 0)
            return;
        if (r == 10 && found)
        {
            processor.setSlotBypassForId(slotId, !bypassed);
            syncChainStripAfterGraphEdit();
            return;
        }
        if (r == 11)
        {
            processor.removeGraphSlotById(slotId);
            syncChainStripAfterGraphEdit();
            return;
        }
        if (r == 12)
        {
            processor.moveRootSlotContainingId(slotId, -1);
            syncChainStripAfterGraphEdit();
            return;
        }
        if (r == 13)
        {
            processor.moveRootSlotContainingId(slotId, 1);
            syncChainStripAfterGraphEdit();
            return;
        }
        if (r == 14)
        {
            processor.duplicateRootModuleAfter(slotId);
            syncChainStripAfterGraphEdit();
            return;
        }
        if (r == 16)
            showAddModuleMenuForSlot(slotId);
        else if (r == 17)
            showParallelSplitMenuForSlot(slotId);
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
    chainStrip.onRequestSwapRootModules = [this](uint32_t a, uint32_t b) {
        processor.swapDirectRootModules(a, b);
        syncChainStripAfterGraphEdit();
    };
    chainStrip.onChainContextMenu = [this](razumov::ui::ChainContextTarget t, uint32_t id, juce::Point<int> pos) {
        showChainContextMenu(t, id, pos);
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
    viewport.setScrollBarsShown(false, false);

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

    if (sectionMacro_.getWidth() > 0)
    {
        g.setColour(juce::Colour(tkn::argb::surfaceModuleBackdrop));
        g.fillRoundedRectangle(sectionMacro_.toFloat(), 10.0f);
        g.setColour(juce::Colour(tkn::argb::borderModulePanel).withAlpha(0.55f));
        g.drawRoundedRectangle(sectionMacro_.toFloat().reduced(0.5f), 10.0f, 1.0f);
    }
    const int m = scaled(16);
    auto r = getLocalBounds().reduced(m);
    const int headerH = scaled(68);
    auto headerBg = r.removeFromTop(headerH);

    g.setColour(juce::Colour(tkn::argb::shadowElevated));
    g.fillRoundedRectangle(headerBg.translated(0, 3).toFloat(), 10.0f);

    juce::ColourGradient hg(
        juce::Colour(tkn::argb::backgroundNode).brighter(0.03f),
        (float) headerBg.getX(),
        (float) headerBg.getY(),
        juce::Colour(tkn::argb::backgroundChainStrip).brighter(0.02f),
        (float) headerBg.getX(),
        (float) headerBg.getBottom(),
        false);
    g.setGradientFill(hg);
    g.fillRoundedRectangle(headerBg.toFloat(), 10.0f);
    g.setColour(juce::Colour(tkn::argb::borderModulePanel));
    g.drawRoundedRectangle(headerBg.toFloat().reduced(0.5f), 10.0f, 1.0f);

    auto headerInner = headerBg.reduced(scaled(12), scaled(10));
    auto titleArea = headerInner.removeFromLeft(scaled(260));
    auto versionBlock = headerInner.removeFromRight(scaled(170));

    g.setColour(juce::Colour(tkn::argb::textTitle));
    g.setFont(juce::FontOptions(17.0f, juce::Font::bold));
    g.drawText("Razumov VocalChainer", titleArea, juce::Justification::centredLeft);

    const juce::String versionLine = juce::String("v") + JucePlugin_VersionString;
    const juce::String buildLine = juce::String("build ") + kBuildDateTime;

    g.setFont(juce::FontOptions(12.0f));
    g.setColour(juce::Colour(tkn::argb::textLabel));
    auto v1 = versionBlock.removeFromTop(18);
    g.drawText(versionLine, v1, juce::Justification::centredRight);
    g.setFont(juce::FontOptions(10.0f));
    g.setColour(juce::Colour(tkn::argb::textTertiary));
    g.drawText(buildLine, versionBlock, juce::Justification::centredRight);
}

void RazumovVocalChainAudioProcessorEditor::resized()
{
    uiScale_ = juce::jlimit(0.88f, 1.5f, (float) getWidth() / 1280.f);

    auto bounds = getLocalBounds().reduced(scaled(16));
    const int headerH = scaled(68);
    auto headerRow = bounds.removeFromTop(headerH);
    {
        auto h = headerRow.reduced(scaled(12), scaled(8));
        h.removeFromLeft(scaled(260));
        h.removeFromRight(scaled(170));
        presetLabel.setBounds(h.removeFromTop(scaled(14)));
        presetCombo.setBounds(h.removeFromTop(scaled(36)).reduced(0, 2));
    }

    const int macroBlockH = scaled(178);
    sectionMacro_ = juce::Rectangle<int>(bounds.getX(), bounds.getY(), bounds.getWidth(), macroBlockH);
    layoutMacroHeroRow(bounds.removeFromTop(macroBlockH));

    const int chainH = scaled(248);
    chainStrip.setBounds(bounds.removeFromTop(chainH));

    viewport.setBounds(bounds);
    layoutModuleViewport(viewport.getWidth());
    refreshRotaryStyles();
}

RazumovVocalChainAudioProcessorEditor::~RazumovVocalChainAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}
