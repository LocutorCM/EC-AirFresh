#include "PluginProcessor.h"
#include "PluginEditor.h"

ECAirProcessor::ECAirProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true).withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Params", createParams()) {}

ECAirProcessor::~ECAirProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout ECAirProcessor::createParams() {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    params.push_back(std::make_unique<juce::AudioParameterFloat>("MID_AIR", "Mid Air", 0.0f, 100.0f, 45.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("HIGH_AIR", "High Air", 0.0f, 100.0f, 68.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("MIX", "Mix", 0.0f, 100.0f, 65.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("OUTPUT", "Output", -24.0f, 12.0f, -0.5f));
    params.push_back(std::make_unique<juce::AudioParameterBool>("POWER", "Power", true));
    params.push_back(std::make_unique<juce::AudioParameterBool>("BYPASS", "Bypass", false));
    params.push_back(std::make_unique<juce::AudioParameterChoice>("PRESET", "Preset", juce::StringArray{"Warm & Airy","Vocal Air","Master Shine","Drum Air","Solo High Air"}, 0));
    return {params.begin(), params.end()};
}

void ECAirProcessor::prepareToPlay(double sr, int samples) {
    juce::ignoreUnused(samples);
    juce::dsp::ProcessSpec spec{sr, (juce::uint32)samples, 2};
    for (int ch = 0; ch < 2; ++ch) {
        *midHighPass[ch].coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighPass(sr, 2500.0f, 0.7f);
        *highHighPass[ch].coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighPass(sr, 9500.0f, 0.7f);
        *midShelf[ch].coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowPass(sr, 14000.0f, 0.7f);
        *highShelf[ch].coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowPass(sr, 19000.0f, 0.7f);
        midHighPass[ch].prepare(spec); highHighPass[ch].prepare(spec);
        midShelf[ch].prepare(spec); highShelf[ch].prepare(spec);
    }
}
void ECAirProcessor::releaseResources() {}

float ECAirProcessor::excite(float x, float amount) {
    float driven = x * (1.0f + amount * 1.5f);
    float harmonics = std::tanh(driven * 1.2f) - std::tanh(driven * 0.4f) * 0.3f;
    return harmonics * 0.6f;
}

void ECAirProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) {
    bool power = apvts.getRawParameterValue("POWER")->load() > 0.5f;
    bool bypass = apvts.getRawParameterValue("BYPASS")->load() > 0.5f;
    if (!power || bypass) return;
    auto midAmt = apvts.getRawParameterValue("MID_AIR")->load() / 100.0f;
    auto highAmt = apvts.getRawParameterValue("HIGH_AIR")->load() / 100.0f;
    auto mix = apvts.getRawParameterValue("MIX")->load() / 100.0f;
    auto output = juce::Decibels::decibelsToGain(apvts.getRawParameterValue("OUTPUT")->load());
    inLevel = buffer.getMagnitude(0, 0, buffer.getNumSamples());
    float airSum = 0.0f;
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
        auto* data = buffer.getWritePointer(ch);
        int fch = juce::jmin(ch, 1);
        for (int i = 0; i < buffer.getNumSamples(); ++i) {
            float dry = data[i];
            float midBand = midHighPass[fch].processSample(dry);
            midBand = midShelf[fch].processSample(midBand);
            float midExcited = excite(midBand, midAmt) * midAmt * 1.8f;
            float highBand = highHighPass[fch].processSample(dry);
            highBand = highShelf[fch].processSample(highBand);
            float highExcited = excite(highBand, highAmt) * highAmt * 2.2f;
            float air = midExcited + highExcited;
            airSum += std::abs(air);
            float wet = dry + air;
            float out = dry * (1.0f - mix) + wet * mix;
            out = std::tanh(out * 0.98f);
            data[i] = out * output;
        }
    }
    outLevel = buffer.getMagnitude(0, 0, buffer.getNumSamples());
    airLevel = airSum / (buffer.getNumSamples() * buffer.getNumChannels() + 1.0f);
}

juce::AudioProcessorEditor* ECAirProcessor::createEditor() { return new ECAirEditor(*this); }
void ECAirProcessor::getStateInformation(juce::MemoryBlock& dest) { auto state = apvts.copyState(); std::unique_ptr<juce::XmlElement> xml(state.createXml()); copyXmlToBinary(*xml, dest); }
void ECAirProcessor::setStateInformation(const void* data, int size) { std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, size)); if (xml) apvts.replaceState(juce::ValueTree::fromXml(*xml)); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new ECAirProcessor(); }
