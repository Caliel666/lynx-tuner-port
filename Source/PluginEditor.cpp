#include "PluginEditor.h"

LynxTunerAudioProcessorEditor::LynxTunerAudioProcessorEditor(LynxTunerAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p), controls(std::make_unique<juce::GenericAudioProcessorEditor>(p)) {
    addAndMakeVisible(*controls);
    setSize(760, 900);
}
void LynxTunerAudioProcessorEditor::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xff17191f));
    g.setColour(juce::Colour(0xfff2f3f5));
    g.setFont(juce::Font(20.0f, juce::Font::bold));
    g.drawText("LYNX TUNER", 16, 8, getWidth()-32, 30, juce::Justification::centredLeft);
}
void LynxTunerAudioProcessorEditor::resized() {
    controls->setBounds(8, 42, getWidth()-16, getHeight()-50);
}
