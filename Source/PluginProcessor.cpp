#include "PluginProcessor.h"
#include "PluginEditor.h"

juce::AudioProcessorValueTreeState::ParameterLayout LynxTunerAudioProcessor::createParameterLayout() {
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    const auto& info = lynx::lynxtune::info();
    for (int i = 0; i < (int)info.params.size(); ++i) {
        const auto& p = info.params[(size_t)i];
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{juce::String("p") + juce::String(i), 1}, p.name,
            juce::NormalisableRange<float>{0.0f, 1.0f}, p.defaultValue));
    }
    return layout;
}

LynxTunerAudioProcessor::LynxTunerAudioProcessor()
    : juce::AudioProcessor(BusesProperties()
          .withInput("Input", juce::AudioChannelSet::stereo(), true)
          .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "LYNX_TUNER_STATE", createParameterLayout()),
      effect(lynx::lynxtune::make()) {
    const auto count = (int)lynx::lynxtune::info().params.size();
    parameterPointers.reserve((size_t)count);
    values.resize((size_t)count);
    for (int i = 0; i < count; ++i)
        parameterPointers.push_back(apvts.getRawParameterValue(juce::String("p") + juce::String(i)));
}

void LynxTunerAudioProcessor::prepareToPlay(double sampleRate, int) {
    effect->prepare(sampleRate);
    effect->reset();
    heldNotes = {};
    lastLatency = -1;
}
void LynxTunerAudioProcessor::releaseResources() { effect->reset(); heldNotes = {}; }

bool LynxTunerAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
    const auto in = layouts.getMainInputChannelSet();
    const auto out = layouts.getMainOutputChannelSet();
    return (in == juce::AudioChannelSet::mono() || in == juce::AudioChannelSet::stereo()) && in == out;
}

void LynxTunerAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) {
    juce::ScopedNoDenormals noDenormals;
    const int channels = getTotalNumInputChannels();
    const int samples = buffer.getNumSamples();
    for (int ch = getTotalNumInputChannels(); ch < getTotalNumOutputChannels(); ++ch)
        buffer.clear(ch, 0, samples);

    for (const auto metadata : midi) {
        const auto msg = metadata.getMessage();
        if (msg.isNoteOn() || msg.isNoteOff()) {
            const int channel = juce::jlimit(0, 15, msg.getChannel() - 1);
            const int note = juce::jlimit(0, 127, msg.getNoteNumber());
            auto& word = heldNotes[(size_t)channel][(size_t)(note >> 6)];
            const auto bit = (std::uint64_t{1} << (note & 63));
            if (msg.isNoteOn()) word |= bit; else word &= ~bit;
        }
    }
    effect->setHeldNotes(heldNotes.data(), (int)heldNotes.size());
    for (size_t i = 0; i < values.size(); ++i)
        values[i] = parameterPointers[i]->load(std::memory_order_relaxed);

    if (channels > 0) {
        auto* left = buffer.getWritePointer(0);
        auto* right = channels > 1 ? buffer.getWritePointer(1) : left;
        effect->process(left, right, samples, values, 120.0);
    }
    const int latency = effect->latencySamples();
    if (latency != lastLatency) { setLatencySamples(latency); lastLatency = latency; }
}

juce::AudioProcessorEditor* LynxTunerAudioProcessor::createEditor() { return new LynxTunerAudioProcessorEditor(*this); }
void LynxTunerAudioProcessor::getStateInformation(juce::MemoryBlock& dest) {
    if (auto xml = apvts.copyState().createXml()) copyXmlToBinary(*xml, dest);
}
void LynxTunerAudioProcessor::setStateInformation(const void* data, int size) {
    if (auto xml = getXmlFromBinary(data, size); xml && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new LynxTunerAudioProcessor(); }
