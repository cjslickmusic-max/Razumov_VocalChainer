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
/** Warm stone (not cold blue-grey): studio-adjacent, readable with slate text. */
inline constexpr uint32_t backgroundEditor = 0xffebe8e3;
/** Bottom vignette overlay in PluginEditor::paint (low alpha warm brown). */
inline constexpr uint32_t backgroundAtmosphereBottom = 0x14908070;
inline constexpr uint32_t backgroundChainStrip = 0xffddd8d2;
inline constexpr uint32_t backgroundNode = 0xfff5f2ee;
inline constexpr uint32_t backgroundMergeNode = 0xffe5e0da;

inline constexpr uint32_t surfaceMicPreviewInner = 0xfff0ebe7;
inline constexpr uint32_t surfaceModuleBackdrop = 0xfff2ede8;

inline constexpr uint32_t borderMicPreview = 0xffb8b0a8;
inline constexpr uint32_t borderModulePanel = 0xffc4bcb4;

inline constexpr uint32_t textPrimary = 0xff1a1f2a;
inline constexpr uint32_t textSecondary = 0xff5a6578;
inline constexpr uint32_t textLabel = 0xff3d4a5c;
inline constexpr uint32_t textTertiary = 0xff7a8494;
inline constexpr uint32_t textCaption = 0xff6a7488;
inline constexpr uint32_t textTitle = 0xff0f141c;

inline constexpr uint32_t controlRotaryOutline = 0xffaeb8c8;
/** Rotary numeric field: readable on light glass (not white-on-white). */
inline constexpr uint32_t rotaryValueBoxFill = 0xfff3eee9;
inline constexpr uint32_t rotaryValueBoxText = 0xff1a1f2a;

inline constexpr uint32_t accentSignal = 0xff5b82c8;
inline constexpr uint32_t accentSelection = 0xff2563eb;
inline constexpr uint32_t accentBypass = 0xffd97757;

inline constexpr uint32_t controlButtonFace = 0xfffaf8f6;
inline constexpr uint32_t controlButtonFaceOver = 0xffffffff;
inline constexpr uint32_t controlButtonBorder = 0xffb0b8c8;
inline constexpr uint32_t controlButtonShadow = 0x40000000;
/** Drop shadow under elevated controls (cards, knobs) — visible on light panels. */
inline constexpr uint32_t shadowElevated = 0x6e000000;
/** Broad diffuse shadow under rotary (warm neutral, soft UI). */
inline constexpr uint32_t shadowRotaryAmbient = 0x455a5850;
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

/**
 * Dark EQ plot (resources/design/tokens/tokens.css -- web chain prototype).
 * JUCE module area is light; EQ card uses this inset palette for contrast.
 */
namespace eqPanel
{
inline constexpr uint32_t plotGradientTop = 0x4d312e81;     /** indigo tint */
inline constexpr uint32_t plotGradientMid = 0xee14141f;
inline constexpr uint32_t plotGradientBottom = 0xff0a0a14;
inline constexpr uint32_t plotInnerGlow = 0x1a6c9fd2;         /** wire glow */
inline constexpr uint32_t spectrumFillHi = 0x556c9fd2;        /** accent-signal */
inline constexpr uint32_t spectrumFillLo = 0x186c9fd2;
inline constexpr uint32_t spectrumLine = 0xb0a8c8e8;
inline constexpr uint32_t curveGlow = 0x559fd2ff;           /** accent-selection */
inline constexpr uint32_t curveCore = 0xeec8e0ff;
inline constexpr uint32_t gridLine = 0x1effffff;            /** border-subtle */
inline constexpr uint32_t zeroDbLine = 0x5a9fd2ff;
inline constexpr uint32_t frameBorder = 0x55353d4a;
inline constexpr uint32_t captionText = 0xff94a3b8;          /** text-muted */
/** Band handles (distinct on dark; signal / CTA / merge / green / cyan). */
inline constexpr uint32_t band0 = 0xff6c9fd2;
inline constexpr uint32_t band1 = 0xfff97316;
inline constexpr uint32_t band2 = 0xffa78bfa;
inline constexpr uint32_t band3 = 0xff4ade80;
inline constexpr uint32_t band4 = 0xff22d3ee;
inline constexpr uint32_t band5 = 0xffe879f9;
inline constexpr uint32_t band6 = 0xfffacc15;
inline constexpr uint32_t band7 = 0xfffb7185;
inline constexpr uint32_t band8 = 0xff34d399;
inline constexpr uint32_t band9 = 0xff818cf8;
inline constexpr uint32_t nodeLabel = 0xfff8fafc;            /** text-inverse */
} // namespace eqPanel

} // namespace razumov::ui::tokens
