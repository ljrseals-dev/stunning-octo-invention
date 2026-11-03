#include "VocalProcessor.h"
#include "vocal_hifi_abi.h"
#include <new>
#include <cstdlib>
#include <cctype>
#include <cerrno>
#include <cstring>
#include <cmath>

struct vp_handle { vocal_hifi::VocalProcessor processor; };
extern "C" {
vp_handle* vp_create(void) { return new(std::nothrow) vp_handle{}; }
void vp_destroy(vp_handle* h) { delete h; }
vp_status vp_prepare(vp_handle* h,double sr,int maxFrames,int channels,int layers) {
 if(!h) return VP_ERROR_NULL;
 return h->processor.prepare(sr,maxFrames,channels,layers)?VP_OK:VP_ERROR_RANGE;
}
vp_status vp_set_param(vp_handle* h,int p,float v) {
 if(!h) return VP_ERROR_NULL; return h->processor.setParameter(p,v);
}
vp_status vp_get_param(const vp_handle* h,int p,float* v) {
 if(!h||!v) return VP_ERROR_NULL;
 if(p<0||p>=VP_PARAM_COUNT) return VP_ERROR_RANGE;
 *v=h->processor.getParameter(p); return VP_OK;
}
static const char* skipws(const char* p) { while(*p&&std::isspace(static_cast<unsigned char>(*p))) ++p; return p; }
vp_status vp_load_preset(vp_handle* h,const char* json) {
 if(!h||!json) return VP_ERROR_NULL;
 // Strict flat JSON object: string keys, finite numeric values, no duplicate keys/trailing data.
 float staged[VP_PARAM_COUNT]; bool seen[VP_PARAM_COUNT]{};
 for(int i=0;i<VP_PARAM_COUNT;++i) staged[i]=h->processor.getParameter(i);
 const char* p=skipws(json); if(*p++!='{') return VP_ERROR_PARSE;
 p=skipws(p);
 if(*p=='}') { ++p; return *skipws(p)=='\0'?VP_OK:VP_ERROR_PARSE; }
 for(;;) {
  p=skipws(p); if(*p++!='"') return VP_ERROR_PARSE;
  char key[64]; std::size_t n=0;
  while(*p&&*p!='"') { if(*p=='\\'||static_cast<unsigned char>(*p)<0x20||n+1>=sizeof(key)) return VP_ERROR_PARSE; key[n++]=*p++; }
  if(*p++!='"') return VP_ERROR_PARSE; key[n]='\0';
  int id=-1;
  for(int i=0;i<VP_PARAM_COUNT;++i) {
   static const char* names[VP_PARAM_COUNT]={"input_gain","hp_hz","mud_db","presence_db","air_db","artifact_dip_db","warmth","deess_amount","dolby_stretch","breath_amount","compressor_amount","breath_lift","layer_gain","adt_mix","adt_delay_ms","bus_warmth","output_gain","soft_clip"};
   if(std::strcmp(key,names[i])==0) { id=i; break; }
  }
  if(id<0||seen[id]) return VP_ERROR_PARSE;
  p=skipws(p); if(*p++!=':') return VP_ERROR_PARSE; p=skipws(p);
  char* end=nullptr; errno=0; const float val=std::strtof(p,&end);
  if(end==p||errno==ERANGE||!std::isfinite(val)) return VP_ERROR_PARSE;
  // Validate against the same public parameter contract before any mutation.
  static const float lo[VP_PARAM_COUNT]={0,20,-18,-18,-18,-18,0,0,0,0,0,0,0,0,1,0,0,0};
  static const float hi[VP_PARAM_COUNT]={4,300,18,18,18,18,1,1,1,1,1,0.5f,4,0.5f,50,1,4,1};
  if(val<lo[id]||val>hi[id]) return VP_ERROR_RANGE;
  staged[id]=val; seen[id]=true; p=skipws(end);
  if(*p=='}') { ++p; break; }
  if(*p++!=',') return VP_ERROR_PARSE;
 }
 if(*skipws(p)!='\0') return VP_ERROR_PARSE;
 // Host must quiesce audio before preset mutation; validation is complete before commit.
 for(int i=0;i<VP_PARAM_COUNT;++i) if(seen[i]) {
  const vp_status s=h->processor.setParameter(i,staged[i]); if(s!=VP_OK) return s;
 }
 return VP_OK;
}
vp_status vp_process_planar(vp_handle* h,float* const* c,int nc,int nf) {
 if(!h||!c) return VP_ERROR_NULL;
 if(nf<0||nc<1||nc>2) return VP_ERROR_RANGE;
 for(int i=0;i<nc;++i) if(!c[i]) return VP_ERROR_NULL;
 h->processor.process(c,nc,nf); return VP_OK;
}
vp_status vp_process_interleaved(vp_handle* h,float* d,int nc,int nf) {
 if(!h||!d) return VP_ERROR_NULL;
 if(nf<0||nc<1||nc>2) return VP_ERROR_RANGE;
 h->processor.processInterleaved(d,nc,nf); return VP_OK;
}
int vp_latency_samples(const vp_handle*) { return 0; }
uint32_t vp_last_warnings(const vp_handle* h) { return h?h->processor.warnings():0; }
vp_status vp_last_status(const vp_handle* h) { return h?h->processor.status():VP_ERROR_NULL; }
}
