#include "nyas.h"

#include <mathc.h>
#include <stdio.h>

struct PbrDataDesc
{
    float Model[16];
    float Color[3];
    float UseAlbedo;
    float UseRougAndMetal;
    float TilingX;
    float TilingY;
    float Reflectance;
    float Roughness;
    float Metallic;
    float UseNormalMap;
    float _Padding;
};

struct PbrSharedDesc
{
    float ViewProj[16];
    NyVec3 CameraEye;
    float _Padding;
    float Sunlight[4]; // Alpha for light intensity
};

struct PbrMaps
{
    NyasHandle Alb, Nor, Rou, Met;
};

static const struct
{
    const NyasShaderDesc Pbr;
    const NyasShaderDesc FullscreenImg;
    const NyasShaderDesc Sky;
} g_shader_descriptors = {
    {"pbr", 7 * 4, 4, 0, 6 * 4, 1, 2}, {"fullscreen-img", 0, 0, 0, 0, 1, 0}, {"skybox", 0, 0, 0, 4 * 4, 0, 1}};

struct
{
    NyasHandle FullscreenImg;
    NyasHandle Skybox;
    NyasHandle Pbr;
} G_Shaders;

struct
{
    NyasHandle sky;

    struct PbrMaps gold;
    struct PbrMaps peeled;
    struct PbrMaps rusted;
    struct PbrMaps tiles;
    struct PbrMaps plastic;
    struct PbrMaps shore;
    struct PbrMaps cliff;
    struct PbrMaps granite;
    struct PbrMaps foam;
} G_Tex;

NyasHandle G_Framebuf;
NyasHandle G_FbTex;
NyasHandle G_Mesh;

void Init(void)
{
    NyUtil::nyut_mesh_init_geometry();
    NyasHandle irradiance;
    NyasHandle prefilter;
    NyasHandle lut;

    ShaderLoaderArgs fsimgargs = {g_shader_descriptors.FullscreenImg, &G_Shaders.FullscreenImg};
    ShaderLoaderArgs skyargs = {g_shader_descriptors.Sky, &G_Shaders.Skybox};
    ShaderLoaderArgs pbrargs = {g_shader_descriptors.Pbr, &G_Shaders.Pbr};
    MeshLoaderArgs mesh = {"assets/obj/matball.msh", &G_Mesh};
    EnvLoaderArgs envargs = {"assets/env/canyon.env", &G_Tex.sky, &irradiance, &prefilter, &lut};

    AssetLoader *ldr = NyUtil::nyut_assets_create();
    ldr->AddShader(&fsimgargs);
    ldr->AddShader(&skyargs);
    ldr->AddShader(&pbrargs);
    ldr->AddMesh(&mesh);
    ldr->AddEnv(&envargs);

    NyasTexDesc texdesc[] = {
        NyasTexDesc(NyasTexType_2D, NyasTexFmt_SRGB_8, 0, 0), NyasTexDesc(NyasTexType_2D, NyasTexFmt_RGB_8, 0, 0),
        NyasTexDesc(NyasTexType_2D, NyasTexFmt_R_8, 0, 0), NyasTexDesc(NyasTexType_2D, NyasTexFmt_R_8, 0, 0)};

    const char *texpaths[] = {"assets/tex/celtic-gold/celtic-gold_A.png",
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
                              "assets/tex/foam/foam_M.png"};

    TexLoaderArgs loadtexargs[9 * 4];
    NyasHandle *tex_begin = &G_Tex.gold.Alb;
    for (int i = 0; i < 9 * 4; ++i)
    {
        *tex_begin = Nyas::CreateTexture();
        loadtexargs[i].Tex = *tex_begin++;
        loadtexargs[i].Descriptor = texdesc[i & 3];
        loadtexargs[i].Path = texpaths[i];
    }

    for (int i = 0; i < 9 * 4; ++i)
    {
        ldr->AddTex(&loadtexargs[i]);
    }

    ldr->Load(24);

    struct PbrSharedDesc *common_pbr = (PbrSharedDesc *)Nyas::GetMaterialSharedData(G_Shaders.Pbr);
    common_pbr->Sunlight[0] = 0.0f;
    common_pbr->Sunlight[1] = -1.0f;
    common_pbr->Sunlight[2] = -0.1f;
    common_pbr->Sunlight[3] = 0.0f;

    NyasHandle *pbr_scene_tex = Nyas::GetMaterialSharedTextures(G_Shaders.Pbr);
    pbr_scene_tex[0] = lut;
    pbr_scene_tex[1] = irradiance;
    pbr_scene_tex[2] = prefilter;

    G_Framebuf = Nyas::CreateFramebuffer();
    NyVec2i vp = Nyas::IO.WindowSize;
    NyasTexDesc descriptor(NyasTexType_2D, NyasTexFmt_RGB_32F, vp.X, vp.Y);
    G_FbTex = Nyas::CreateTexture();
    Nyas::SetTexture(G_FbTex, &descriptor);
    NyasTexTarget color = {G_FbTex, NyasTexFace_2D, NyasFbAttach_Color, 0};

    NyasTexDesc depthscriptor(NyasTexType_2D, NyasTexFmt_Depth, vp.X, vp.Y);
    NyasHandle fb_depth = Nyas::CreateTexture();
    Nyas::SetTexture(fb_depth, &depthscriptor);
    NyasTexTarget depth = {fb_depth, NyasTexFace_2D, NyasFbAttach_Depth, 0};

    Nyas::SetFramebufferTarget(G_Framebuf, 0, color);
    Nyas::SetFramebufferTarget(G_Framebuf, 1, depth);

    struct PbrDataDesc pbr;
    pbr.Color[0] = 1.0f;
    pbr.Color[1] = 1.0f;
    pbr.Color[2] = 1.0f;
    pbr.TilingX = 4.0f;
    pbr.TilingY = 4.0f;
    pbr.Reflectance = 0.5f;
    pbr.UseAlbedo = 1.0f;
    pbr.UseRougAndMetal = 1.0f;
    pbr.Metallic = 0.5f;
    pbr.Roughness = 0.5f;
    pbr.UseNormalMap = 1.0f;

    float position[3] = {-2.0f, 0.0f, 0.0f};

    // CelticGold
    {
        int eidx = Nyas::Entities.Add();
        NyasEntity *e = &Nyas::Entities[eidx];
        mat4_identity(e->Transform);
        mat4_translation(e->Transform, e->Transform, position);
        e->Mesh = G_Mesh;
        e->Material = Nyas::CreateMaterial(G_Shaders.Pbr);
        *(struct PbrDataDesc *)e->Material.Ptr = pbr;
        NyasHandle *t = Nyas::GetMaterialTextures(e->Material);
        t[0] = G_Tex.gold.Alb;
        t[1] = G_Tex.gold.Met;
        t[2] = G_Tex.gold.Rou;
        t[3] = G_Tex.gold.Nor;
    }

    // Shore
    {
        pbr.TilingX = 2.0f;
        pbr.TilingY = 2.0f;
        pbr.UseNormalMap = 0.5f;
        int eidx = Nyas::Entities.Add();
        struct NyasEntity *e = &Nyas::Entities[eidx];
        mat4_identity(e->Transform);
        position[0] = 0.0f;
        mat4_translation(e->Transform, e->Transform, position);
        e->Mesh = G_Mesh;
        e->Material = Nyas::CreateMaterial(G_Shaders.Pbr);
        *(struct PbrDataDesc *)e->Material.Ptr = pbr;
        NyasHandle *t = Nyas::GetMaterialTextures(e->Material);
        t[0] = G_Tex.shore.Alb;
        t[1] = G_Tex.shore.Met;
        t[2] = G_Tex.shore.Rou;
        t[3] = G_Tex.shore.Nor;
    }

    // Peeling
    {
        pbr.TilingX = 1.0f;
        pbr.TilingY = 1.0f;
        pbr.UseNormalMap = 0.7f;
        position[0] = 2.0f;
        int eidx = Nyas::Entities.Add();
        struct NyasEntity *e = &Nyas::Entities[eidx];
        mat4_identity(e->Transform);
        mat4_translation(e->Transform, e->Transform, position);
        e->Mesh = G_Mesh;
        e->Material = Nyas::CreateMaterial(G_Shaders.Pbr);
        *(struct PbrDataDesc *)e->Material.Ptr = pbr;
        NyasHandle *t = Nyas::GetMaterialTextures(e->Material);
        t[0] = G_Tex.peeled.Alb;
        t[1] = G_Tex.peeled.Met;
        t[2] = G_Tex.peeled.Rou;
        t[3] = G_Tex.peeled.Nor;
    }

    // Rusted
    {
        pbr.TilingX = 1.0f;
        pbr.TilingY = 1.0f;
        pbr.UseNormalMap = 0.2f;
        position[0] = -2.0f;
        position[2] = -2.0f;
        int eidx = Nyas::Entities.Add();
        struct NyasEntity *e = &Nyas::Entities[eidx];
        mat4_identity(e->Transform);
        mat4_translation(e->Transform, e->Transform, position);
        e->Mesh = G_Mesh;
        e->Material = Nyas::CreateMaterial(G_Shaders.Pbr);
        *(struct PbrDataDesc *)e->Material.Ptr = pbr;
        NyasHandle *t = Nyas::GetMaterialTextures(e->Material);
        t[0] = G_Tex.rusted.Alb;
        t[1] = G_Tex.rusted.Met;
        t[2] = G_Tex.rusted.Rou;
        t[3] = G_Tex.rusted.Nor;
    }

    // Tiles
    {
        pbr.TilingX = 4.0f;
        pbr.TilingY = 4.0f;
        pbr.UseNormalMap = 1.0f;
        position[0] = 0.0f;
        int eidx = Nyas::Entities.Add();
        struct NyasEntity *e = &Nyas::Entities[eidx];
        mat4_identity(e->Transform);
        mat4_translation(e->Transform, e->Transform, position);
        e->Mesh = G_Mesh;
        e->Material = Nyas::CreateMaterial(G_Shaders.Pbr);
        *(struct PbrDataDesc *)e->Material.Ptr = pbr;
        NyasHandle *t = Nyas::GetMaterialTextures(e->Material);
        t[0] = G_Tex.tiles.Alb;
        t[1] = G_Tex.tiles.Met;
        t[2] = G_Tex.tiles.Rou;
        t[3] = G_Tex.tiles.Nor;
    }

    // Ship Panels
    {
        pbr.TilingX = 1.0f;
        pbr.TilingY = 1.0f;
        pbr.UseNormalMap = 1.0f;
        position[0] = 2.0f;
        int eidx = Nyas::Entities.Add();
        struct NyasEntity *e = &Nyas::Entities[eidx];
        mat4_identity(e->Transform);
        mat4_translation(e->Transform, e->Transform, position);
        e->Mesh = G_Mesh;
        e->Material = Nyas::CreateMaterial(G_Shaders.Pbr);
        *(struct PbrDataDesc *)e->Material.Ptr = pbr;
        NyasHandle *t = Nyas::GetMaterialTextures(e->Material);
        t[0] = G_Tex.plastic.Alb;
        t[1] = G_Tex.plastic.Met;
        t[2] = G_Tex.plastic.Rou;
        t[3] = G_Tex.plastic.Nor;
    }

    // Cliff
    {
        pbr.TilingX = 8.0f;
        pbr.TilingY = 8.0f;
        pbr.UseNormalMap = 1.0f;
        position[0] = -2.0f;
        position[2] = -4.0f;
        int eidx = Nyas::Entities.Add();
        struct NyasEntity *e = &Nyas::Entities[eidx];
        mat4_identity(e->Transform);
        mat4_translation(e->Transform, e->Transform, position);
        e->Mesh = G_Mesh;
        e->Material = Nyas::CreateMaterial(G_Shaders.Pbr);
        *(struct PbrDataDesc *)e->Material.Ptr = pbr;
        NyasHandle *t = Nyas::GetMaterialTextures(e->Material);
        t[0] = G_Tex.cliff.Alb;
        t[1] = G_Tex.cliff.Met;
        t[2] = G_Tex.cliff.Rou;
        t[3] = G_Tex.cliff.Nor;
    }

    // Granite
    {
        pbr.TilingX = 2.0f;
        pbr.TilingY = 2.0f;
        pbr.UseNormalMap = 1.0f;
        position[0] = 0.0f;
        int eidx = Nyas::Entities.Add();
        struct NyasEntity *e = &Nyas::Entities[eidx];
        mat4_identity(e->Transform);
        mat4_translation(e->Transform, e->Transform, position);
        e->Mesh = G_Mesh;
        e->Material = Nyas::CreateMaterial(G_Shaders.Pbr);
        *(struct PbrDataDesc *)e->Material.Ptr = pbr;
        NyasHandle *t = Nyas::GetMaterialTextures(e->Material);
        t[0] = G_Tex.granite.Alb;
        t[1] = G_Tex.granite.Met;
        t[2] = G_Tex.granite.Rou;
        t[3] = G_Tex.granite.Nor;
    }

    // Foam
    {
        pbr.TilingX = 2.0f;
        pbr.TilingY = 2.0f;
        pbr.UseNormalMap = 0.5f;
        position[0] = 2.0f;
        int eidx = Nyas::Entities.Add();
        struct NyasEntity *e = &Nyas::Entities[eidx];
        mat4_identity(e->Transform);
        mat4_translation(e->Transform, e->Transform, position);
        e->Mesh = G_Mesh;
        e->Material = Nyas::CreateMaterial(G_Shaders.Pbr);
        *(struct PbrDataDesc *)e->Material.Ptr = pbr;
        NyasHandle *t = Nyas::GetMaterialTextures(e->Material);
        t[0] = G_Tex.foam.Alb;
        t[1] = G_Tex.foam.Met;
        t[2] = G_Tex.foam.Rou;
        t[3] = G_Tex.foam.Nor;
    }

    *Nyas::GetMaterialSharedTextures(G_Shaders.Skybox) = G_Tex.sky;
    *Nyas::GetMaterialSharedTextures(G_Shaders.FullscreenImg) = G_FbTex;
}

void BuildFrame(NyArray<NyasDrawCmd, NyCircularAllocator<NY_MEGABYTES(16)>> &new_frame)
{
    Nyas::PollIO();
    NyVec2i vp = Nyas::IO.WindowSize;
    Nyas::Camera.Navigate();

    /* PBR common shader data. */
    struct PbrSharedDesc *common_pbr = (PbrSharedDesc *)Nyas::GetMaterialSharedData(G_Shaders.Pbr);
    mat4_multiply(common_pbr->ViewProj, Nyas::Camera.Proj, Nyas::Camera.View);
    common_pbr->CameraEye = Nyas::Camera.Eye();

    // Scene entities
    {
        NyasDrawCmd draw;
        draw.Framebuf = G_Framebuf;
        draw.ShaderMaterial = Nyas::CopyShaderMaterialTmp(G_Shaders.Pbr);
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
        draw.Units = (NyasDrawUnit *)NyFrameAllocator::Alloc(Nyas::Entities.Count * sizeof(NyasDrawUnit));
        for (int i = 0; i < Nyas::Entities.Count; ++i)
        {
            mat4_assign((float *)Nyas::Entities[i].Material.Ptr, Nyas::Entities[i].Transform);
            draw.Units[i].Material = Nyas::CopyMaterialTmp(Nyas::Entities[i].Material);
            draw.Units[i].Mesh = Nyas::Entities[i].Mesh;
        }

        new_frame.Push(draw);
    }

    // Skybox
    {
        NyasDrawCmd draw;
        Nyas::Camera.OriginViewProj((float *)Nyas::GetMaterialSharedData(G_Shaders.Skybox));
        draw.ShaderMaterial = Nyas::CopyShaderMaterialTmp(G_Shaders.Skybox);
        draw.State.DisableFlags |= NyasDrawFlags_FaceCulling;
        draw.State.Depth = NyasDepthFunc_LessEqual;
        draw.UnitCount = 1;
        draw.Units = (NyasDrawUnit *)NyFrameAllocator::Alloc(sizeof(NyasDrawUnit));
        draw.Units->Material.Shader = G_Shaders.Skybox;
        draw.Units->Mesh = NYAS_UTILS_CUBE;
        new_frame.Push(draw);
    }

    // Frame texture
    {
        NyasDrawCmd draw;
        draw.Framebuf = NyasCode_Default;
        draw.ShaderMaterial = Nyas::CopyShaderMaterialTmp(G_Shaders.FullscreenImg);
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
        draw.Units = (NyasDrawUnit *)NyFrameAllocator::Alloc(sizeof(NyasDrawUnit));
        draw.Units->Material.Shader = G_Shaders.FullscreenImg;
        draw.Units->Mesh = NYAS_UTILS_QUAD;
        new_frame.Push(draw);
    }
}

int main(int argc, char **argv)
{
    NY_UNUSED(argc), NY_UNUSED(argv);
    Nyas::InitIO("NYAS PBR Material Demo", 1920, 1080);
    Nyas::Camera.Init(Nyas::IO);
    // nuklear_init();
    Init();
    NyChrono frame_chrono;
    while (!Nyas::IO.WindowClosed)
    {
        // NewFrame
        float delta_time = NyChrono::Seconds((double)frame_chrono.Elapsed());
        frame_chrono.Restart();
        Nyas::IO.DeltaTime = delta_time;
        // Build
        NyArray<NyasDrawCmd, NyFrameAllocator> frame;
        BuildFrame(frame);

        // Render
        for (int i = 0; i < frame.Size; ++i)
        {
            Nyas::Draw(&frame[i]);
        }

        // nuklear_draw();
        Nyas::WindowSwap();
        // End render
    }

    return 0;
}
