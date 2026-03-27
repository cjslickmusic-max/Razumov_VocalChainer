#pragma once

#include "FlexGraphDesc.h"
#include <memory>

namespace razumov::graph
{

class AudioNode;

/** Создание DSP-узла по описанию модуля (без Split). */
std::unique_ptr<AudioNode> createAudioNodeFromModuleDesc(const FlexSlotDesc& moduleDesc);

} // namespace razumov::graph
