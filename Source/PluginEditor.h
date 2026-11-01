#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include <array>
#include <memory>
#include <vector>

class LynxTunerAudioProcessorEditor final : public juce::AudioProcessorEditor, private juce::Timer {
public:
    explicit LynxTunerAudioProcessorEditor(LynxTunerAudioProcessor&);
    void paint(juce::Graphics&) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent&) override;
    void timerCallback() override;
private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    LynxTunerAudioProcessor& processor;
    juce::Label title;
    juce::ComboBox inputRange, key, scale, midiChannel, voicePreset;
    juce::ToggleButton lowLatency, formant, classic, learnScale, inScale;
    juce::Slider tracking, retune, flexTune, naturalVibrato, humanize, formantShift, throat, transpose, reference, mix;
    std::vector<std::unique_ptr<SliderAttachment>> sliderAttachments;
    std::vector<std::unique_ptr<ButtonAttachment>> buttonAttachments;
    std::vector<std::unique_ptr<ComboAttachment>> comboAttachments;
    std::array<juce::TextButton, 3> pages;
    std::array<juce::ToggleButton, 12> noteKeys;
    int activePage = 0;
    std::array<float, 180> pitchHistory{};
    int pitchHistoryWrite = 0;
    bool removeMode = true;
    juce::String midiNoteName(float normalized) const;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LynxTunerAudioProcessorEditor)
};