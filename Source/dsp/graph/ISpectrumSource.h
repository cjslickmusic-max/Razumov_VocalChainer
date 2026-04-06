#pragma once

#include <cstdint>

namespace razumov::graph
{

/** Узел, отдающий нормализованные (0...1) уровни спектра для UI (чтение с message thread). */
class ISpectrumSource
{
public:
    static constexpr int kSpectrumBins = 512;

    virtual ~ISpectrumSource() = default;
    virtual void copySpectrum256(float* dst) const noexcept = 0;
};

} // namespace razumov::graph
