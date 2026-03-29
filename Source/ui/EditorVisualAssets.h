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

/**
 * Gaussian-blurred drop shadow under a rotary knob (cached raster, tinted via design tokens).
 * alphaMul scales both layers (e.g. disabled state).
 */
void drawKnobSoftShadowStack(juce::Graphics& g, juce::Point<float> centre, float radius, float alphaMul);

} // namespace razumov::ui
