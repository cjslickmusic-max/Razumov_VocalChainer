#include "EditorVisualAssets.h"

#if JUCE_TARGET_HAS_BINARY_DATA
#include <BinaryData.h>
#endif

namespace razumov::ui
{
namespace
{
#if JUCE_TARGET_HAS_BINARY_DATA
juce::Image loadBrushedTexture()
{
    static const juce::Image tex = [] {
        juce::Image i = juce::ImageFileFormat::loadFrom(BinaryData::brushed_panel_placeholder_png,
                                                        BinaryData::brushed_panel_placeholder_pngSize);
        return i;
    }();
    return tex;
}
#endif
} // namespace

void drawEditorBackgroundLayer(juce::Graphics& g, juce::Rectangle<int> area)
{
#if JUCE_TARGET_HAS_BINARY_DATA
    const juce::Image tex = loadBrushedTexture();
    if (!tex.isNull())
    {
        const int tw = tex.getWidth();
        const int th = tex.getHeight();
        g.setOpacity(0.15f);
        for (int y = area.getY(); y < area.getBottom(); y += th)
        {
            for (int x = area.getX(); x < area.getRight(); x += tw)
            {
                const int w = juce::jmin(tw, area.getRight() - x);
                const int h = juce::jmin(th, area.getBottom() - y);
                g.drawImage(tex, x, y, w, h, 0, 0, w, h);
            }
        }
        g.setOpacity(1.0f);
    }
#else
    juce::ignoreUnused(g, area);
#endif
}

void drawEditorCornerAccents(juce::Graphics& g, juce::Rectangle<int> fullEditorBounds)
{
#if JUCE_TARGET_HAS_BINARY_DATA
    auto drawCorner = [&g](const void* data, int size, float x, float y) {
        if (auto d = juce::Drawable::createFromImageData(data, (size_t) size))
        {
            const auto box = d->getDrawableBounds();
            d->draw(g, 1.0f, juce::AffineTransform::translation(x - box.getX(), y - box.getY()));
        }
    };
    const float margin = 10.0f;
    drawCorner(BinaryData::editor_corner_accent_svg, BinaryData::editor_corner_accent_svgSize,
               (float) fullEditorBounds.getX() + margin, (float) fullEditorBounds.getY() + margin);
    if (auto d = juce::Drawable::createFromImageData(BinaryData::editor_corner_accent_r_svg,
                                                     BinaryData::editor_corner_accent_r_svgSize))
    {
        const auto box = d->getDrawableBounds();
        const float x = (float) fullEditorBounds.getRight() - margin - box.getWidth();
        d->draw(g, 1.0f, juce::AffineTransform::translation(x - box.getX(), (float) fullEditorBounds.getY() + margin - box.getY()));
    }
#else
    juce::ignoreUnused(g, fullEditorBounds);
#endif
}

void drawRoundedRectGlow(juce::Graphics& g, juce::Rectangle<float> card, float cornerRadius,
                         juce::Colour glowColour, int numLayers)
{
    for (int i = numLayers; i >= 1; --i)
    {
        const float expand = (float) i * 2.2f;
        const float alpha = 0.028f + 0.022f * (float) (numLayers + 1 - i);
        g.setColour(glowColour.withAlpha(juce::jlimit(0.0f, 1.0f, alpha)));
        g.drawRoundedRectangle(card.expanded(expand), cornerRadius + expand * 0.15f, 1.0f + (float) i * 0.4f);
    }
}

} // namespace razumov::ui
