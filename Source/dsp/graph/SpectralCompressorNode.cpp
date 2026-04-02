#include "SpectralCompressorNode.h"

#include <array>
#include <cmath>

namespace razumov::graph
{

namespace
{

void buildSidechainWeights(double sampleRate,
                           int fftSize,
                           int half,
                           float fcHz,
                           float Q,
                           float* wOut) noexcept
{
    const float nyq = (float) (sampleRate * 0.5);
    const float fc = juce::jlimit(20.0f, juce::jmin(nyq * 0.99f, 20000.0f), fcHz);
    const float sigmaHz = fc / juce::jmax(0.25f, Q * 2.0f);

    wOut[0] = 0.0f;
    float sumW = 0.0f;
    for (int k = 1; k <= half; ++k)
    {
        const float fk = (float) k * (float) sampleRate / (float) fftSize;
        const float d = (fk - fc) / juce::jmax(1.0f, sigmaHz);
        const float ww = std::exp(-d * d);
        wOut[(size_t) k] = ww;
        sumW += ww;
    }

    if (sumW > 1.0e-12f)
    {
        const float inv = 1.0f / sumW;
        for (int k = 1; k <= half; ++k)
            wOut[(size_t) k] *= inv;
    }
}

float weightedPowerDb(const float* magIn, const float* w, int half) noexcept
{
    float sumP = 0.0f;
    for (int k = 1; k <= half; ++k)
    {
        const float m = magIn[(size_t) k];
        sumP += w[(size_t) k] * (m * m);
    }
    return 10.0f * std::log10(sumP + 1.0e-18f);
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
    float envDb { -120.0f };

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
        envDb = -120.0f;
    }

    void reset() noexcept
    {
        std::fill(inputRing.begin(), inputRing.end(), 0.0f);
        std::fill(frame.begin(), frame.end(), 0.0f);
        std::fill(ola.begin(), ola.end(), 0.0f);
        writePos = 0;
        inputSampleCounter = 0;
        envDb = -120.0f;
    }

    void processFrame(int fftSize,
                      const float* window,
                      juce::dsp::FFT& fft,
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
        std::array<float, 513> weight {};
        const int magCount = juce::jmin(half + 1, (int) magIn.size());
        for (int k = 0; k < magCount; ++k)
        {
            const auto& c = freqDomain[(size_t) k];
            const float re = c.real();
            const float im = c.imag();
            magIn[(size_t) k] = std::sqrt(re * re + im * im);
        }

        buildSidechainWeights(owner.sampleRate_, fftSize, half, owner.sidechainHz_, owner.sidechainQ_, weight.data());

        const float detDb = weightedPowerDb(magIn.data(), weight.data(), half);

        const double hop = (double) (fftSize / 2);
        const double sr = owner.sampleRate_;
        const float frameDt = (float) (hop / juce::jmax(1.0e-6, sr));
        const float attS = juce::jmax(0.0005f, owner.attackMs_ * 0.001f);
        const float relS = juce::jmax(0.0005f, owner.releaseMs_ * 0.001f);
        const float alphaA = 1.0f - std::exp(-frameDt / attS);
        const float alphaR = 1.0f - std::exp(-frameDt / relS);

        if (detDb > envDb)
            envDb += (detDb - envDb) * alphaA;
        else
            envDb += (detDb - envDb) * alphaR;

        const float thr = owner.thresholdDb_;
        const float rat = owner.ratio_;
        const float overDb = juce::jmax(0.0f, envDb - thr);
        const float grDb = overDb * (1.0f - 1.0f / rat);
        const float gainLin = std::pow(10.0f, -grDb * 0.05f);

        if (channelIndex == 0)
        {
            owner.scEnvDbUi_.store(envDb, std::memory_order_relaxed);
            owner.commitSpectralMeterFrame(half, magIn.data(), weight.data(), gainLin, thr, rat);
        }

        for (int k = 0; k <= half; ++k)
        {
            auto& c = freqDomain[(size_t) k];
            const float re = c.real();
            const float im = c.imag();
            const float mag = magIn[(size_t) k];
            const float wk = weight[(size_t) k];
            const float g = 1.0f + wk * (gainLin - 1.0f);
            c = { re * g, im * g };
        }

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
                    SpectralCompressorNode& owner,
                    int channelIndex)
    {
        inputRing[(size_t) writePos] = x;
        writePos = (writePos + 1) % fftSize;
        ++inputSampleCounter;

        if (inputSampleCounter >= (uint64_t) fftSize
            && ((inputSampleCounter - (uint64_t) fftSize) % (uint64_t) hop) == 0)
            processFrame(fftSize, window, fft, owner, channelIndex);
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
                                                      const float* weight,
                                                      float gainLin,
                                                      float thresholdDb,
                                                      float ratio) noexcept
{
    juce::ignoreUnused(thresholdDb, ratio);
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
            const float wk = weight[(size_t) k];
            const float g = 1.0f + wk * (gainLin - 1.0f);
            const float m2 = mag * g;
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

float SpectralCompressorNode::getSpectralSidechainEnvDbForUi() const noexcept
{
    return scEnvDbUi_.load(std::memory_order_relaxed);
}

SpectralCompressorNode::SpectralCompressorNode() = default;

SpectralCompressorNode::~SpectralCompressorNode() = default;

int SpectralCompressorNode::getLatencySamples() const noexcept
{
    return bypass_ ? 0 : kLatencySamples_;
}

void SpectralCompressorNode::prepare(double sampleRate, int maxBlockSize, int numChannels)
{
    sampleRate_ = juce::jmax(1.0, sampleRate);
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
    scEnvDbUi_.store(-120.0f, std::memory_order_relaxed);
}

void SpectralCompressorNode::process(juce::AudioBuffer<float>& buffer)
{
    if (bypass_)
    {
        for (auto& a : specInNorm_)
            a.store(0.f, std::memory_order_relaxed);
        for (auto& a : specRedNorm_)
            a.store(0.f, std::memory_order_relaxed);
        scEnvDbUi_.store(-120.0f, std::memory_order_relaxed);
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

    for (int i = 0; i < n; ++i)
    {
        for (int c = 0; c < ch; ++c)
        {
            const float x = buffer.getSample(c, i);
            channels_[(size_t) c]->pushSample(x, fftSize_, hop_, window_.data(), *fft_, *this, c);
            const float wet = channels_[(size_t) c]->getWetOutput(kLatencySamples_);
            const float dry = dryScratch_.getSample(c, i);
            buffer.setSample(c, i, wet * mix + dry * dryMix);
        }
    }
}

} // namespace razumov::graph
