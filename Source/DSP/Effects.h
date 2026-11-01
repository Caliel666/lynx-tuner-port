#pragma once
#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace lynx {
using EffectParams = std::vector<float>;
using NoteMask = std::array<std::uint64_t, 2>;
struct EffectParamInfo { const char* name; float defaultValue; };
struct EffectInfo { const char* name; std::vector<EffectParamInfo> params; };
class Effect {
public:
    virtual ~Effect() = default;
    virtual void prepare(double sampleRate) noexcept = 0;
    virtual void reset() noexcept = 0;
    virtual void process(float* left, float* right, int samples, const EffectParams& params, double bpm) noexcept = 0;
    virtual void setHeldNotes(const NoteMask*, int) noexcept {}
    virtual float controlOutput(int) const noexcept { return 0.0f; }
    virtual int latencySamples() const noexcept { return 0; }
};
}
