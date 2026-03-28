#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace razumov::ui
{

/** Tiled PNG texture + base fill (see Resources/ui + docs/design/UI_ASSETS.md). */
void drawEditorBackgroundLayer(juce::Graphics& g, juce::Rectangle<int> area);

/** Embedded SVG corner accents (top left / top right). */
void drawEditorCornerAccents(juce::Graphics& g, juce::Rectangle<int> fullEditorBounds);

/** Soft multi-pass outline for selected chain slot (glow-style). */
void drawRoundedRectGlow(juce::Graphics& g, juce::Rectangle<float> card, float cornerRadius,
                         juce::Colour glowColour, int numLayers = 5);

} // namespace razumov::ui
