#include "vocal_hifi_abi.h"
#include <cassert>
#include <cmath>
#include <cstdio>
#include <limits>
int main() {
 vp_handle* h=vp_create(); assert(h);
 assert(vp_prepare(h,48000,512,2,1)==VP_OK);
 assert(vp_set_param(h,VP_PARAM_DOLBY_STRETCH,1.0f)==VP_OK);
 assert(vp_set_param(h,VP_PARAM_BREATH_AMOUNT,0.2f)==VP_OK);
 assert(vp_last_warnings(h)==0); // warning is refreshed at processing boundary
 float l[512],r[512]; for(int i=0;i<512;++i){l[i]=r[i]=0.1f*std::sin(0.03f*i);}
 float* p[2]={l,r}; assert(vp_process_planar(h,p,2,512)==VP_OK);
 assert(vp_last_warnings(h)&VP_WARN_BREATH_CLAMPED);
 for(int i=0;i<512;++i) assert(std::isfinite(l[i])&&std::isfinite(r[i]));
 assert(vp_set_param(h,VP_PARAM_WARMTH,std::numeric_limits<float>::quiet_NaN())==VP_ERROR_RANGE);
 assert(vp_load_preset(h,"{\"warmth\":0.4,\"artifact_dip_db\":-2.0}")==VP_OK);
 assert(vp_load_preset(h,"{\"warmth\":NaN}")==VP_ERROR_PARSE);
 assert(vp_load_preset(h,"{\"warmth\":0.4} trailing")==VP_ERROR_PARSE);
 assert(vp_latency_samples(h)==0);
 vp_destroy(h);
 std::puts("vocal_hifi tests passed");
}
