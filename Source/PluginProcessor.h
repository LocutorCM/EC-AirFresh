#pragma once
#include <JuceHeader.h>

class ECAirProcessor : public juce::AudioProcessor {
public:
    ECAirProcessor();
    ~ECAirProcessor() override;
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "EC AIR EXCITER"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;
    juce::AudioProcessorValueTreeState apvts;
    float inLevel = 0.0f;
    float outLevel = 0.0f;
    float airLevel = 0.0f;
private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParams();
    juce::dsp::IIR::Filter<float> midHighPass[2];
    juce::dsp::IIR::Filter<float> highHighPass[2];
    juce::dsp::IIR::Filter<float> midShelf[2];
    juce::dsp::IIR::Filter<float> highShelf[2];
    float excite(float x, float amount);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ECAirProcessor)
};
