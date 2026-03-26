#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace razumov::presets
{

/** Количество встроенных пресетов (банк фазы 4). */
int getNumFactoryPresets() noexcept;

/** Имя для UI / getProgramName (UTF-8). */
const char* getFactoryPresetName(int index) noexcept;

/** Применить пресет к APVTS (message / audio thread — вызывать с той же дисциплиной, что и setValue). */
void applyFactoryPreset(int index, juce::AudioProcessorValueTreeState& apvts);

} // namespace razumov::presets
