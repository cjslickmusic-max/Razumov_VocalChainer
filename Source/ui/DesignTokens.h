#pragma once

#include <cstdint>

namespace razumov::ui::tokens
{

/**
 * Opaque colours as 0xAARRGGBB for juce::Colour(...).
 * Sync with resources/design/tokens/tokens.json (color.*.background / text / accent).
 * When changing a product colour: update tokens.json first, then this file, then tokens.css.
 */
namespace argb
{
inline constexpr uint32_t backgroundEditor = 0xff1a1d23;
inline constexpr uint32_t backgroundChainStrip = 0xff252830;
inline constexpr uint32_t backgroundNode = 0xff1e2229;
inline constexpr uint32_t backgroundMergeNode = 0xff222630;

inline constexpr uint32_t surfaceMicPreviewInner = 0xff252b34;
inline constexpr uint32_t surfaceModuleBackdrop = 0xff1e2229;

inline constexpr uint32_t borderMicPreview = 0xff4a5568;
inline constexpr uint32_t borderModulePanel = 0xff353d4a;

inline constexpr uint32_t textPrimary = 0xffe8eaed;
inline constexpr uint32_t textSecondary = 0xff8892a0;
inline constexpr uint32_t textLabel = 0xffaab4c0;
inline constexpr uint32_t textTertiary = 0xff7a8490;
inline constexpr uint32_t textCaption = 0xff6a7480;
inline constexpr uint32_t textTitle = 0xffffffff;

inline constexpr uint32_t controlRotaryOutline = 0xff353d48;

inline constexpr uint32_t accentSignal = 0xff6c9fd2;
inline constexpr uint32_t accentSelection = 0xff9fd2ff;
inline constexpr uint32_t accentBypass = 0xffe89868;
} // namespace argb

/** Macro row: distinct fill colour per macro knob (matches APVTS macro mapping). */
namespace macro
{
inline constexpr uint32_t sectionLabel = 0xffc8d0dc;
inline constexpr uint32_t glue = 0xffc080a0;
inline constexpr uint32_t air = 0xff80c0c8;
inline constexpr uint32_t sibilance = 0xffd0a868;
inline constexpr uint32_t presence = 0xffa8d080;
inline constexpr uint32_t punch = 0xffe88860;
inline constexpr uint32_t body = 0xffb89870;
inline constexpr uint32_t smooth = 0xff8898d0;
inline constexpr uint32_t density = 0xff78b0a0;
} // namespace macro

/** Per-module rotary accent (addKnob fill colour). */
namespace knob
{
inline constexpr uint32_t micAmount = 0xff9a86d4;
inline constexpr uint32_t gain = 0xff6c9fd2;
inline constexpr uint32_t lowpass = 0xff8fd28c;
inline constexpr uint32_t deess = 0xffd2a86c;
inline constexpr uint32_t opto = 0xff6cb8d2;
inline constexpr uint32_t fet = 0xffd26c8c;
inline constexpr uint32_t vca = 0xffb8d26c;
inline constexpr uint32_t exciter = 0xffe0c080;
inline constexpr uint32_t spectral = 0xff9a9ae0;
} // namespace knob

} // namespace razumov::ui::tokens
