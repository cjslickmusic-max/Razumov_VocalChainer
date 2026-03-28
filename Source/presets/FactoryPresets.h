#pragma once

#include "params/Phase3RealtimeParams.h"
#include <juce_audio_processors/juce_audio_processors.h>

namespace razumov::presets
{

/** Количество встроенных пресетов (банк фазы 4). */
int getNumFactoryPresets() noexcept;

/** Имя для UI / getProgramName (UTF-8). */
const char* getFactoryPresetName(int index) noexcept;

/** Глобальные APVTS: mic profile, макросы (цепочка chainProfile не меняется пресетом). */
void applyFactoryPresetGlobals(int index, juce::AudioProcessorValueTreeState& apvts);

/** Снимок параметров модулей (одинаковый для всех слотов), без учёта макросов в DSP. */
razumov::params::Phase3RealtimeParams buildFactoryPresetModulePhase3(int index);

} // namespace razumov::presets
