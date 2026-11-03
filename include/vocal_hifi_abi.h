#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct vp_handle vp_handle;
typedef enum vp_status {
 VP_OK=0, VP_ERROR_NULL=-1, VP_ERROR_RANGE=-2, VP_ERROR_STATE=-3, VP_ERROR_PARSE=-4, VP_ERROR_MEMORY=-5
} vp_status;
typedef enum vp_parameter {
 VP_PARAM_INPUT_GAIN=0, VP_PARAM_HP_HZ, VP_PARAM_MUD_DB, VP_PARAM_PRESENCE_DB,
 VP_PARAM_AIR_DB, VP_PARAM_ARTIFACT_DIP_DB, VP_PARAM_WARMTH, VP_PARAM_DEESS_AMOUNT,
 VP_PARAM_DOLBY_STRETCH, VP_PARAM_BREATH_AMOUNT, VP_PARAM_COMPRESSOR_AMOUNT,
 VP_PARAM_BREATH_LIFT, VP_PARAM_LAYER_GAIN, VP_PARAM_ADT_MIX, VP_PARAM_ADT_DELAY_MS,
 VP_PARAM_BUS_WARMTH, VP_PARAM_OUTPUT_GAIN, VP_PARAM_SOFT_CLIP, VP_PARAM_COUNT
} vp_parameter;
enum { VP_WARN_NONE=0, VP_WARN_BREATH_CLAMPED=1u<<0 };
vp_handle* vp_create(void);
void vp_destroy(vp_handle*);
vp_status vp_prepare(vp_handle*, double sample_rate, int max_frames, int channels, int layers);
vp_status vp_set_param(vp_handle*, int parameter, float value);
vp_status vp_get_param(const vp_handle*, int parameter, float* value);
vp_status vp_load_preset(vp_handle*, const char* json);
vp_status vp_process_planar(vp_handle*, float* const* channels, int num_channels, int num_frames);
vp_status vp_process_interleaved(vp_handle*, float* samples, int num_channels, int num_frames);
int vp_latency_samples(const vp_handle*);
uint32_t vp_last_warnings(const vp_handle*);
vp_status vp_last_status(const vp_handle*);
#ifdef __cplusplus
}
#endif
