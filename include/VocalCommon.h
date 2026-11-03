#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>

namespace vocal_hifi {
constexpr float kPi = 3.14159265358979323846f;
constexpr int kMaxChannels = 2;
constexpr int kMaxLayers = 8;
inline bool finite(float x) noexcept { return std::isfinite(x); }
inline float clampFinite(float x, float lo, float hi, float fallback=0.0f) noexcept {
    return finite(x) ? std::max(lo, std::min(hi, x)) : fallback;
}
struct StereoFrame { float l=0.0f, r=0.0f; };
struct Biquad {
    float b0=1, b1=0, b2=0, a1=0, a2=0, z1=0, z2=0;
    void reset() noexcept { z1=z2=0; }
    bool set(float nb0,float nb1,float nb2,float na0,float na1,float na2) noexcept {
        if (!finite(nb0)||!finite(nb1)||!finite(nb2)||!finite(na0)||!finite(na1)||!finite(na2)||std::abs(na0)<1e-20f) return false;
        const float inv=1.0f/na0;
        b0=nb0*inv; b1=nb1*inv; b2=nb2*inv; a1=na1*inv; a2=na2*inv;
        return finite(b0)&&finite(b1)&&finite(b2)&&finite(a1)&&finite(a2);
    }
    float process(float x) noexcept {
        const float y=b0*x+z1;
        z1=b1*x-a1*y+z2; z2=b2*x-a2*y;
        if (!finite(y)||!finite(z1)||!finite(z2)) { reset(); return 0.0f; }
        return y;
    }
    bool lowpass(float sr,float hz,float q) noexcept {
        hz=clampFinite(hz,10.0f,0.45f*sr,1000.0f); q=clampFinite(q,0.1f,20.0f,0.707f);
        const float w=2*kPi*hz/sr, c=std::cos(w), s=std::sin(w), a=s/(2*q);
        return set((1-c)*0.5f,1-c,(1-c)*0.5f,1+a,-2*c,1-a);
    }
    bool highpass(float sr,float hz,float q) noexcept {
        hz=clampFinite(hz,10.0f,0.45f*sr,80.0f); q=clampFinite(q,0.1f,20.0f,0.707f);
        const float w=2*kPi*hz/sr, c=std::cos(w), s=std::sin(w), a=s/(2*q);
        return set((1+c)*0.5f,-(1+c),(1+c)*0.5f,1+a,-2*c,1-a);
    }
    bool peaking(float sr,float hz,float q,float db) noexcept {
        hz=clampFinite(hz,10.0f,0.45f*sr,1000.0f); q=clampFinite(q,0.1f,20.0f,1.0f); db=clampFinite(db,-18,18,0);
        const float A=std::pow(10.0f,db/40.0f), w=2*kPi*hz/sr, c=std::cos(w), a=std::sin(w)/(2*q);
        return set(1+a*A,-2*c,1-a*A,1+a/A,-2*c,1-a/A);
    }
};
}
