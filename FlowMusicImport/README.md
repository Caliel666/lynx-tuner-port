# Flow Music UI port (work in progress)

This branch is reserved for porting the Google Flow Music LYNX-style interface into the native JUCE VST3 editor.

## Porting target

The React/Tailwind presentation is to be translated into JUCE components and custom painting. The plugin must not embed a browser or depend on React, WebAssembly, or Flow Music's private `@flowmusic/vst` runtime. JUCE's VST3 wrapper remains responsible for Windows, Linux, and macOS host integration.

The intended UI elements are:
- LYNX LIVE AUTOTUNER header with key, scale, and formant controls
- pitch/correction telemetry display
- chromatic scale quantizer keyboard
- rotary controls for retune speed, correction amount, smoothness, formant shift, reference pitch, mix, and output gain

## Import status

The code pasted into the conversation is not a complete compilable export. The TSX has truncated identifiers, missing braces/handlers, and malformed JSX fragments. The AssemblyScript/WASM file also has broken declarations and missing variable definitions. It cannot safely be copied into the build as-is; the original files or project archive are needed to recover missing code without inventing DSP behavior.

The supplied parameter list is retained as the UI design specification. Its parameter model does not match the existing native LYNX Tune engine's parameter IDs and value laws, so the port must explicitly map each visible control to the native parameter or clearly mark controls that have no native equivalent. Do not silently add a second pitch-correction engine.

## DSP comparison

The supplied WASM sketch is not the same processing implementation as the upstream/native LYNX Tune engine. It sketches a YIN-like difference-function pitch detector, nearest-note scale snapping, and a simple dual-grain delay-line pitch shifter. The existing native engine has its own pitch detector, scale/key targeting, MIDI-held-note behavior, formant/throat processing, vibrato handling, and correction laws. Matching the interface does not make the two DSP implementations equivalent.

## Cross-platform requirements

- Keep the editor and processor in standard C++/JUCE.
- Avoid platform-specific APIs and webview dependencies.
- Build the VST3 target on Windows, Linux, and macOS in GitHub Actions.
- Do not claim cross-platform completion until all three CI jobs pass.
