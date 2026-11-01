# LYNX Tuner — VST3 port

A standalone VST3 effect port of the LYNX Tune real-time vocal pitch corrector from [LYNX SHARE](https://github.com/Ka1-S/LYNX-SHARE).

## Current implementation

- JUCE 8 VST3 effect target, stereo and mono layouts.
- Ported LYNX Tune DSP engine and YIN / TD-PSOLA vocal pitch toolkit.
- Host-automatable parameters with persistent plugin state.
- MIDI note input for MIDI-target / scale-learning workflows; plugin latency is reported to the host.
- Windows x64 GitHub Actions build.

The editor currently uses JUCE's generic parameter editor as a functional first-pass interface. A purpose-built LYNX Tune interface (keyboard, scale selection, pitch display, meters and grouped controls) is the next UI milestone.

## Build (Windows, Visual Studio 2022)

```powershell
cmake -S . -B build -A x64 -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --target LynxTuner_VST3
```

JUCE 8.0.4 is fetched by CMake. The resulting VST3 bundle is under `build/LynxTuner_artefacts/Release/VST3/`.

## License

The upstream DSP is licensed under AGPL-3.0. See [LICENSE](LICENSE) and [NOTICE.md](NOTICE.md).
