
#include "nyas.h"
#include <mathc.h>
#include <stdio.h>

#include <array>

NyTextures  GTextures;
NyPipelines GShaders;

struct PbrUnitData
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
    int   AlbedoIdx;
    float AlbedoLayer;
    int   NormalIdx;
    float NormalLayer;
    int   RoughnessIdx;
    float RoughnessLayer;
    int   MetallicIdx;
    float MetallicLayer;
};

struct PbrData
{
    float       ViewProj[16];
    NyVec3      CameraEye;
    float       _Padding;
    float       Sunlight[4]; // Alpha for light intensity
    int         LutIdx;
    float       LutLayer;
    int         IrrIdx;
    float       IrrLayer;
    int         PrefIdx;
    float       PrefLayer;
    float       _Padding2[2];
    PbrUnitData Entity[NYAS_PIPELINE_MAX_UNITS];
};

struct PbrMaps
{
    NyasTexture Alb, Nor, Rou, Met;
};

NyasHandle PbrShader;
NyasHandle SkyShader;
NyasHandle ImgShader;

struct
{
    NyasTexture Sky;
    PbrMaps     Pbr[9];
} G_Tex;

NyasHandle  G_Framebuf;
NyasTexture G_FbTex;
NyasHandle  G_Mesh;

void Init()
{
    PbrShader = GShaders.Load(sizeof(PbrData), "assets/shaders/pbr-vert.glsl", "assets/shaders/pbr-frag.glsl");
    SkyShader = GShaders.Load(20 * sizeof(float), "assets/shaders/skybox-vert.glsl", "assets/shaders/skybox-frag.glsl");
    ImgShader = GShaders.Load(16, "assets/shaders/fullscreen-img-vert.glsl", "assets/shaders/fullscreen-img-frag.glsl");

    #if 0
    {
        PbrShader = GShaders.Alloc(sizeof(PbrData));
        NyasShaderSrc vert_src;
        vert_src.AddFile("assets/shaders/pbr-vert.glsl");
        NyasShaderSrc frag_src;
        frag_src.AddFile("assets/shaders/pbr-frag.glsl");
        GShaders.Update(PbrShader, NyasShaderStage_Vertex, std::move(vert_src));
        GShaders.Update(PbrShader, NyasShaderStage_Fragment, std::move(frag_src));
    }
    {
        SkyShader = GShaders.Alloc(20 * sizeof(float));
        NyasShaderSrc vert_src;
        vert_src.AddFile("assets/shaders/skybox-vert.glsl");
        NyasShaderSrc frag_src;
        frag_src.AddFile("assets/shaders/skybox-frag.glsl");
        GShaders.Update(SkyShader, NyasShaderStage_Vertex, std::move(vert_src));
        GShaders.Update(SkyShader, NyasShaderStage_Fragment, std::move(frag_src));
    }
    {
        ImgShader = GShaders.Alloc(16);
        NyasShaderSrc vert_src;
        vert_src.AddFile("assets/shaders/fullscreen-img-vert.glsl");
        NyasShaderSrc frag_src;
        frag_src.AddFile("assets/shaders/fullscreen-img-frag.glsl");
        GShaders.Update(ImgShader, NyasShaderStage_Vertex, std::move(vert_src));
        GShaders.Update(ImgShader, NyasShaderStage_Fragment, std::move(frag_src));
    }
#endif
    G_Mesh = Nyas::LoadMesh("assets/obj/matball.msh");

    NyasTexture irradiance;
    NyasTexture prefilter;
    NyasTexture lut;
    NyUtil::LoadEnv("assets/env/canyon.env", &lut, &G_Tex.Sky, &irradiance, &prefilter);
    NyUtil::LoadBasicGeometries();

    std::array<const char *, 36> texpaths { "assets/tex/celtic-gold/celtic-gold_A.png",
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

    for (int i = 0; i < 9; ++i)
    {
        G_Tex.Pbr[i].Alb = GTextures.Load(texpaths[i * 4], NyasTexFmt_SRGB_8, 1);
        G_Tex.Pbr[i].Nor = GTextures.Load(texpaths[i * 4 + 1], NyasTexFmt_RGB_8, 1);
        G_Tex.Pbr[i].Rou = GTextures.Load(texpaths[i * 4 + 2], NyasTexFmt_R_8, 1);
        G_Tex.Pbr[i].Met = GTextures.Load(texpaths[i * 4 + 3], NyasTexFmt_R_8, 1);
    }

    auto *pbr_data        = (PbrData *)GShaders.Pipelines[PbrShader].Data;
    pbr_data->Sunlight[0] = 0.0f;
    pbr_data->Sunlight[1] = -1.0f;
    pbr_data->Sunlight[2] = -0.1f;
    pbr_data->Sunlight[3] = 0.0f;
    pbr_data->LutIdx      = lut.Index;
    pbr_data->LutLayer    = (float)lut.Layer;
    pbr_data->IrrIdx      = irradiance.Index;
    pbr_data->IrrLayer    = (float)irradiance.Layer;
    pbr_data->PrefIdx     = prefilter.Index;
    pbr_data->PrefLayer   = (float)prefilter.Layer;

    G_Framebuf           = Nyas::CreateFramebuffer();
    NyVec2i vp           = Nyas::GetCurrentCtx()->Platform.WindowSize;
    G_FbTex              = GTextures.Alloc({ NyasTexFmt_RGB_32F, vp.X, vp.Y, 1 });
    NyasTexture fb_depth = GTextures.Alloc({ NyasTexFmt_Depth, vp.X, vp.Y, 1 });

    NyasTexTarget color = { G_FbTex, NyasTexFace_2D, NyasFbAttach_Color, 0 };
    NyasTexTarget depth = { fb_depth, NyasTexFace_2D, NyasFbAttach_Depth, 0 };
    Nyas::SetFramebufferTarget(G_Framebuf, 0, color);
    Nyas::SetFramebufferTarget(G_Framebuf, 1, depth);

    for (int i = 0; i < 9; ++i)
    {
        auto &e = Nyas::Entities[Nyas::Entities.Add()];
        memcpy(e.Transform,
            smat4_translation(smat4_identity(), { (i % 3) * 2.0f - 2.0f, 0.0f, (i / 3) * -2.0f }).v,
            16 * sizeof(float));
        e.Mesh   = G_Mesh;
        e.Shader = PbrShader;

        auto &mat           = pbr_data->Entity[i];
        mat.Color[0]        = 1.0f;
        mat.Color[1]        = 1.0f;
        mat.Color[2]        = 1.0f;
        mat.TilingX         = 2.0f;
        mat.TilingY         = 2.0f;
        mat.Reflectance     = 0.5f;
        mat.UseAlbedo       = 1.0f;
        mat.UseRougAndMetal = 1.0f;
        mat.Metallic        = 0.5f;
        mat.Roughness       = 0.5f;
        mat.UseNormalMap    = 1.0f;

        mat.AlbedoIdx      = G_Tex.Pbr[i].Alb.Index;
        mat.AlbedoLayer    = G_Tex.Pbr[i].Alb.Layer;
        mat.NormalIdx      = G_Tex.Pbr[i].Nor.Index;
        mat.NormalLayer    = G_Tex.Pbr[i].Nor.Layer;
        mat.RoughnessIdx   = G_Tex.Pbr[i].Rou.Index;
        mat.RoughnessLayer = G_Tex.Pbr[i].Rou.Layer;
        mat.MetallicIdx    = G_Tex.Pbr[i].Met.Index;
        mat.MetallicLayer  = G_Tex.Pbr[i].Met.Layer;
    }

    ((int *)GShaders.Pipelines[SkyShader].Data)[16]    = G_Tex.Sky.Index;
    GShaders.Pipelines[SkyShader].Data[17]             = G_Tex.Sky.Layer;
    *(NyasTexture *)GShaders.Pipelines[ImgShader].Data = G_FbTex;
}

void BuildFrame(NyArray<NyasDrawCmd, NyCircularAllocator<NY_MEGABYTES(16)>> &new_frame)
{
    Nyas::PollIO();
    NyVec2i vp = Nyas::GetCurrentCtx()->Platform.WindowSize;
    Nyas::Camera.Navigate();

    /* PBR common shader data. */
    auto *pbr_shared_block = (PbrData *)GShaders.Pipelines[PbrShader].Data;
    mat4_multiply(pbr_shared_block->ViewProj, Nyas::Camera.Proj, Nyas::Camera.View);
    pbr_shared_block->CameraEye = Nyas::Camera.Eye();

    // Scene entities
    {
        NyasDrawCmd draw;
        draw.Framebuf           = G_Framebuf;
        draw.Shader             = PbrShader;
        draw.State.BgColorR     = 0.2f;
        draw.State.BgColorG     = 0.2f;
        draw.State.BgColorB     = 0.2f;
        draw.State.BgColorA     = 1.0f;
        draw.State.ViewportMinX = 0;
        draw.State.ViewportMinY = 0;
        draw.State.ViewportMaxX = vp.X;
        draw.State.ViewportMaxY = vp.Y;
        draw.State.EnableFlags |= NyasDrawFlags_ColorClear;
        draw.State.EnableFlags |= NyasDrawFlags_DepthClear;
        draw.State.EnableFlags |= NyasDrawFlags_Blend;
        draw.State.EnableFlags |= NyasDrawFlags_DepthTest;
        draw.State.EnableFlags |= NyasDrawFlags_DepthWrite;
        draw.State.BlendSrc    = NyasBlendFunc_One;
        draw.State.BlendDst    = NyasBlendFunc_Zero;
        draw.State.Depth       = NyasDepthFunc_Less;
        draw.State.FaceCulling = NyasFaceCull_Back;

        draw.UnitCount = 1;
        draw.Units     = (NyasDrawUnit *)NyFrameAllocator::Alloc(1 * sizeof(NyasDrawUnit));
        for (int i = 0; i < Nyas::Entities.Count; ++i)
        {
            auto *pbr_uniform_block =
                (PbrUnitData *)(((PbrData *)GShaders.Pipelines[PbrShader].Data)->Entity);
            mat4_assign(pbr_uniform_block[i].Model, Nyas::Entities[i].Transform);
        }
        draw.Units->Shader    = Nyas::Entities[0].Shader;
        draw.Units->Mesh      = Nyas::Entities[0].Mesh;
        draw.Units->Instances = Nyas::Entities.Count;

        new_frame.Push(draw);
    }

    // Skybox
    {
        NyasDrawCmd draw;
        Nyas::Camera.OriginViewProj(GShaders.Pipelines[SkyShader].Data);
        ((int *)GShaders.Pipelines[SkyShader].Data)[16] = G_Tex.Sky.Index;
        GShaders.Pipelines[SkyShader].Data[17]          = G_Tex.Sky.Layer;
        draw.Shader                                     = SkyShader;
        draw.State.DisableFlags |= NyasDrawFlags_FaceCulling;
        draw.State.Depth      = NyasDepthFunc_LessEqual;
        draw.UnitCount        = 1;
        draw.Units            = (NyasDrawUnit *)NyFrameAllocator::Alloc(sizeof(NyasDrawUnit));
        draw.Units->Shader    = SkyShader;
        draw.Units->Mesh      = NYAS_CUBE;
        draw.Units->Instances = 1;
        new_frame.Push(draw);
    }

    // Frame texture
    {
        NyasDrawCmd draw;
        draw.Framebuf           = NyasCode_Default;
        draw.Shader             = ImgShader;
        draw.State.ViewportMinX = 0;
        draw.State.ViewportMinY = 0;
        draw.State.ViewportMaxX = vp.X;
        draw.State.ViewportMaxY = vp.Y;
        draw.State.DisableFlags |= NyasDrawFlags_DepthTest;
        draw.UnitCount        = 1;
        draw.Units            = (NyasDrawUnit *)NyFrameAllocator::Alloc(sizeof(NyasDrawUnit));
        draw.Units->Shader    = ImgShader;
        draw.Units->Mesh      = NYAS_QUAD;
        draw.Units->Instances = 1;
        new_frame.Push(draw);
    }
}

int main(int argc, char **argv)
{
    NY_UNUSED(argc), NY_UNUSED(argv);
    Nyas::InitIO("NYAS PBR Material Demo", 2000, 1200);
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
