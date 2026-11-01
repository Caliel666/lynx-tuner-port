#pragma once
#include <JuceHeader.h>
#include "DSP/Effects.h"
#include "DSP/LynxTune.h"
#include <array>
#include <memory>
#include <vector>

class LynxTunerAudioProcessor final : public juce::AudioProcessor {
public:
    LynxTunerAudioProcessor();
    ~LynxTunerAudioProcessor() override = default;
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "LYNX Tuner"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.25; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;
    juce::AudioProcessorValueTreeState& parameters() noexcept { return apvts; }
    const std::vector<float>& parameterValues() const noexcept { return values; }
    const lynx::Effect& engine() const noexcept { return *effect; }
private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState apvts;
    std::vector<std::atomic<float>*> parameterPointers;
    std::vector<float> values;
    std::unique_ptr<lynx::Effect> effect;
    std::array<lynx::NoteMask,16> heldNotes{};
    int lastLatency = -1;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LynxTunerAudioProcessor)
};
