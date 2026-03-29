#pragma once

#include <cstdint>

namespace razumov::ui::tokens
{

/**
 * Opaque colours as 0xAARRGGBB for juce::Colour(...).
 * Sync with resources/design/tokens/tokens.json (color.*.background / text / accent opaque).
 * When changing a product colour: update tokens.json first, then this file, then tokens.css.
 */
namespace argb
{
inline constexpr uint32_t backgroundEditor = 0xff1a1d23;
inline constexpr uint32_t backgroundChainStrip = 0xff252830;
inline constexpr uint32_t backgroundNode = 0xff1e2229;
inline constexpr uint32_t backgroundMergeNode = 0xff222630;

inline constexpr uint32_t textPrimary = 0xffe8eaed;
inline constexpr uint32_t textSecondary = 0xff8892a0;
inline constexpr uint32_t textLabel = 0xffaab4c0;

inline constexpr uint32_t accentSignal = 0xff6c9fd2;
inline constexpr uint32_t accentSelection = 0xff9fd2ff;
inline constexpr uint32_t accentBypass = 0xffe89868;
} // namespace argb

} // namespace razumov::ui::tokens
