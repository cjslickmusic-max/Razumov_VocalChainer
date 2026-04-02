#include "SpectralCompressorNode.h"

#include <array>
#include <cmath>

namespace razumov::graph
{

namespace
{

float compressMag(float mag, float thresholdDb, float ratio) noexcept
{
    const float inDb = 20.0f * std::log10(mag + 1.0e-20f);
    const float over = juce::jmax(0.0f, inDb - thresholdDb);
    const float outDb = inDb - over * (1.0f - 1.0f / ratio);
    return std::pow(10.0f, outDb * 0.05f);
}

} // namespace

struct SpectralCompressorNode::ChannelData
{
    static constexpr size_t olaSize = 65536;

    std::vector<float> inputRing;
    std::vector<float> frame;
    int writePos { 0 };
    uint64_t inputSampleCounter { 0 };
    std::vector<float> ola;
    std::vector<juce::dsp::Complex<float>> timeDomain;
    std::vector<juce::dsp::Complex<float>> freqDomain;

    void prepare(int fftSize, int maxBlockHint)
    {
        juce::ignoreUnused(maxBlockHint);
        inputRing.assign((size_t) fftSize, 0.0f);
        frame.assign((size_t) fftSize, 0.0f);
        ola.assign(olaSize, 0.0f);
        timeDomain.assign((size_t) fftSize, {});
        freqDomain.assign((size_t) fftSize, {});
        writePos = 0;
        inputSampleCounter = 0;
    }

    void reset() noexcept
    {
        std::fill(inputRing.begin(), inputRing.end(), 0.0f);
        std::fill(frame.begin(), frame.end(), 0.0f);
        std::fill(ola.begin(), ola.end(), 0.0f);
        writePos = 0;
        inputSampleCounter = 0;
    }

    void processFrame(int fftSize,
                      const float* window,
                      juce::dsp::FFT& fft,
                      float thresholdDb,
                      float ratio,
                      SpectralCompressorNode& owner,
                      int channelIndex)
    {
        for (int i = 0; i < fftSize; ++i)
        {
            const int idx = (writePos - fftSize + i + fftSize) % fftSize;
            frame[(size_t) i] = inputRing[(size_t) idx] * window[(size_t) i];
        }

        for (int i = 0; i < fftSize; ++i)
            timeDomain[(size_t) i] = juce::dsp::Complex<float> { frame[(size_t) i], 0.0f };

        fft.perform(timeDomain.data(), freqDomain.data(), false);

        const int half = fftSize / 2;
        std::array<float, 513> magIn {};
        const int magCount = juce::jmin(half + 1, (int) magIn.size());
        for (int k = 0; k < magCount; ++k)
        {
            const auto& c = freqDomain[(size_t) k];
            const float re = c.real();
            const float im = c.imag();
            magIn[(size_t) k] = std::sqrt(re * re + im * im);
        }

        for (int k = 0; k <= half; ++k)
        {
            auto& c = freqDomain[(size_t) k];
            const float re = c.real();
            const float im = c.imag();
            const float mag = magIn[(size_t) k];
            const float m2 = compressMag(mag, thresholdDb, ratio);
            const float g = m2 / (mag + 1.0e-20f);
            c = { re * g, im * g };
        }

        if (channelIndex == 0)
            owner.commitSpectralMeterFrame(half, magIn.data(), thresholdDb, ratio);

        freqDomain[0] = { freqDomain[0].real(), 0.0f };
        freqDomain[(size_t) half] = { freqDomain[(size_t) half].real(), 0.0f };

        for (int k = 1; k < half; ++k)
            freqDomain[(size_t) (fftSize - k)] = std::conj(freqDomain[(size_t) k]);

        fft.perform(freqDomain.data(), timeDomain.data(), true);

        const float invN = 1.0f / (float) fftSize;
        const uint64_t base = inputSampleCounter - (uint64_t) fftSize;

        for (int i = 0; i < fftSize; ++i)
        {
            const float syn = timeDomain[(size_t) i].real() * invN * window[(size_t) i];
            const size_t idx = (size_t) ((base + (uint64_t) i) % olaSize);
            ola[idx] += syn;
        }
    }

    void pushSample(float x,
                    int fftSize,
                    int hop,
                    const float* window,
                    juce::dsp::FFT& fft,
                    float thresholdDb,
                    float ratio,
                    SpectralCompressorNode& owner,
                    int channelIndex)
    {
        inputRing[(size_t) writePos] = x;
        writePos = (writePos + 1) % fftSize;
        ++inputSampleCounter;

        if (inputSampleCounter >= (uint64_t) fftSize
            && ((inputSampleCounter - (uint64_t) fftSize) % (uint64_t) hop) == 0)
            processFrame(fftSize, window, fft, thresholdDb, ratio, owner, channelIndex);
    }

    float getWetOutput(int latencySamples) noexcept
    {
        if (inputSampleCounter < (uint64_t) latencySamples)
            return 0.0f;

        const uint64_t readIdx = inputSampleCounter - (uint64_t) latencySamples;
        const size_t idx = (size_t) (readIdx % olaSize);
        const float y = ola[idx];
        ola[idx] = 0.0f;
        return y;
    }
};

void SpectralCompressorNode::commitSpectralMeterFrame(int half,
                                                      const float* magIn,
                                                      float thresholdDb,
                                                      float ratio) noexcept
{
    constexpr int nBins = kSpectralDisplayBins;
    std::array<float, nBins> frameIn {};
    std::array<float, nBins> frameRed {};

    for (int b = 0; b < nBins; ++b)
    {
        const int k0 = b * (half + 1) / nBins;
        const int k1 = (b + 1) * (half + 1) / nBins;
        float maxInNorm = 0.f;
        float maxRedNorm = 0.f;
        for (int k = k0; k < k1 && k <= half; ++k)
        {
            const float mag = magIn[(size_t) k];
            const float m2 = compressMag(mag, thresholdDb, ratio);
            const float inDb = 20.0f * std::log10(mag + 1.0e-20f);
            const float normIn = juce::jlimit(0.f, 1.f, (inDb + 90.f) / 90.f);
            const float outDb = 20.0f * std::log10(m2 + 1.0e-20f);
            const float redDb = juce::jmax(0.f, inDb - outDb);
            const float redNorm = juce::jlimit(0.f, 1.f, redDb / 24.f);
            maxInNorm = juce::jmax(maxInNorm, normIn);
            maxRedNorm = juce::jmax(maxRedNorm, redNorm);
        }
        frameIn[(size_t) b] = maxInNorm;
        frameRed[(size_t) b] = maxRedNorm;
    }

    constexpr float rel = 0.88f;
    for (int i = 0; i < nBins; ++i)
    {
        const float prevIn = specInNorm_[(size_t) i].load(std::memory_order_relaxed);
        const float prevR = specRedNorm_[(size_t) i].load(std::memory_order_relaxed);
        const float curIn = frameIn[(size_t) i];
        const float curR = frameRed[(size_t) i];
        float newIn = (curIn > prevIn) ? curIn : prevIn * rel + curIn * (1.f - rel);
        float newR = (curR > prevR) ? curR : prevR * rel + curR * (1.f - rel);
        newIn = juce::jlimit(0.f, 1.f, newIn);
        newR = juce::jlimit(0.f, 1.f, newR);
        specInNorm_[(size_t) i].store(newIn, std::memory_order_relaxed);
        specRedNorm_[(size_t) i].store(newR, std::memory_order_relaxed);
    }
}

bool SpectralCompressorNode::copySpectralCompressionDisplay256(float* inNorm256, float* redNorm256) const noexcept
{
    if (inNorm256 == nullptr || redNorm256 == nullptr)
        return false;
    for (int i = 0; i < kSpectralDisplayBins; ++i)
    {
        inNorm256[i] = specInNorm_[(size_t) i].load(std::memory_order_relaxed);
        redNorm256[i] = specRedNorm_[(size_t) i].load(std::memory_order_relaxed);
    }
    return true;
}

SpectralCompressorNode::SpectralCompressorNode() = default;

SpectralCompressorNode::~SpectralCompressorNode() = default;

int SpectralCompressorNode::getLatencySamples() const noexcept
{
    return bypass_ ? 0 : kLatencySamples_;
}

void SpectralCompressorNode::prepare(double sampleRate, int maxBlockSize, int numChannels)
{
    juce::ignoreUnused(sampleRate);
    const int mb = juce::jmax(1, maxBlockSize);
    numChannels_ = juce::jmax(1, numChannels);

    fft_ = std::make_unique<juce::dsp::FFT>(fftOrder_);

    window_.resize((size_t) fftSize_);
    for (int i = 0; i < fftSize_; ++i)
    {
        const float t = (float) i / (float) (fftSize_ - 1);
        window_[(size_t) i] = 0.5f * (1.0f - std::cos(2.0f * juce::MathConstants<float>::pi * t));
    }

    dryPad_ = std::make_unique<MergeDelayPad>();
    dryPad_->prepare(numChannels_, mb, kLatencySamples_ + mb + 32);

    dryScratch_.setSize(numChannels_, mb);

    channels_.clear();
    channels_.reserve((size_t) numChannels_);
    for (int c = 0; c < numChannels_; ++c)
    {
        auto ch = std::make_unique<SpectralCompressorNode::ChannelData>();
        ch->prepare(fftSize_, mb);
        channels_.push_back(std::move(ch));
    }
}

void SpectralCompressorNode::reset()
{
    if (dryPad_ != nullptr)
        dryPad_->setDelaySamples(0);

    for (auto& ch : channels_)
        if (ch != nullptr)
            ch->reset();

    for (auto& a : specInNorm_)
        a.store(0.f, std::memory_order_relaxed);
    for (auto& a : specRedNorm_)
        a.store(0.f, std::memory_order_relaxed);
}

void SpectralCompressorNode::process(juce::AudioBuffer<float>& buffer)
{
    if (bypass_)
    {
        for (auto& a : specInNorm_)
            a.store(0.f, std::memory_order_relaxed);
        for (auto& a : specRedNorm_)
            a.store(0.f, std::memory_order_relaxed);
        return;
    }

    const int n = buffer.getNumSamples();
    const int ch = juce::jmin(buffer.getNumChannels(), numChannels_, (int) channels_.size());
    if (n <= 0 || ch <= 0 || fft_ == nullptr)
        return;

    dryScratch_.setSize(ch, n, false, false, false);
    for (int c = 0; c < ch; ++c)
        dryScratch_.copyFrom(c, 0, buffer, c, 0, n);

    dryPad_->setDelaySamples(kLatencySamples_);
    dryPad_->process(dryScratch_);

    const float mix = mix_;
    const float dryMix = 1.0f - mix;
    const float thr = thresholdDb_;
    const float rat = ratio_;

    for (int i = 0; i < n; ++i)
    {
        for (int c = 0; c < ch; ++c)
        {
            const float x = buffer.getSample(c, i);
            channels_[(size_t) c]->pushSample(x, fftSize_, hop_, window_.data(), *fft_, thr, rat, *this, c);
            const float wet = channels_[(size_t) c]->getWetOutput(kLatencySamples_);
            const float dry = dryScratch_.getSample(c, i);
            buffer.setSample(c, i, wet * mix + dry * dryMix);
        }
    }
}

} // namespace razumov::graph
