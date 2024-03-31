#include "nyas.h"

#include <stdio.h>
#include <mathc.h>

struct pbr_desc_unit {
	float model[16];
	float color[3];
	float use_albedo_map;
	float use_pbr_maps;
	float tiling_x;
	float tiling_y;
	float reflectance;
	float roughness;
	float metallic;
	float normal_map_intensity;
	float paddingg;
};

struct pbr_desc_scene {
	float view_projection[16];
	NyVec3 camera_position;
	float padding;
	float sunlight[4];
};

struct pbr_maps {
	NyasHandle a, n, r, m;
};

static const struct {
	const NyasShaderDesc pbr;
	const NyasShaderDesc fullscreen_img;
	const NyasShaderDesc sky;
} g_shader_descriptors = {{"pbr", 7 * 4, 4, 0, 6 * 4, 1, 2}, {"fullscreen-img", 0, 0, 0, 0, 1, 0}, {"skybox", 0, 0, 0, 4 * 4, 0, 1 }};

struct {
	NyasHandle fullscreen_img;
	NyasHandle skybox;
	NyasHandle pbr;
} g_shaders;

struct {
	NyasHandle sky;

	struct pbr_maps gold;
	struct pbr_maps peeled;
	struct pbr_maps rusted;
	struct pbr_maps tiles;
	struct pbr_maps plastic;
	struct pbr_maps shore;
	struct pbr_maps cliff;
	struct pbr_maps granite;
	struct pbr_maps foam;
} g_tex;

NyasHandle g_fb;
NyasHandle fb_tex;
NyasHandle g_mesh;

void
Init(void)
{
	NyUtil::nyut_mesh_init_geometry();
	NyasHandle irradiance;
	NyasHandle prefilter;
	NyasHandle lut;

	ShaderLoaderArgs fsimgargs = {g_shader_descriptors.fullscreen_img, &g_shaders.fullscreen_img};
	ShaderLoaderArgs skyargs = {g_shader_descriptors.sky, &g_shaders.skybox};
	ShaderLoaderArgs pbrargs = {g_shader_descriptors.pbr, &g_shaders.pbr};
	MeshLoaderArgs mesh = {"assets/obj/matball.msh", &g_mesh};
	EnvLoaderArgs envargs = {"assets/env/canyon.env", &g_tex.sky, &irradiance, &prefilter, &lut};

	AssetLoader *ldr = NyUtil::nyut_assets_create();
	NyUtil::nyut_assets_add_shader(ldr, &fsimgargs);
	NyUtil::nyut_assets_add_shader(ldr, &skyargs);
	NyUtil::nyut_assets_add_shader(ldr, &pbrargs);
	NyUtil::nyut_assets_add_mesh(ldr, &mesh);
	NyUtil::nyut_assets_add_env(ldr, &envargs);

	NyasTexDesc texdesc[] = {
		NyasTexDesc(NyasTexType_2D, NyasTexFmt_SRGB_8, 0, 0),
		NyasTexDesc(NyasTexType_2D, NyasTexFmt_RGB_8, 0, 0),
		NyasTexDesc(NyasTexType_2D, NyasTexFmt_R_8, 0, 0),
		NyasTexDesc(NyasTexType_2D, NyasTexFmt_R_8, 0, 0)
	};

	const char *texpaths[] = {
		"assets/tex/celtic-gold/celtic-gold_A.png",
		"assets/tex/celtic-gold/celtic-gold_N.png",
		"assets/tex/celtic-gold/celtic-gold_R.png",
		"assets/tex/celtic-gold/celtic-gold_M.png",
		"assets/tex/peeling/peeling_A.png",
		"assets/tex/peeling/peeling_N.png",
		"assets/tex/peeling/peeling_R.png",
		"assets/tex/peeling/peeling_M.png",
		"assets/tex/rusted/rusted_A.png",
		"assets/tex/rusted/rusted_N.png",
		"assets/tex/rusted/rusted_R.png",
		"assets/tex/rusted/rusted_M.png",
		"assets/tex/tiles/tiles_A.png",
		"assets/tex/tiles/tiles_N.png",
		"assets/tex/tiles/tiles_R.png",
		"assets/tex/tiles/tiles_M.png",
		"assets/tex/ship-panels/ship-panels_A.png",
		"assets/tex/ship-panels/ship-panels_N.png",
		"assets/tex/ship-panels/ship-panels_R.png",
		"assets/tex/ship-panels/ship-panels_M.png",
		"assets/tex/shore/shore_A.png",
		"assets/tex/shore/shore_N.png",
		"assets/tex/shore/shore_R.png",
		"assets/tex/shore/shore_M.png",
		"assets/tex/cliff/cliff_A.png",
		"assets/tex/cliff/cliff_N.png",
		"assets/tex/cliff/cliff_R.png",
		"assets/tex/cliff/cliff_M.png",
		"assets/tex/granite/granite_A.png",
		"assets/tex/granite/granite_N.png",
		"assets/tex/granite/granite_R.png",
		"assets/tex/granite/granite_M.png",
		"assets/tex/foam/foam_A.png",
		"assets/tex/foam/foam_N.png",
		"assets/tex/foam/foam_R.png",
		"assets/tex/foam/foam_M.png"
	};

	TexLoaderArgs loadtexargs[9 * 4];
	NyasHandle *tex_begin = &g_tex.gold.a;
	for (int i = 0; i < 9 * 4; ++i) {
		*tex_begin = Nyas::CreateTexture();
		loadtexargs[i].Tex = *tex_begin++;
		loadtexargs[i].Descriptor = texdesc[i & 3];
		loadtexargs[i].Path = texpaths[i];
	}

	for (int i = 0; i < 9 * 4; ++i) {
		NyUtil::nyut_assets_add_tex(ldr, &loadtexargs[i]);
	}

	NyUtil::nyut_assets_load(ldr, 24);

	struct pbr_desc_scene *common_pbr = (pbr_desc_scene*)Nyas::GetMaterialSharedData(g_shaders.pbr);
	common_pbr->sunlight[0] = 0.0f;
	common_pbr->sunlight[1] = -1.0f;
	common_pbr->sunlight[2] = -0.1f;
	common_pbr->sunlight[3] = 0.0f;

	NyasHandle *pbr_scene_tex = Nyas::GetMaterialSharedTextures(g_shaders.pbr);
	pbr_scene_tex[0] = lut;
	pbr_scene_tex[1] = irradiance;
	pbr_scene_tex[2] = prefilter;

	g_fb = Nyas::CreateFramebuffer();
	NyVec2i vp = Nyas::IO.WindowSize;
	NyasTexDesc descriptor(NyasTexType_2D, NyasTexFmt_RGB_32F, vp.X, vp.Y);
	fb_tex = Nyas::CreateTexture();
	Nyas::SetTexture(fb_tex, &descriptor);
	NyasTexTarget color = {fb_tex, NyasTexFace_2D, NyasFbAttach_Color, 0};

	NyasTexDesc depthscriptor(NyasTexType_2D, NyasTexFmt_Depth, vp.X, vp.Y);
	NyasHandle fb_depth = Nyas::CreateTexture();
	Nyas::SetTexture(fb_depth, &depthscriptor);
	NyasTexTarget depth = {fb_depth, NyasTexFace_2D, NyasFbAttach_Depth, 0};

	Nyas::SetFramebufferTarget(g_fb, 0, color);
	Nyas::SetFramebufferTarget(g_fb, 1, depth);

	struct pbr_desc_unit pbr;
	pbr.color[0] = 1.0f;
	pbr.color[1] = 1.0f;
	pbr.color[2] = 1.0f;
	pbr.tiling_x = 4.0f;
	pbr.tiling_y = 4.0f;
	pbr.reflectance = 0.5f;
	pbr.use_albedo_map = 1.0f;
	pbr.use_pbr_maps = 1.0f;
	pbr.metallic = 0.5f;
	pbr.roughness = 0.5f;
	pbr.normal_map_intensity = 1.0f;

	float position[3] = { -2.0f, 0.0f, 0.0f };

	// CelticGold
	{
		int eidx = Nyas::Entities.Add();
		NyasEntity *e = &Nyas::Entities[eidx];
		mat4_identity(e->Transform);
		mat4_translation(e->Transform, e->Transform, position);
		e->Mesh = g_mesh;
		e->Material = Nyas::CreateMaterial(g_shaders.pbr);
		*(struct pbr_desc_unit *)e->Material.Ptr = pbr;
		NyasHandle *t = Nyas::GetMaterialTextures(e->Material);
		t[0] = g_tex.gold.a;
		t[1] = g_tex.gold.m;
		t[2] = g_tex.gold.r;
		t[3] = g_tex.gold.n;
	}

	// Shore
	{
		pbr.tiling_x = 2.0f;
		pbr.tiling_y = 2.0f;
		pbr.normal_map_intensity = 0.5f;
		int eidx = Nyas::Entities.Add();
		struct NyasEntity *e = &Nyas::Entities[eidx];
		mat4_identity(e->Transform);
		position[0] = 0.0f;
		mat4_translation(e->Transform, e->Transform, position);
		e->Mesh = g_mesh;
		e->Material = Nyas::CreateMaterial(g_shaders.pbr);
		*(struct pbr_desc_unit *)e->Material.Ptr = pbr;
		NyasHandle *t = Nyas::GetMaterialTextures(e->Material);
		t[0] = g_tex.shore.a;
		t[1] = g_tex.shore.m;
		t[2] = g_tex.shore.r;
		t[3] = g_tex.shore.n;
	}

	// Peeling
	{
		pbr.tiling_x = 1.0f;
		pbr.tiling_y = 1.0f;
		pbr.normal_map_intensity = 0.7f;
		position[0] = 2.0f;
		int eidx = Nyas::Entities.Add();
		struct NyasEntity *e = &Nyas::Entities[eidx];
		mat4_identity(e->Transform);
		mat4_translation(e->Transform, e->Transform, position);
		e->Mesh = g_mesh;
		e->Material = Nyas::CreateMaterial(g_shaders.pbr);
		*(struct pbr_desc_unit *)e->Material.Ptr = pbr;
		NyasHandle *t = Nyas::GetMaterialTextures(e->Material);
		t[0] = g_tex.peeled.a;
		t[1] = g_tex.peeled.m;
		t[2] = g_tex.peeled.r;
		t[3] = g_tex.peeled.n;
	}

	// Rusted
	{
		pbr.tiling_x = 1.0f;
		pbr.tiling_y = 1.0f;
		pbr.normal_map_intensity = 0.2f;
		position[0] = -2.0f;
		position[2] = -2.0f;
		int eidx = Nyas::Entities.Add();
		struct NyasEntity *e = &Nyas::Entities[eidx];
		mat4_identity(e->Transform);
		mat4_translation(e->Transform, e->Transform, position);
		e->Mesh = g_mesh;
		e->Material = Nyas::CreateMaterial(g_shaders.pbr);
		*(struct pbr_desc_unit *)e->Material.Ptr = pbr;
		NyasHandle *t = Nyas::GetMaterialTextures(e->Material);
		t[0] = g_tex.rusted.a;
		t[1] = g_tex.rusted.m;
		t[2] = g_tex.rusted.r;
		t[3] = g_tex.rusted.n;
	}

	// Tiles
	{
		pbr.tiling_x = 4.0f;
		pbr.tiling_y = 4.0f;
		pbr.normal_map_intensity = 1.0f;
		position[0] = 0.0f;
		int eidx = Nyas::Entities.Add();
		struct NyasEntity *e = &Nyas::Entities[eidx];
		mat4_identity(e->Transform);
		mat4_translation(e->Transform, e->Transform, position);
		e->Mesh = g_mesh;
		e->Material = Nyas::CreateMaterial(g_shaders.pbr);
		*(struct pbr_desc_unit *)e->Material.Ptr = pbr;
		NyasHandle *t = Nyas::GetMaterialTextures(e->Material);
		t[0] = g_tex.tiles.a;
		t[1] = g_tex.tiles.m;
		t[2] = g_tex.tiles.r;
		t[3] = g_tex.tiles.n;
	}

	// Ship Panels
	{
		pbr.tiling_x = 1.0f;
		pbr.tiling_y = 1.0f;
		pbr.normal_map_intensity = 1.0f;
		position[0] = 2.0f;
		int eidx = Nyas::Entities.Add();
		struct NyasEntity *e = &Nyas::Entities[eidx];
		mat4_identity(e->Transform);
		mat4_translation(e->Transform, e->Transform, position);
		e->Mesh = g_mesh;
		e->Material = Nyas::CreateMaterial(g_shaders.pbr);
		*(struct pbr_desc_unit *)e->Material.Ptr = pbr;
		NyasHandle *t = Nyas::GetMaterialTextures(e->Material);
		t[0] = g_tex.plastic.a;
		t[1] = g_tex.plastic.m;
		t[2] = g_tex.plastic.r;
		t[3] = g_tex.plastic.n;
	}

	// Cliff
	{
		pbr.tiling_x = 8.0f;
		pbr.tiling_y = 8.0f;
		pbr.normal_map_intensity = 1.0f;
		position[0] = -2.0f;
		position[2] = -4.0f;
		int eidx = Nyas::Entities.Add();
		struct NyasEntity *e = &Nyas::Entities[eidx];
		mat4_identity(e->Transform);
		mat4_translation(e->Transform, e->Transform, position);
		e->Mesh = g_mesh;
		e->Material = Nyas::CreateMaterial(g_shaders.pbr);
		*(struct pbr_desc_unit *)e->Material.Ptr = pbr;
		NyasHandle *t = Nyas::GetMaterialTextures(e->Material);
		t[0] = g_tex.cliff.a;
		t[1] = g_tex.cliff.m;
		t[2] = g_tex.cliff.r;
		t[3] = g_tex.cliff.n;
	}

	// Granite
	{
		pbr.tiling_x = 2.0f;
		pbr.tiling_y = 2.0f;
		pbr.normal_map_intensity = 1.0f;
		position[0] = 0.0f;
		int eidx = Nyas::Entities.Add();
		struct NyasEntity *e = &Nyas::Entities[eidx];
		mat4_identity(e->Transform);
		mat4_translation(e->Transform, e->Transform, position);
		e->Mesh = g_mesh;
		e->Material = Nyas::CreateMaterial(g_shaders.pbr);
		*(struct pbr_desc_unit *)e->Material.Ptr = pbr;
		NyasHandle *t = Nyas::GetMaterialTextures(e->Material);
		t[0] = g_tex.granite.a;
		t[1] = g_tex.granite.m;
		t[2] = g_tex.granite.r;
		t[3] = g_tex.granite.n;
	}

	// Foam
	{
		pbr.tiling_x = 2.0f;
		pbr.tiling_y = 2.0f;
		pbr.normal_map_intensity = 0.5f;
		position[0] = 2.0f;
		int eidx = Nyas::Entities.Add();
		struct NyasEntity *e = &Nyas::Entities[eidx];
		mat4_identity(e->Transform);
		mat4_translation(e->Transform, e->Transform, position);
		e->Mesh = g_mesh;
		e->Material = Nyas::CreateMaterial(g_shaders.pbr);
		*(struct pbr_desc_unit *)e->Material.Ptr = pbr;
		NyasHandle *t = Nyas::GetMaterialTextures(e->Material);
		t[0] = g_tex.foam.a;
		t[1] = g_tex.foam.m;
		t[2] = g_tex.foam.r;
		t[3] = g_tex.foam.n;
	}

	*Nyas::GetMaterialSharedTextures(g_shaders.skybox) = g_tex.sky;
	*Nyas::GetMaterialSharedTextures(g_shaders.fullscreen_img) = fb_tex;
}

void
BuildFrame(NyArray<NyasDrawCmd, NyCircularAllocator<NY_MEGABYTES(16)>>& new_frame)
{
	Nyas::PollIO();
	NyVec2i vp = Nyas::IO.WindowSize;
	Nyas::Camera.Navigate();

	/* PBR common shader data. */
	struct pbr_desc_scene *common_pbr = (pbr_desc_scene*)Nyas::GetMaterialSharedData(g_shaders.pbr);
	mat4_multiply(common_pbr->view_projection, Nyas::Camera.Proj, Nyas::Camera.View);
	common_pbr->camera_position = Nyas::Camera.Eye();

	// Scene entities
	{
		NyasDrawCmd draw;
		draw.Framebuf = g_fb;
		draw.ShaderMaterial = Nyas::CopyShaderMaterialTmp(g_shaders.pbr);
		draw.State.BgColorR = 0.2f;
		draw.State.BgColorG = 0.2f;
		draw.State.BgColorB = 0.2f;
		draw.State.BgColorA = 1.0f;
		draw.State.ViewportMinX = 0;
		draw.State.ViewportMinY = 0;
		draw.State.ViewportMaxX = vp.X;
		draw.State.ViewportMaxY = vp.Y;
		draw.State.EnableFlags |= NyasDrawFlags_ColorClear;
		draw.State.EnableFlags |= NyasDrawFlags_DepthClear;
		draw.State.EnableFlags |= NyasDrawFlags_Blend;
		draw.State.EnableFlags |= NyasDrawFlags_DepthTest;
		draw.State.EnableFlags |= NyasDrawFlags_DepthWrite;
		draw.State.BlendSrc = NyasBlendFunc_One;
		draw.State.BlendDst = NyasBlendFunc_Zero;
		draw.State.Depth = NyasDepthFunc_Less;
		draw.State.FaceCulling = NyasFaceCull_Back;

		draw.UnitCount = Nyas::Entities.Count;
		draw.Units = (NyasDrawUnit*)NyFrameAllocator::Alloc(Nyas::Entities.Count * sizeof(NyasDrawUnit));
		for (int i = 0; i < Nyas::Entities.Count; ++i)
		{
			mat4_assign((float*)Nyas::Entities[i].Material.Ptr, Nyas::Entities[i].Transform);
			draw.Units[i].Material = Nyas::CopyMaterialTmp(Nyas::Entities[i].Material);
			draw.Units[i].Mesh = Nyas::Entities[i].Mesh;
		}

		new_frame.Push(draw);
	}
	
	// Skybox
	{
		NyasDrawCmd draw;
		Nyas::Camera.OriginViewProj((float*)Nyas::GetMaterialSharedData(g_shaders.skybox));
		draw.ShaderMaterial = Nyas::CopyShaderMaterialTmp(g_shaders.skybox);
		draw.State.DisableFlags |= NyasDrawFlags_FaceCulling;
		draw.State.Depth = NyasDepthFunc_LessEqual;
		draw.UnitCount = 1;
		draw.Units = (NyasDrawUnit*)NyFrameAllocator::Alloc(sizeof(NyasDrawUnit));
		draw.Units->Material.Shader = g_shaders.skybox;
		draw.Units->Mesh = NYAS_UTILS_CUBE;
		new_frame.Push(draw);
	}

	// Frame texture
	{
		NyasDrawCmd draw;
		draw.Framebuf = NyasCode_Default;
		draw.ShaderMaterial = Nyas::CopyShaderMaterialTmp(g_shaders.fullscreen_img);
		draw.State.BgColorR = 1.0f;
		draw.State.BgColorG = 0.0f;
		draw.State.BgColorB = 0.0f;
		draw.State.BgColorA = 1.0f;
		draw.State.ViewportMinX = 0;
		draw.State.ViewportMinY = 0;
		draw.State.ViewportMaxX = vp.X;
		draw.State.ViewportMaxY = vp.Y;
		draw.State.DisableFlags |= NyasDrawFlags_DepthTest;
		draw.UnitCount = 1;
		draw.Units = (NyasDrawUnit*)NyFrameAllocator::Alloc(sizeof(NyasDrawUnit));
		draw.Units->Material.Shader = g_shaders.fullscreen_img;
		draw.Units->Mesh = NYAS_UTILS_QUAD;
		new_frame.Push(draw);
	}
}

int
main(int argc, char **argv)
{
	NY_UNUSED(argc), NY_UNUSED(argv);
	Nyas::InitIO("NYAS PBR Material Demo", 1920, 1080);
	Nyas::Camera.Init(Nyas::IO);
	//nuklear_init();
	Init();
	NyChrono frame_chrono;
	while (!Nyas::IO.WindowClosed) {
		// NewFrame
		float delta_time = NyChrono::Seconds((double)frame_chrono.Elapsed());
		frame_chrono.Restart();
		Nyas::IO.DeltaTime = delta_time;

		// Build
		NyArray<NyasDrawCmd, NyFrameAllocator> frame;
		BuildFrame(frame);

		// Render
		for (int i = 0; i < frame.Size; ++i) {
			Nyas::Draw(&frame[i]);
		}

		//nuklear_draw();
		Nyas::WindowSwap();
		// End render
	}

	return 0;
}
