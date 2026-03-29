#pragma once

#include <cstdint>

namespace razumov::ui::tokens
{

/**
 * Opaque colours as 0xAARRGGBB for juce::Colour(...).
 * Sync with resources/design/tokens/tokens.json (color.*.background / text / accent).
 * Light glass theme: global uiScale still applied in editor for resize.
 */
namespace argb
{
inline constexpr uint32_t backgroundEditor = 0xffe6e9ef;
inline constexpr uint32_t backgroundChainStrip = 0xffdce1ea;
inline constexpr uint32_t backgroundNode = 0xfff2f4f8;
inline constexpr uint32_t backgroundMergeNode = 0xffe8ecf4;

inline constexpr uint32_t surfaceMicPreviewInner = 0xffeef1f6;
inline constexpr uint32_t surfaceModuleBackdrop = 0xfff7f8fb;

inline constexpr uint32_t borderMicPreview = 0xffb8c0d0;
inline constexpr uint32_t borderModulePanel = 0xffc5ccd8;

inline constexpr uint32_t textPrimary = 0xff1a1f2a;
inline constexpr uint32_t textSecondary = 0xff5a6578;
inline constexpr uint32_t textLabel = 0xff3d4a5c;
inline constexpr uint32_t textTertiary = 0xff7a8494;
inline constexpr uint32_t textCaption = 0xff6a7488;
inline constexpr uint32_t textTitle = 0xff0f141c;

inline constexpr uint32_t controlRotaryOutline = 0xffaeb8c8;

inline constexpr uint32_t accentSignal = 0xff5b8fd4;
inline constexpr uint32_t accentSelection = 0xff2563eb;
inline constexpr uint32_t accentBypass = 0xffd97757;

inline constexpr uint32_t controlButtonFace = 0xfff8f9fc;
inline constexpr uint32_t controlButtonFaceOver = 0xffffffff;
inline constexpr uint32_t controlButtonBorder = 0xffb0b8c8;
inline constexpr uint32_t controlButtonShadow = 0x40000000;
/** Drop shadow under elevated controls (cards, knobs) — visible on light panels. */
inline constexpr uint32_t shadowElevated = 0x6e000000;
/** Broad diffuse shadow under rotary (cool grey-blue, soft UI). */
inline constexpr uint32_t shadowRotaryAmbient = 0x45607080;
/** Mid layer under rotary (neutral). */
inline constexpr uint32_t shadowRotaryMid = 0x42000000;
/** Tight contact shadow at knob base. */
inline constexpr uint32_t shadowRotaryContact = 0x62000000;
/** Inactive arc / track background (near-black for contrast with accent). */
inline constexpr uint32_t rotaryTrackInactive = 0xff1e222a;
/** Hover overlay on knob body (soft UI). */
inline constexpr uint32_t rotaryHoverLighten = 0x1effffff;
} // namespace argb

/** Macro row: fixed accent hues (readable on light panels). */
namespace macro
{
inline constexpr uint32_t sectionLabel = 0xff2d3748;
inline constexpr uint32_t glue = 0xffb84d8a;
inline constexpr uint32_t air = 0xff2aa8c4;
inline constexpr uint32_t sibilance = 0xffd9a23c;
inline constexpr uint32_t presence = 0xff4caf7c;
inline constexpr uint32_t punch = 0xffe07040;
inline constexpr uint32_t body = 0xffa08060;
inline constexpr uint32_t smooth = 0xff6078d0;
inline constexpr uint32_t density = 0xff48a898;
} // namespace macro

/** Per-module rotary accent (addKnob fill colour). */
namespace knob
{
inline constexpr uint32_t micAmount = 0xff7c6fd4;
inline constexpr uint32_t gain = 0xff4a8fd4;
inline constexpr uint32_t lowpass = 0xff5cb870;
inline constexpr uint32_t deess = 0xffd9a050;
inline constexpr uint32_t opto = 0xff4a9fd0;
inline constexpr uint32_t fet = 0xffd05088;
inline constexpr uint32_t vca = 0xff88b050;
inline constexpr uint32_t exciter = 0xffd0a868;
inline constexpr uint32_t spectral = 0xff8888d8;
} // namespace knob

} // namespace razumov::ui::tokens
