#ifndef NYAS_RENDER_H
#define NYAS_RENDER_H

#include "nyas_types.h"

namespace nyas::render
{
	void _NySetProcLoader(void *(*load_fn)(const char *));
	void _NyCreateTex(NyasTexFlags flags, int unit, NyResourceID *out_id, int count = 1);
	void _NyAllocTex(NyResourceID id, NyasTexInfo info, int layers = 1);
	void _NySetTex(NyResourceID id, NyasTexture tex, NyasTexImage img, NyasTexInfo info, int faces = 1);

	void _NyCreatePipeline(NyResourceID *id, NyasPipeline *pipeline);
	void _NyBuildPipeline(NyResourceID id, NyasPipelineBuilder *pb);
	void _NyUsePipeline(NyResourceID id, NyasPipeline pipeline);

	void _NyCreateMesh(uint32_t *id, uint32_t *vid, uint32_t *iid);
	void _NyUseMesh(NyasMesh *m);
	void _NySetMesh(NyasMesh *mesh, uint32_t shader_id);
	void _NyReleaseMesh(uint32_t *id, uint32_t *vid, uint32_t *iid);

	void _NyCreateFramebuf(NyasFramebuffer *fb);
	void _NySetFramebuf(uint32_t fb_id, NyasTexTarget *tt);
	void _NyUseFramebuf(uint32_t id);
	void _NyReleaseFramebuf(NyasFramebuffer *fb);

	void _NyClear(bool color = true, bool depth = true, bool stencil = false);
	void _NyDraw(int elem_count, int index_type, int instances = 1);
	void _NyClearColor(float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 1.0f);
	void _NyEnableScissor();
	void _NyDisableScissor();
	void _NyEnableBlend();
	void _NyDisableBlend();
	void _NySetBlend(NyasBlendFunc blend_func_src, NyasBlendFunc blend_func_dst);
	void _NyEnableCull();
	void _NyDisableCull();
	void _NySetCull(NyasFaceCull cull_face);
	void _NyEnableDepthTest();
	void _NyDisableDepthTest();
	void _NyEnableDepthMask();
	void _NyDisableDepthMask();
	void _NySetDepthFunc(NyasDepthFunc depth_func);
	void _NyEnableStencilTest();
	void _NyDisableStencilTest();
	void _NyEnableStencilMask();
	void _NyDisableStencilMask();
	void _NyViewport(nym::rect_t rect);
	void _NyScissor(nym::rect_t rect);
}

#endif //NYAS_RENDER_H
