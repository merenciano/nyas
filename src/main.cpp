#include "nyas.h"
#include <mathc.h>
#include <stdio.h>

struct PbrDataDesc
{
    float Model[16];
    float Color[3];
    float UseAlbedo;
    float TilingX;
    float TilingY;
    float UseRougAndMetal;
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
} G_ShaderDescriptors = { { "pbr", 1, 2, 4, sizeof(PbrDataDesc) * NYAS_PIPELINE_MAX_UNITS, sizeof(PbrSharedDesc)},
    { "fullscreen-img", 1 }, { "skybox", 0, 1, 0, 16 * sizeof(float), 0 } };

struct
{
    NyasHandle FullscreenImg;
    NyasHandle Skybox;
    NyasHandle Pbr;
} G_Shaders;

struct
{
    NyasHandle Sky;
    PbrMaps PbrMaps;
} G_Tex;

NyasHandle G_Framebuf;
NyasHandle G_FbTex;
NyasHandle G_Mesh;

void Init(void)
{
    NyUtil::LoadBasicGeometries();
    NyasHandle irradiance;
    NyasHandle prefilter;
    NyasHandle lut;

    NyAssetLoader::ShaderArgs fsimgargs = { G_ShaderDescriptors.FullscreenImg,
        &G_Shaders.FullscreenImg };
    NyAssetLoader::ShaderArgs skyargs = { G_ShaderDescriptors.Sky, &G_Shaders.Skybox };
    NyAssetLoader::ShaderArgs pbrargs = { G_ShaderDescriptors.Pbr, &G_Shaders.Pbr };
    NyAssetLoader::MeshArgs mesh = { "assets/obj/matball.msh", &G_Mesh };
    NyAssetLoader::EnvArgs envargs = { "assets/env/canyon.env", &G_Tex.Sky, &irradiance, &prefilter,
        &lut };

    NyAssetLoader ldr;
    ldr.AddShader(&fsimgargs);
    ldr.AddShader(&skyargs);
    ldr.AddShader(&pbrargs);
    ldr.AddMesh(&mesh);
    ldr.AddEnv(&envargs);

    NyasTexDesc texdesc[] = { NyasTexDesc(NyasTexType_Array2D, NyasTexFmt_SRGB_8, 0, 0, 9),
        NyasTexDesc(NyasTexType_Array2D, NyasTexFmt_RGB_8, 0, 0, 9),
        NyasTexDesc(NyasTexType_Array2D, NyasTexFmt_R_8, 0, 0, 9),
        NyasTexDesc(NyasTexType_Array2D, NyasTexFmt_R_8, 0, 0, 9) };

    const char *texpaths[] = { "assets/tex/celtic-gold/celtic-gold_A.png",
        "assets/tex/celtic-gold/celtic-gold_N.png", "assets/tex/celtic-gold/celtic-gold_R.png",
        "assets/tex/celtic-gold/celtic-gold_M.png", "assets/tex/peeling/peeling_A.png",
        "assets/tex/peeling/peeling_N.png", "assets/tex/peeling/peeling_R.png",
        "assets/tex/peeling/peeling_M.png", "assets/tex/rusted/rusted_A.png",
        "assets/tex/rusted/rusted_N.png", "assets/tex/rusted/rusted_R.png",
        "assets/tex/rusted/rusted_M.png", "assets/tex/tiles/tiles_A.png",
        "assets/tex/tiles/tiles_N.png", "assets/tex/tiles/tiles_R.png",
        "assets/tex/tiles/tiles_M.png", "assets/tex/ship-panels/ship-panels_A.png",
        "assets/tex/ship-panels/ship-panels_N.png", "assets/tex/ship-panels/ship-panels_R.png",
        "assets/tex/ship-panels/ship-panels_M.png", "assets/tex/shore/shore_A.png",
        "assets/tex/shore/shore_N.png", "assets/tex/shore/shore_R.png",
        "assets/tex/shore/shore_M.png", "assets/tex/cliff/cliff_A.png",
        "assets/tex/cliff/cliff_N.png", "assets/tex/cliff/cliff_R.png",
        "assets/tex/cliff/cliff_M.png", "assets/tex/granite/granite_A.png",
        "assets/tex/granite/granite_N.png", "assets/tex/granite/granite_R.png",
        "assets/tex/granite/granite_M.png", "assets/tex/foam/foam_A.png",
        "assets/tex/foam/foam_N.png", "assets/tex/foam/foam_R.png", "assets/tex/foam/foam_M.png" };

    NyAssetLoader::TexArgs loadtexargs[4];
    NyasHandle *tex_begin = &G_Tex.PbrMaps.Alb;
    for (int i = 0; i < 4; ++i)
    {
        *tex_begin = Nyas::CreateTexture(0, 0, texdesc[i].Type, texdesc[i].Format, 9);
        loadtexargs[i].Tex = *tex_begin++;
        loadtexargs[i].Descriptor = texdesc[i];
        for (int t = 0; t < 9; ++t)
        {
            loadtexargs[i].Path[t] = texpaths[t * 4 + i];
        }
    }

    for (int i = 0; i < 4; ++i)
    {
        ldr.AddTex(&loadtexargs[i]);
    }

    ldr.Load(24);

    Nyas::Shaders[G_Shaders.Pbr].TexArrays = (NyasHandle*)malloc(4 * sizeof(NyasHandle));
    PbrSharedDesc *shared = (PbrSharedDesc*)Nyas::Shaders[G_Shaders.Pbr].SharedBlock;
    shared->Sunlight[0] = 0.0f;
    shared->Sunlight[1] = -1.0f;
    shared->Sunlight[2] = -0.1f;
    shared->Sunlight[3] = 0.0f;

    NyasHandle *pbr_scene_tex = Nyas::Shaders[G_Shaders.Pbr].Shared;
    pbr_scene_tex[0] = lut;
    pbr_scene_tex[1] = irradiance;
    pbr_scene_tex[2] = prefilter;

    G_Framebuf = Nyas::CreateFramebuffer();
    NyVec2i vp = Nyas::GetCurrentCtx()->Platform.WindowSize;
    NyasTexDesc descriptor(NyasTexType_2D, NyasTexFmt_RGB_32F, vp.X, vp.Y);
    G_FbTex = Nyas::CreateTexture();
    Nyas::SetTexture(G_FbTex, &descriptor);
    NyasTexTarget color = { G_FbTex, NyasTexFace_2D, NyasFbAttach_Color, 0 };

    NyasTexDesc depthscriptor(NyasTexType_2D, NyasTexFmt_Depth, vp.X, vp.Y);
    NyasHandle fb_depth = Nyas::CreateTexture();
    Nyas::SetTexture(fb_depth, &depthscriptor);
    NyasTexTarget depth = { fb_depth, NyasTexFace_2D, NyasFbAttach_Depth, 0 };

    Nyas::SetFramebufferTarget(G_Framebuf, 0, color);
    Nyas::SetFramebufferTarget(G_Framebuf, 1, depth);

    PbrDataDesc pbr;
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

    float position[3] = { -2.0f, 0.0f, 0.0f };

    auto *pbr_uniform_block = (PbrDataDesc*)Nyas::Shaders[G_Shaders.Pbr].UnitBlock;
    // CelticGold
    {
        int eidx = Nyas::Entities.Add();
        NyasEntity *e = &Nyas::Entities[eidx];
        mat4_identity(e->Transform);
        mat4_translation(e->Transform, e->Transform, position);
        e->Mesh = G_Mesh;
        e->Shader = G_Shaders.Pbr;
        pbr_uniform_block[eidx] = pbr;
    }

    // Peeling
    {
        pbr.TilingX = 1.0f;
        pbr.TilingY = 1.0f;
        pbr.UseNormalMap = 0.7f;
        position[0] = 0.0f;
        int eidx = Nyas::Entities.Add();
        struct NyasEntity *e = &Nyas::Entities[eidx];
        mat4_identity(e->Transform);
        mat4_translation(e->Transform, e->Transform, position);
        e->Mesh = G_Mesh;
        e->Shader = G_Shaders.Pbr;
        pbr_uniform_block[eidx] = pbr;
    }

    // Rusted
    {
        pbr.TilingX = 1.0f;
        pbr.TilingY = 1.0f;
        pbr.UseNormalMap = 0.2f;
        position[0] = 2.0f;
        int eidx = Nyas::Entities.Add();
        struct NyasEntity *e = &Nyas::Entities[eidx];
        mat4_identity(e->Transform);
        mat4_translation(e->Transform, e->Transform, position);
        e->Mesh = G_Mesh;
        e->Shader = G_Shaders.Pbr;
        pbr_uniform_block[eidx] = pbr;
    }

    // Tiles
    {
        pbr.TilingX = 4.0f;
        pbr.TilingY = 4.0f;
        pbr.UseNormalMap = 1.0f;
        position[0] = -2.0f;
        position[2] = -2.0f;
        int eidx = Nyas::Entities.Add();
        struct NyasEntity *e = &Nyas::Entities[eidx];
        mat4_identity(e->Transform);
        mat4_translation(e->Transform, e->Transform, position);
        e->Mesh = G_Mesh;
        e->Shader = G_Shaders.Pbr;
        pbr_uniform_block[eidx] = pbr;
    }

    // Ship Panels
    {
        pbr.TilingX = 1.0f;
        pbr.TilingY = 1.0f;
        pbr.UseNormalMap = 1.0f;
        position[0] = 0.0f;
        int eidx = Nyas::Entities.Add();
        struct NyasEntity *e = &Nyas::Entities[eidx];
        mat4_identity(e->Transform);
        mat4_translation(e->Transform, e->Transform, position);
        e->Mesh = G_Mesh;
        e->Shader = G_Shaders.Pbr;
        pbr_uniform_block[eidx] = pbr;
    }

     // Shore
    {
        pbr.TilingX = 2.0f;
        pbr.TilingY = 2.0f;
        pbr.UseNormalMap = 0.5f;
        int eidx = Nyas::Entities.Add();
        struct NyasEntity *e = &Nyas::Entities[eidx];
        mat4_identity(e->Transform);
        position[0] = 2.0f;
        mat4_translation(e->Transform, e->Transform, position);
        e->Mesh = G_Mesh;
        e->Shader = G_Shaders.Pbr;
        pbr_uniform_block[eidx] = pbr;
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
        e->Shader = G_Shaders.Pbr;
        pbr_uniform_block[eidx] = pbr;
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
        e->Shader = G_Shaders.Pbr;
        pbr_uniform_block[eidx] = pbr;
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
        e->Shader = G_Shaders.Pbr;
        pbr_uniform_block[eidx] = pbr;
    }

    *Nyas::Shaders[G_Shaders.Skybox].Shared = G_Tex.Sky;
    *Nyas::Shaders[G_Shaders.FullscreenImg].Shared = G_FbTex;
    Nyas::Shaders[G_Shaders.Pbr].TexArrays[0] = G_Tex.PbrMaps.Alb;
    Nyas::Shaders[G_Shaders.Pbr].TexArrays[1] = G_Tex.PbrMaps.Met;
    Nyas::Shaders[G_Shaders.Pbr].TexArrays[2] = G_Tex.PbrMaps.Rou;
    Nyas::Shaders[G_Shaders.Pbr].TexArrays[3] = G_Tex.PbrMaps.Nor;
}

void BuildFrame(NyArray<NyasDrawCmd, NyCircularAllocator<NY_MEGABYTES(16)>> &new_frame)
{
    Nyas::PollIO();
    NyVec2i vp = Nyas::GetCurrentCtx()->Platform.WindowSize;
    Nyas::Camera.Navigate();

    /* PBR common shader data. */
    auto *pbr_shared_block = (PbrSharedDesc*)Nyas::Shaders[G_Shaders.Pbr].SharedBlock;
    mat4_multiply(pbr_shared_block->ViewProj, Nyas::Camera.Proj, Nyas::Camera.View);
    pbr_shared_block->CameraEye = Nyas::Camera.Eye();

    // Scene entities
    {
        NyasDrawCmd draw;
        draw.Framebuf = G_Framebuf;
        draw.Shader = G_Shaders.Pbr;
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

        draw.UnitCount = 1;
        draw.Units =
            (NyasDrawUnit *)NyFrameAllocator::Alloc(1 * sizeof(NyasDrawUnit));
        for (int i = 0; i < Nyas::Entities.Count; ++i)
        {
            auto *pbr_uniform_block = (PbrDataDesc*)Nyas::Shaders[G_Shaders.Pbr].UnitBlock;
            mat4_assign(pbr_uniform_block[i].Model, Nyas::Entities[i].Transform);
        }
        draw.Units->Shader = Nyas::Entities[0].Shader;
        draw.Units->Mesh = Nyas::Entities[0].Mesh;
        draw.Units->Instances = Nyas::Entities.Count;

        new_frame.Push(draw);
    }

    // Skybox
    {
        NyasDrawCmd draw;
        Nyas::Camera.OriginViewProj((float *)Nyas::Shaders[G_Shaders.Skybox].UnitBlock);
        draw.Shader = G_Shaders.Skybox;
        draw.State.DisableFlags |= NyasDrawFlags_FaceCulling;
        draw.State.Depth = NyasDepthFunc_LessEqual;
        draw.UnitCount = 1;
        draw.Units = (NyasDrawUnit *)NyFrameAllocator::Alloc(sizeof(NyasDrawUnit));
        draw.Units->Shader = G_Shaders.Skybox;
        draw.Units->Mesh = NYAS_CUBE;
        draw.Units->Instances = 1;
        new_frame.Push(draw);
    }

    // Frame texture
    {
        NyasDrawCmd draw;
        draw.Framebuf = NyasCode_Default;
        draw.Shader = G_Shaders.FullscreenImg;
        draw.State.ViewportMinX = 0;
        draw.State.ViewportMinY = 0;
        draw.State.ViewportMaxX = vp.X;
        draw.State.ViewportMaxY = vp.Y;
        draw.State.DisableFlags |= NyasDrawFlags_DepthTest;
        draw.UnitCount = 1;
        draw.Units = (NyasDrawUnit *)NyFrameAllocator::Alloc(sizeof(NyasDrawUnit));
        draw.Units->Shader = G_Shaders.FullscreenImg;
        draw.Units->Mesh = NYAS_QUAD;
        draw.Units->Instances = 1;
        new_frame.Push(draw);
    }
}

int main(int argc, char **argv)
{
    NY_UNUSED(argc), NY_UNUSED(argv);
    Nyas::InitIO("NYAS PBR Material Demo", 1920, 1080);
    Nyas::Camera.Init(*Nyas::GetCurrentCtx());
    Init();
    NyChrono frame_chrono;
    while (!Nyas::GetCurrentCtx()->Platform.WindowClosed)
    {
        // NewFrame
        float delta_time = NyChrono::Seconds((double)frame_chrono.Elapsed());
        frame_chrono.Restart();
        Nyas::GetCurrentCtx()->Platform.DeltaTime = delta_time;
        // Build
        NyArray<NyasDrawCmd, NyFrameAllocator> frame;
        BuildFrame(frame);

        // Render
        for (int i = 0; i < frame.Size; ++i)
        {
            Nyas::Draw(&frame[i]);
        }

        Nyas::WindowSwap();
    }

    return 0;
}
