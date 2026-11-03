#pragma once
#include "VocalCommon.h"
#include "ADT.h"
#include "DolbyStretch.h"
#include "vocal_hifi_abi.h"
#include <array>

namespace vocal_hifi {
struct Params {
 std::array<float,VP_PARAM_COUNT> v{};
 Params() noexcept {
  v[VP_PARAM_INPUT_GAIN]=1; v[VP_PARAM_HP_HZ]=80; v[VP_PARAM_MUD_DB]=-1.5f;
  v[VP_PARAM_PRESENCE_DB]=1.0f; v[VP_PARAM_AIR_DB]=1.0f;
  v[VP_PARAM_ARTIFACT_DIP_DB]=-1.0f; v[VP_PARAM_WARMTH]=0.2f;
  v[VP_PARAM_DEESS_AMOUNT]=0.25f; v[VP_PARAM_DOLBY_STRETCH]=0;
  v[VP_PARAM_BREATH_AMOUNT]=0.2f; v[VP_PARAM_COMPRESSOR_AMOUNT]=0.25f;
  v[VP_PARAM_BREATH_LIFT]=0.05f; v[VP_PARAM_LAYER_GAIN]=1;
  v[VP_PARAM_ADT_MIX]=0; v[VP_PARAM_ADT_DELAY_MS]=12;
  v[VP_PARAM_BUS_WARMTH]=0.1f; v[VP_PARAM_OUTPUT_GAIN]=1; v[VP_PARAM_SOFT_CLIP]=0;
 }
};
class VocalProcessor {
public:
 bool prepare(double sr,int maxFrames,int channels,int layers) noexcept;
 vp_status setParameter(int p,float value) noexcept;
 float getParameter(int p) const noexcept;
 void process(float* const* data,int channels,int frames) noexcept;
 void processInterleaved(float* data,int channels,int frames) noexcept;
 uint32_t warnings() const noexcept { return warnings_; }
 void setStatus(vp_status s) noexcept { status_=s; }
 vp_status status() const noexcept { return status_; }
private:
 void syncParams() noexcept;
 void processOne(float& l,float& r) noexcept;
 Params params_;
 double sr_=48000;
 int maxFrames_=0, channels_=2, layers_=1;
 bool prepared_=false;
 std::array<Biquad,2> hp_,mud_,presence_,air_,dip_,dc_;
 std::array<float,2> deessEnv_{};
 ADT adt_;
 DolbyStretch dolby_;
 float smoothedWarmth_=0,smoothedMix_=0,smoothedOutputGain_=1;
 float busEnv_=0;
 uint32_t warnings_=0;
 vp_status status_=VP_OK;
};
}
