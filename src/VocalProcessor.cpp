#include "VocalProcessor.h"
#include <cmath>

namespace vocal_hifi {
bool VocalProcessor::prepare(double sr,int maxFrames,int channels,int layers) noexcept {
 if (!(sr>=44100.0&&sr<=192000.0)||maxFrames<=0||(channels!=1&&channels!=2)||layers<1||layers>kMaxLayers) return false;
 sr_=sr; maxFrames_=maxFrames; channels_=channels; layers_=layers;
 for(int c=0;c<2;++c) {
  hp_[c].highpass(static_cast<float>(sr),80,0.707f);
  mud_[c].peaking(static_cast<float>(sr),300,0.8f,-1.5f);
  presence_[c].peaking(static_cast<float>(sr),3000,0.9f,1);
  air_[c].peaking(static_cast<float>(sr),10000,0.7f,1);
  dip_[c].peaking(static_cast<float>(sr),2600,4.0f,-1);
  dc_[c].highpass(static_cast<float>(sr),20,0.707f);
 }
 if(!adt_.prepare(sr,maxFrames)) return false;
 dolby_.prepare(sr); prepared_=true; syncParams(); return true;
}
vp_status VocalProcessor::setParameter(int p,float x) noexcept {
 if(p<0||p>=VP_PARAM_COUNT) return status_=VP_ERROR_RANGE;
 if(!finite(x)) return status_=VP_ERROR_RANGE;
 static const float lo[VP_PARAM_COUNT]={0,20,-18,-18,-18,-18,0,0,0,0,0,0,0,0,1,0,0,0};
 static const float hi[VP_PARAM_COUNT]={4,300,18,18,18,18,1,1,1,1,1,0.5f,4,0.5f,50,1,4,1};
 if(x<lo[p]||x>hi[p]) return status_=VP_ERROR_RANGE;
 params_.v[p]=x; status_=VP_OK; return VP_OK;
}
float VocalProcessor::getParameter(int p) const noexcept { return (p>=0&&p<VP_PARAM_COUNT)?params_.v[p]:0.0f; }
void VocalProcessor::syncParams() noexcept {
 const auto& v=params_.v;
 const float hp=clampFinite(v[VP_PARAM_HP_HZ],20,300,80);
 const float warmth=clampFinite(v[VP_PARAM_WARMTH],0,1,0);
 smoothedWarmth_ += 0.05f*(warmth-smoothedWarmth_);
 smoothedMix_ += 0.05f*(v[VP_PARAM_ADT_MIX]-smoothedMix_);
 smoothedOutputGain_ += 0.05f*(v[VP_PARAM_OUTPUT_GAIN]-smoothedOutputGain_);
 for(int c=0;c<2;++c) {
  hp_[c].highpass(static_cast<float>(sr_),hp,0.707f);
  mud_[c].peaking(static_cast<float>(sr_),300,0.8f,v[VP_PARAM_MUD_DB]);
  presence_[c].peaking(static_cast<float>(sr_),3000,0.9f,v[VP_PARAM_PRESENCE_DB]);
  air_[c].peaking(static_cast<float>(sr_),10000,0.7f,v[VP_PARAM_AIR_DB]);
  dip_[c].peaking(static_cast<float>(sr_),2600,4.0f,v[VP_PARAM_ARTIFACT_DIP_DB]);
 }
 dolby_.setEnabled(v[VP_PARAM_DOLBY_STRETCH]>0.5f);
 dolby_.updateCoefficients(v[VP_PARAM_DOLBY_STRETCH]);
 adt_.set(smoothedMix_,v[VP_PARAM_ADT_DELAY_MS]);
 warnings_=0;
 if(v[VP_PARAM_DOLBY_STRETCH]>0.5f && v[VP_PARAM_BREATH_AMOUNT]>0.05f) warnings_|=VP_WARN_BREATH_CLAMPED;
}
void VocalProcessor::processOne(float& l,float& r) noexcept {
 float* x[2]={&l,&r};
 for(int c=0;c<2;++c) {
  float y=x[c][0]*params_.v[VP_PARAM_INPUT_GAIN];
  y=hp_[c].process(y); y=mud_[c].process(y); y=presence_[c].process(y);
  y=air_[c].process(y); y=dip_[c].process(y);
  y*=1.0f+smoothedWarmth_*0.12f*std::tanh(y*2.0f);
  y=dc_[c].process(y);
  const float env=std::abs(y);
  const float coeff=env>deessEnv_[c]?0.02f:0.002f;
  deessEnv_[c]+=coeff*(env-deessEnv_[c]);
  if(env>0.25f) y*=1.0f-params_.v[VP_PARAM_DEESS_AMOUNT]*0.25f;
  x[c][0]=y;
 }
}
void VocalProcessor::process(float* const* data,int channels,int frames) noexcept {
 if(!prepared_||!data||frames<0||frames>maxFrames_||channels<1||channels>2) return;
 if(frames==0) return;
 syncParams();
 if(channels==1) {
  for(int i=0;i<frames;++i) { float l=data[0][i],r=l; processOne(l,r); data[0][i]=0.5f*(l+r); }
 } else {
  for(int i=0;i<frames;++i) processOne(data[0][i],data[1][i]);
  dolby_.process(data[0],data[1],frames,params_.v[VP_PARAM_DOLBY_STRETCH]);
  adt_.process(data[0],data[1],frames);
 }
 for(int i=0;i<frames;++i) {
  float l=data[0][i],r=channels>1?data[1][i]:l;
  const float peak=std::max(std::abs(l),std::abs(r));
  busEnv_=0.995f*busEnv_+0.005f*peak;
  const float comp=1.0f/(1.0f+params_.v[VP_PARAM_COMPRESSOR_AMOUNT]*std::max(0.0f,busEnv_-0.18f)*2.0f);
  l*=comp*smoothedOutputGain_; r*=comp*smoothedOutputGain_;
  const float warmth=params_.v[VP_PARAM_BUS_WARMTH];
  l*=1.0f+warmth*0.04f*std::tanh(l*3); r*=1.0f+warmth*0.04f*std::tanh(r*3);
  if(params_.v[VP_PARAM_SOFT_CLIP]>0.5f) { l=0.9f*std::tanh(l/0.9f); r=0.9f*std::tanh(r/0.9f); }
  data[0][i]=l; if(channels>1) data[1][i]=r;
 }
}
void VocalProcessor::processInterleaved(float* d,int channels,int frames) noexcept {
 if(!d||channels<1||channels>2||frames<0||frames>maxFrames_||!prepared_) return;
 // No temporary allocation: process in place using scalar mono/stereo state path.
 syncParams();
 for(int i=0;i<frames;++i) {
  float l=d[static_cast<std::size_t>(i)*channels],r=channels>1?d[static_cast<std::size_t>(i)*channels+1]:l;
  processOne(l,r);
  if(channels>1) { float* lp=&l; float* rp=&r; dolby_.process(lp,rp,1,params_.v[VP_PARAM_DOLBY_STRETCH]); }
  const float peak=std::max(std::abs(l),std::abs(r)); busEnv_=0.995f*busEnv_+0.005f*peak;
  const float comp=1.0f/(1.0f+params_.v[VP_PARAM_COMPRESSOR_AMOUNT]*std::max(0.0f,busEnv_-0.18f)*2.0f);
  l*=comp*smoothedOutputGain_; r*=comp*smoothedOutputGain_;
  if(params_.v[VP_PARAM_SOFT_CLIP]>0.5f) { l=0.9f*std::tanh(l/0.9f); r=0.9f*std::tanh(r/0.9f); }
  d[static_cast<std::size_t>(i)*channels]=channels==1?0.5f*(l+r):l;
  if(channels>1) d[static_cast<std::size_t>(i)*channels+1]=r;
 }
}
}
