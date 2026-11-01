#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class LynxTunerAudioProcessorEditor final : public juce::AudioProcessorEditor {
public:
    explicit LynxTunerAudioProcessorEditor(LynxTunerAudioProcessor&);
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    LynxTunerAudioProcessor& processor;
    std::unique_ptr<juce::GenericAudioProcessorEditor> controls;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LynxTunerAudioProcessorEditor)
};
