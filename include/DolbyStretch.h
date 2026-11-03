#pragma once
#include <algorithm>
#include <cmath>
#include "VocalCommon.h"

namespace vocal_hifi {
class DolbyStretch {
public:
 void prepare(double sr) noexcept { sr_=sr; reset(); updateCoefficients(0.5f); }
 void reset() noexcept { env_.fill(0.0f); }
 void setEnabled(bool enabled) noexcept { enabled_=enabled; }
 void updateCoefficients(float amount) noexcept {
   amount=clampFinite(amount,0.0f,1.0f,0.5f);
   const float configuredMaxBoost[4]={1.0f+0.7f*amount,1.0f+0.45f*amount,1.0f+0.3f*amount,1.0f+0.2f*amount};
   const float configuredRatio[4]={1.0f+2.0f*amount,1.0f+1.5f*amount,1.0f+amount,1.0f+0.5f*amount};
   for(int b=0;b<4;++b) {
     maxBoost_[b]=configuredMaxBoost[b];
     ratio_[b]=configuredRatio[b];
     atk_[b]=std::exp(-1.0f/static_cast<float>(sr_*0.008));
     rel_[b]=std::exp(-1.0f/static_cast<float>(sr_*0.120));
   }
 }
 void process(float* l,float* r,int n,float amount) noexcept {
   if(!enabled_||!l||!r) return;
   amount=clampFinite(amount,0.0f,1.0f,0.0f);
   for(int i=0;i<n;++i) {
     float* x[2]={&l[i],&r[i]};
     for(int ch=0;ch<2;++ch) {
       const float a=std::abs(*x[ch]);
       const float c=a>env_[ch]?atk_[0]:rel_[0];
       env_[ch]=c*env_[ch]+(1.0f-c)*a;
       const float target=std::min(maxBoost_[0],1.0f+amount*0.18f/(0.02f+env_[ch]));
       const float gain=std::min(target,1.0f+(target-1.0f)/ratio_[0]);
       *x[ch]*=gain;
     }
   }
 }
private:
 double sr_=48000.0;
 bool enabled_=false;
 float maxBoost_[4]{1,1,1,1}, ratio_[4]{1,1,1,1}, atk_[4]{}, rel_[4]{};
 std::array<float,2> env_{};
};
}
