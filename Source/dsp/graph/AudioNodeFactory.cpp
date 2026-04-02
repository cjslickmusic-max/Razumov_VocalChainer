#include "AudioNodeFactory.h"

#include "CompressorArchetypeNode.h"
#include "DeesserNode.h"
#include "ExciterNode.h"
#include "FilterNode.h"
#include "GainNode.h"
#include "LatencyNode.h"
#include "MicCorrectionNode.h"
#include "ParametricEqNode.h"
#include "SpectralCompressorNode.h"
#include "SpectrumAnalyzerNode.h"

namespace razumov::graph
{

std::unique_ptr<AudioNode> createAudioNodeFromModuleDesc(const FlexSlotDesc& d)
{
    jassert(d.descType == FlexSlotDescType::Module);

    switch (d.kind)
    {
        case AudioNodeKind::MicCorrection:
            return std::make_unique<MicCorrectionNode>();
        case AudioNodeKind::Gain:
            return std::make_unique<GainNode>(d.gainLinear);
        case AudioNodeKind::Filter:
        {
            auto f = std::make_unique<FilterNode>();
            f->setCutoffHz(d.filterCutoffHz);
            return f;
        }
        case AudioNodeKind::Deesser:
            return std::make_unique<DeesserNode>();
        case AudioNodeKind::OptoCompressor:
            return std::make_unique<CompressorArchetypeNode>(CompressorArchetypeNode::Archetype::Opto);
        case AudioNodeKind::FetCompressor:
            return std::make_unique<CompressorArchetypeNode>(CompressorArchetypeNode::Archetype::Fet);
        case AudioNodeKind::VcaCompressor:
            return std::make_unique<CompressorArchetypeNode>(CompressorArchetypeNode::Archetype::Vca);
        case AudioNodeKind::Exciter:
            return std::make_unique<ExciterNode>();
        case AudioNodeKind::SpectralCompressor:
            return std::make_unique<SpectralCompressorNode>();
        case AudioNodeKind::ParametricEq:
            return std::make_unique<ParametricEqNode>();
        case AudioNodeKind::SpectrumAnalyzer:
            return std::make_unique<SpectrumAnalyzerNode>();
        case AudioNodeKind::Latency:
            return std::make_unique<LatencyNode>(juce::jmax(0, d.latencySamples));
        default:
            return std::make_unique<GainNode>(1.0f);
    }
}

} // namespace razumov::graph
