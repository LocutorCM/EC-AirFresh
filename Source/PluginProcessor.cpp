#include "PluginProcessor.h"
#include "PluginEditor.h"

EC_AIRAudioProcessor::EC_AIRAudioProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    midAirParam  = apvts.getRawParameterValue("midAir");
    highAirParam = apvts.getRawParameterValue("highAir");
    mixParam     = apvts.getRawParameterValue("mix");
    outputParam  = apvts.getRawParameterValue("output");
    powerParam   = apvts.getRawParameterValue("power");
}

EC_AIRAudioProcessor::~EC_AIRAudioProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout EC_AIRAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    params.push_back(std::make_unique<juce::AudioParameterFloat>("midAir", "Mid Air", 0.0f, 12.0f, 2.4f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("highAir", "High Air", 0.0f, 12.0f, 4.1f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("mix", "Mix", 0.0f, 100.0f, 65.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("output", "Output", -12.0f, 12.0f, -0.5f));
    params.push_back(std::make_unique<juce::AudioParameterBool>("power", "Power", true));
    return { params.begin(), params.end() };
}

void EC_AIRAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = (juce::uint32) samplesPerBlock;
    spec.numChannels = 2;
    for (int ch = 0; ch < 2; ++ch)
    {
        midHighPass[ch].prepare(spec);
        highHighPass[ch].prepare(spec);
        midShelf[ch].prepare(spec);
        highShelf[ch].prepare(spec);
        midHighPass[ch].reset();
        highHighPass[ch].reset();
        midShelf[ch].reset();
        highShelf[ch].reset();
        *midHighPass[ch].coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 6000.0);
        *highHighPass[ch].coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 12000.0);
        *midShelf[ch].coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate, 6000.0, 0.7f, 1.0f);
        *highShelf[ch].coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate, 12000.0, 0.7f, 1.0f);
    }
    // Fuerza a que updateFilters calcule la primera vez
    lastMidAir = -1.0f;
    lastHighAir = -1.0f;
    updateFilters(sampleRate);
}

// CORREGIDO: Ahora solo se recalcula cuando mueves Mid Air o High Air
// Antes lo hacias en cada processBlock y eso tronaba en Monterey
void EC_AIRAudioProcessor::updateFilters(double sampleRate)
{
    float midAmount = midAirParam != nullptr ? midAirParam->load() : 0.0f;
    float highAmount = highAirParam != nullptr ? highAirParam->load() : 0.0f;
    if (std::abs(midAmount - lastMidAir) < 0.001f && std::abs(highAmount - lastHighAir) < 0.001f)
        return;
    lastMidAir = midAmount;
    lastHighAir = highAmount;
    for (int ch = 0; ch < 2; ++ch)
    {
        *midShelf[ch].coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate, 6000.0, 0.7f, juce::Decibels::decibelsToGain(midAmount * 0.5f));
        *highShelf[ch].coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate, 12000.0, 0.7f, juce::Decibels::decibelsToGain(highAmount * 0.5f));
    }
}

void EC_AIRAudioProcessor::releaseResources() {}

bool EC_AIRAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
    return true;
}

float EC_AIRAudioProcessor::excite(float x, float amount)
{
    float driven = x * (1.0f + amount * 0.5f);
    return std::tanh(driven);
}

void EC_AIRAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    if (powerParam == nullptr || powerParam->load() < 0.5f)
        return;

    float midAmount = midAirParam != nullptr ? midAirParam->load() : 0.0f;
    float highAmount = highAirParam != nullptr ? highAirParam->load() : 0.0f;
    float mix = mixParam != nullptr ? (mixParam->load() / 100.0f) : 0.65f;
    float outGain = outputParam != nullptr ? juce::Decibels::decibelsToGain(outputParam->load()) : 1.0f;

    // CORREGIDO: Solo actualiza si cambio el knob, no en cada buffer
    updateFilters(getSampleRate());

    for (int ch = 0; ch < totalNumInputChannels; ++ch)
    {
        auto* data = buffer.getWritePointer(ch);
        int chIdx = juce::jmin(ch, 1);
        auto& midHP = midHighPass[chIdx];
        auto& highHP = highHighPass[chIdx];
        auto& midSh = midShelf[chIdx];
        auto& highSh = highShelf[chIdx];
        for (int s = 0; s < buffer.getNumSamples(); ++s)
        {
            float dry = data[s];
            // CORREGIDO: Primero HighPass, luego Shelf sobre esa banda, no sobre el seco
            float midBand = midSh.processSample(midHP.processSample(dry));
            float highBand = highSh.processSample(highHP.processSample(dry));
            float excited = excite(midBand, midAmount * 0.15f) + excite(highBand, highAmount * 0.15f);
            // La banda de aire es lo que agregan los Shelf por encima del seco
            float air = (midBand + highBand) * 0.2f;
            float wet = dry + excited * 0.3f + air;
            float mixed = dry * (1.0f - mix) + wet * mix;
            data[s] = juce::jlimit(-1.0f, 1.0f, mixed * outGain);
        }
    }
}

juce::AudioProcessorEditor* EC_AIRAudioProcessor::createEditor()
{
    return new EC_AIRAudioProcessorEditor(*this);
}

bool EC_AIRAudioProcessor::hasEditor() const { return true; }
const juce::String EC_AIRAudioProcessor::getName() const { return JucePlugin_Name; }
bool EC_AIRAudioProcessor::acceptsMidi() const { return false; }
bool EC_AIRAudioProcessor::producesMidi() const { return false; }
bool EC_AIRAudioProcessor::isMidiEffect() const { return false; }
double EC_AIRAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int EC_AIRAudioProcessor::getNumPrograms() { return 1; }
int EC_AIRAudioProcessor::getCurrentProgram() { return 0; }
void EC_AIRAudioProcessor::setCurrentProgram(int) {}
const juce::String EC_AIRAudioProcessor::getProgramName(int) { return {}; }
void EC_AIRAudioProcessor::changeProgramName(int, const juce::String&) {}

void EC_AIRAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void EC_AIRAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new EC_AIRAudioProcessor();
}
