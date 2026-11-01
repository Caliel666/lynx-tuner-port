#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include <memory>
#include <vector>

class LynxTunerAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                            private juce::Timer
{
public:
    explicit LynxTunerAudioProcessorEditor (LynxTunerAudioProcessor&);
    ~LynxTunerAudioProcessorEditor() override = default;
    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    LynxTunerAudioProcessor& processor;
    juce::ComboBox key, scale;
    juce::ToggleButton formantPreserve;
    juce::Slider speed, smoothness, correction, formantShift, reference, mix, outputGain;
    juce::Label header, subtitle, status, targetLabel, centsLabel, inputLabel, correctedLabel, depthLabel;
    juce::Label keyLabel, scaleLabel;
    std::vector<std::unique_ptr<SliderAttachment>> sliderAttachments;
    std::vector<std::unique_ptr<ButtonAttachment>> buttonAttachments;
    std::vector<std::unique_ptr<ComboAttachment>> comboAttachments;

    void addKnob (juce::Slider&, const juce::String&, int);
    void drawKnobGroup (juce::Graphics&, juce::Rectangle<int>, const juce::String&);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LynxTunerAudioProcessorEditor)
};
