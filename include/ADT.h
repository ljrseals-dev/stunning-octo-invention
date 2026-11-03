#pragma once
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstddef>

namespace vocal_hifi {
class ADT {
public:
 bool prepare(double sr, int maxFrames) noexcept {
   if (!(sr>=44100.0 && sr<=192000.0) || maxFrames<=0) return false;
   try {
     const std::size_t maxDelay=static_cast<std::size_t>(sr*0.060)+maxFrames+4;
     left_.assign(maxDelay,0.0f); right_.assign(maxDelay,0.0f);
   } catch (...) { left_.clear(); right_.clear(); return false; }
   sr_=sr; write_=0; return true;
 }
 void reset() noexcept { std::fill(left_.begin(),left_.end(),0); std::fill(right_.begin(),right_.end(),0); write_=0; }
 void set(float mix,float delayMs) noexcept {
   mix_=std::max(0.0f,std::min(0.5f,mix));
   delay_=std::max(1.0f,std::min(50.0f,delayMs))*0.001*sr_;
 }
 void process(float* l,float* r,int n) noexcept {
   if (left_.empty()||right_.empty()||!l||!r) return;
   const std::size_t size=left_.size();
   const std::size_t d=std::min(size-1,static_cast<std::size_t>(delay_));
   for(int i=0;i<n;++i) {
     const std::size_t read=(write_+size-d)%size;
     const float dl=left_[read], dr=right_[read];
     const float inL=l[i], inR=r[i];
     left_[write_]=inL; right_[write_]=inR;
     l[i]=inL*(1.0f-mix_)+dr*mix_;
     r[i]=inR*(1.0f-mix_)+dl*mix_;
     write_=(write_+1)%size;
   }
 }
private:
 std::vector<float> left_,right_;
 double sr_=48000.0, delay_=12.0*48.0;
 std::size_t write_=0;
 float mix_=0.0f;
};
}
