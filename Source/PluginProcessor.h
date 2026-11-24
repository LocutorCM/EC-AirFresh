#pragma once

#include <JuceHeader.h>

class EC_AIRAudioProcessor : public juce::AudioProcessor
{
public:
    EC_AIRAudioProcessor();
    ~EC_AIRAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // Esto es lo que usa el Editor para conectar los knobs
    juce::AudioProcessorValueTreeState apvts;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    // Parámetros en crudo para el DSP
    std::atomic<float>* midAirParam = nullptr;
    std::atomic<float>* highAirParam = nullptr;
    std::atomic<float>* mixParam = nullptr;
    std::atomic<float>* outputParam = nullptr;
    std::atomic<float>* powerParam = nullptr;

    // Filtros para el exciter
    juce::dsp::IIR::Filter<float> midFilterL, midFilterR;
    juce::dsp::IIR::Filter<float> highFilterL, highFilterR;
    juce::dsp::IIR::Coefficients<float>::Ptr midCoeffs;
    juce::dsp::IIR::Coefficients<float>::Ptr highCoeffs;

    double currentSampleRate = 48000.0;

    float processExciter(float input, juce::dsp::IIR::Filter<float>& midF,
                         juce::dsp::IIR::Filter<float>& highF,
                         float midAmount, float highAmount);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EC_AIRAudioProcessor)
};
