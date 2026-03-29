#include "EditorVisualAssets.h"
#include "DesignTokens.h"
#include <juce_graphics/juce_graphics.h>

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
        g.setOpacity(0.07f);
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

namespace
{
struct KnobShadowRasters
{
    juce::Image ambient;
    juce::Image contact;
};

/** One-time Gaussian blur maps (white blob -> alpha); tinted at draw time. */
const KnobShadowRasters& getKnobShadowRasters()
{
    static const KnobShadowRasters maps = [] {
        KnobShadowRasters m;
        constexpr int W = 200;
        constexpr int H = 200;

        {
            juce::Image img(juce::Image::ARGB, W, H, true);
            juce::Graphics g2(img);
            g2.fillAll(juce::Colours::transparentBlack);
            g2.setColour(juce::Colours::white);
            g2.fillEllipse(48.0f, 36.0f, 104.0f, 90.0f);
            juce::ImageConvolutionKernel kernel(31);
            kernel.createGaussianBlur(9.5f);
            juce::Image out = img.createCopy();
            kernel.applyToImage(out, img, img.getBounds());
            m.ambient = std::move(out);
        }
        {
            juce::Image img(juce::Image::ARGB, W, H, true);
            juce::Graphics g2(img);
            g2.fillAll(juce::Colours::transparentBlack);
            g2.setColour(juce::Colours::white);
            g2.fillEllipse(70.0f, 92.0f, 60.0f, 30.0f);
            juce::ImageConvolutionKernel kernel(17);
            kernel.createGaussianBlur(4.2f);
            juce::Image out = img.createCopy();
            kernel.applyToImage(out, img, img.getBounds());
            m.contact = std::move(out);
        }
        return m;
    }();
    return maps;
}

/** Ellipse centers before blur (maps are 200x200). */
constexpr float kAmbientRefX = 100.0f;
constexpr float kAmbientRefY = 81.0f;
constexpr float kContactRefX = 100.0f;
constexpr float kContactRefY = 107.0f;
} // namespace

void drawKnobSoftShadowStack(juce::Graphics& g, juce::Point<float> centre, float radius, float alphaMul)
{
    const auto& maps = getKnobShadowRasters();
    const float squash = 0.88f;
    const float yBias = 7.0f;

    g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);

    const float ambS = (radius + 5.5f) / 52.0f;
    {
        const float w = (float) maps.ambient.getWidth() * ambS;
        const float h = (float) maps.ambient.getHeight() * ambS * squash;
        const float dx = centre.x - kAmbientRefX * ambS;
        const float dy = centre.y + yBias - kAmbientRefY * ambS * squash;
        g.setColour(juce::Colour(razumov::ui::tokens::argb::shadowRotaryAmbient).withMultipliedAlpha(0.72f * alphaMul));
        g.drawImage(maps.ambient,
                    (int) std::floor(dx),
                    (int) std::floor(dy),
                    (int) std::ceil(w),
                    (int) std::ceil(h),
                    0,
                    0,
                    maps.ambient.getWidth(),
                    maps.ambient.getHeight(),
                    true);
    }

    const float ctS = (radius + 4.0f) / 52.0f;
    {
        const float w = (float) maps.contact.getWidth() * ctS;
        const float h = (float) maps.contact.getHeight() * ctS * 0.82f;
        const float dx = centre.x - kContactRefX * ctS;
        const float dy = centre.y + 4.0f - kContactRefY * ctS * 0.82f;
        g.setColour(juce::Colour(razumov::ui::tokens::argb::shadowRotaryContact).withMultipliedAlpha(0.88f * alphaMul));
        g.drawImage(maps.contact,
                    (int) std::floor(dx),
                    (int) std::floor(dy),
                    (int) std::ceil(w),
                    (int) std::ceil(h),
                    0,
                    0,
                    maps.contact.getWidth(),
                    maps.contact.getHeight(),
                    true);
    }

    g.setImageResamplingQuality(juce::Graphics::mediumResamplingQuality);
}

} // namespace razumov::ui
