#ifndef NYAS_TYPES_H
#define NYAS_TYPES_H

#include "nyas_platform.h"

#include <stdint.h>
#include <string.h>

typedef int NyasHandle;

#ifndef NyDrawIdx
typedef unsigned short NyDrawIdx;
#endif

#ifndef NyResourceID
typedef unsigned int NyResourceID;
#endif

// Predeclarations

// Basic data types
struct NyasResource;
struct NyasTexDesc;
struct NyasTexTarget;
struct NyasTexImg;
struct NyasShaderDesc;
struct NyasTexture;
struct NyasMesh;
struct NyasShader;
struct NyasMaterial;
struct NyasFramebuffer;
struct NyasDrawState;
struct NyasDrawUnit;
struct NyasDrawCmd;

// Flags
typedef int NyasResourceFlags; // enum NyasResourceFlags_
typedef int NyasTexFlags; // enum NyasTexFlags_
typedef int NyasVtxAttribFlags; // enum NyasVtxAttribFlags_
typedef int NyasDrawFlags; // enum NyasDrawFlags_

// Enum
typedef int NyasTexType; // enum NyasTexType_
typedef int NyasTexFmt; // enum NyasTexFmt_
typedef int NyasTexFilter; // enum NyasTexFilter_
typedef int NyasTexWrap; // enum NyasTexWrap_
typedef int NyasTexFace; // enum NyasTexFace_
typedef int NyasVtxAttrib; // enum NyasVtxAttrib_
typedef int NyasFbAttach; // enum NyasFbAttach_
typedef int NyasBlendFunc; // enum NyasBlendFunc_
typedef int NyasDepthFunc; // enum NyasDepthFunc_
typedef int NyasFaceCull; // enum NyasFaceCull_

enum NyasTexType_
{
    NyasTexType_Default,
    NyasTexType_2D,
    NyasTexType_Array2D,
    NyasTexType_Cubemap,
    NyasTexType_ArrayCubemap,
    NyasTexType_COUNT
};

enum NyasTexFmt_
{
    NyasTexFmt_Default,
    NyasTexFmt_Depth,
    NyasTexFmt_Stencil,
    NyasTexFmt_DepthStencil,
    NyasTexFmt_R_8,
    NyasTexFmt_RG_8,
    NyasTexFmt_RGB_8,
    NyasTexFmt_RGBA_8,
    NyasTexFmt_SRGB_8,
    NyasTexFmt_R_16F,
    NyasTexFmt_RG_16F,
    NyasTexFmt_RGB_16F,
    NyasTexFmt_RGBA_16F,
    NyasTexFmt_R_32F,
    NyasTexFmt_RG_32F,
    NyasTexFmt_RGB_32F,
    NyasTexFmt_RGBA_32F,
    NyasTexFmt_COUNT
};

enum NyasTexFace_
{
    NyasTexFace_PosX,
    NyasTexFace_NegX,
    NyasTexFace_PosY,
    NyasTexFace_NegY,
    NyasTexFace_PosZ,
    NyasTexFace_NegZ,
    NyasTexFace_2D,
    NyasTexFace_Default,
    NyasTexFace_COUNT
};

enum NyasTexFilter_
{
    NyasTexFilter_Default,
    NyasTexFilter_Linear,
    NyasTexFilter_LinearMipmapLinear,
    NyasTexFilter_LinearMipmapNearest,
    NyasTexFilter_Nearest,
    NyasTexFilter_NearestMipmapLinear,
    NyasTexFilter_NearestMipmapNearest,
    NyasTexFilter_COUNT
};

enum NyasTexWrap_
{
    NyasTexWrap_Default,
    NyasTexWrap_Clamp,
    NyasTexWrap_Repeat,
    NyasTexWrap_Mirror,
    NyasTexWrap_White,
    NyasTexWrap_Black,
    NyasTexWrap_COUNT
};

enum NyasFbAttach_
{
    NyasFbAttach_DepthStencil = -3,
    NyasFbAttach_Stencil = -2,
    NyasFbAttach_Depth = -1,
    NyasFbAttach_Color = 0,
    NyasFbAttach_Color1,
    NyasFbAttach_Color2,
    NyasFbAttach_Color3,
    NyasFbAttach_Color4,
    NyasFbAttach_Color5,
    NyasFbAttach_Color6
};

enum NyasVtxAttrib_
{
    NyasVtxAttrib_Pos,
    NyasVtxAttrib_Normal,
    NyasVtxAttrib_Tan,
    NyasVtxAttrib_Bitan,
    NyasVtxAttrib_UV,
    NyasVtxAttrib_Color,
    NyasVtxAttrib_COUNT
};

enum NyasBlendFunc_
{
    NyasBlendFunc_Default,
    NyasBlendFunc_One,
    NyasBlendFunc_SrcAlpha,
    NyasBlendFunc_OneMinusSrcAlpha,
    NyasBlendFunc_Zero,
    NyasBlendFunc_COUNT
};

enum NyasDepthFunc_
{
    NyasDepthFunc_Default,
    NyasDepthFunc_Less,
    NyasDepthFunc_LessEqual,
    NyasDepthFunc_COUNT
};

enum NyasFaceCull_
{
    NyasFaceCull_Default,
    NyasFaceCull_Front,
    NyasFaceCull_Back,
    NyasFaceCull_FrontAndBack,
    NyasFaceCull_COUNT
};

enum NyasTexFlags_
{
    NyasTexFlags_None = 0,
    NyasTexFlags_GenMipMaps = 1,
    NyasTexFlags_FlipVerticallyOnLoad = 1 << 1
};

enum NyasDrawFlags_
{
    NyasDrawFlags_None = 0,
    NyasDrawFlags_ColorClear = 1,
    NyasDrawFlags_DepthClear = 1 << 1,
    NyasDrawFlags_StencilClear = 1 << 2,
    NyasDrawFlags_DepthTest = 1 << 3,
    NyasDrawFlags_StencilTest = 1 << 4,
    NyasDrawFlags_DepthWrite = 1 << 5,
    NyasDrawFlags_StencilWrite = 1 << 6,
    NyasDrawFlags_Blend = 1 << 7,
    NyasDrawFlags_FaceCulling = 1 << 8,
    NyasDrawFlags_Scissor = 1 << 9,
};

enum NyasVtxAttribFlags_
{
    NyasVtxAttribFlags_None = 0,
    NyasVtxAttribFlags_Position = 1,
    NyasVtxAttribFlags_Normal = 1 << 1,
    NyasVtxAttribFlags_Tangent = 1 << 2,
    NyasVtxAttribFlags_Bitangent = 1 << 3,
    NyasVtxAttribFlags_UV = 1 << 4,
    NyasVtxAttribFlags_Color = 1 << 5,
};

enum NyasResourceFlags_
{
    NyasResourceFlags_None = 0,
    NyasResourceFlags_Dirty = 1 << 3,
    NyasResourceFlags_Created = 1 << 4,
    NyasResourceFlags_ReleaseAppStorage = 1 << 5,
    NyasResourceFlags_Unused = 1 << 6,
    NyasResourceFlags_Mapped = 1 << 7,
};

struct NyasSampler
{
    NyasTexFilter MinFilter = NyasTexFilter_Linear;
    NyasTexFilter MagFilter = NyasTexFilter_Linear;
    NyasTexWrap WrapS = NyasTexWrap_Clamp;
    NyasTexWrap WrapT = NyasTexWrap_Clamp;
    NyasTexWrap WrapR = NyasTexWrap_Clamp;
    float BorderColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
};

typedef struct NyasTexDesc
{
    int Width = 0;
    int Height = 0;
	int Levels = 0;
    int Count = 1; // TexArrays
    NyasTexFlags Flags = NyasTexFlags_None;
    NyasTexType Type = NyasTexType_2D;
    NyasTexFmt Format = NyasTexFmt_SRGB_8;
    
	NyasTexDesc() = default;
    NyasTexDesc(NyasTexType type, NyasTexFmt fmt, int w, int h, int count = 1) :
        Width(w), Height(h), Count(count), Flags(NyasTexFlags_None), Type(type), Format(fmt)
    {}
} NyasTexDesc;

typedef struct NyasTexTarget
{
    NyasHandle Tex;
    NyasTexFace Face;
    NyasFbAttach Attach;
    int MipLevel;
} NyasTexTarget;

typedef struct NyasTexImg
{
    void *Pix;
    NyasTexFace Face;
    int MipLevel;
    int Index;
    NyasTexImg() : Pix(NULL), Face(NyasTexFace_2D), MipLevel(0), Index(0) {}
    NyasTexImg(int aIndex) : Pix(NULL), Face(NyasTexFace_2D), MipLevel(0), Index(aIndex) {}
} NyasTexImg;

typedef struct NyasResource
{
    uint32_t Id;
    NyasResourceFlags Flags;

    NyasResource() : Id(0), Flags(0) {}
} NyasResource;

typedef struct NyasShaderDesc
{
    const char *Name;
    int SharedTexCount;
    int SharedCubemapCount;
    int TexArrCount;
    int UnitSize;
    int SharedSize;

    NyasShaderDesc(const char *id, int stcount = 0, int scmcount = 0, int tacount = 0, int unitsz = 0, int sharedsz = 0) :
        Name(id), SharedTexCount(stcount), SharedCubemapCount(scmcount), TexArrCount(tacount), UnitSize(unitsz), SharedSize(sharedsz) 
    {
    }
} NyasShaderDesc;

typedef struct NyasTexture
{
	int Width;
	int Height;
	int Levels;
	NyasTexFmt Format;
	int Count;
	
    NyasResource Resource;
    NyasTexDesc Data;
	NyasSampler Sampler;
    NyArray<NyasTexImg> Img;

    NyasTexture() = default;
    NyasTexture(NyasTexFmt f, NyasTexType t, int w, int h, int count = 1) : Data(t, f, w, h, count) {}
    NyasTexture(NyasTexDesc desc) : Data(desc) {}
} NyasTexture;

typedef struct NyasMesh
{
    NyasResource Resource;
    NyasResource ResVtx; // vertex buffer resource
    NyasResource ResIdx; // index buffer resource
    float *Vtx;
    NyDrawIdx *Indices;
    int64_t ElementCount;
    uint32_t VtxSize;
    NyasVtxAttribFlags Attribs;
} NyasMesh;

typedef struct NyasShader
{
    NyasResource Resource;
    NyasResource ResUnif;
    NyasResource ResSharedUnif;
    const char *Name;
    int SharedTexLocation;
    int SharedCubemapLocation;
    int TexArrLocation;
    int SharedTexCount;
    int SharedCubemapCount;
    int TexArrCount;
    NyasHandle *Shared;
    void *UnitBlock;
    void *SharedBlock;
    int UnitSize;
    int SharedSize;
    NyasHandle *TexArrays;

    NyasShader()
    {
        memset((void*)&Resource, 0, sizeof(*this));
    }

    NyasShader(NyasShaderDesc *desc) : Name(desc->Name), SharedTexCount(desc->SharedTexCount), SharedCubemapCount(desc->SharedCubemapCount),
        TexArrCount(desc->TexArrCount), UnitSize(desc->UnitSize), SharedSize(desc->SharedSize)
    {
        Resource.Id = 0;
        Resource.Flags = NyasResourceFlags_Dirty;
        ResUnif.Id = 0;
        ResUnif.Flags = NyasResourceFlags_Dirty;
        ResSharedUnif.Id = 0;
        ResSharedUnif.Flags = NyasResourceFlags_Dirty;
        
        UnitBlock = NYAS_ALLOC(UnitSize);
        SharedBlock = NYAS_ALLOC(SharedSize);
        Shared = (NyasHandle*)NYAS_ALLOC((SharedTexCount + SharedCubemapCount) * sizeof(NyasHandle));
        TexArrays = (NyasHandle*)NYAS_ALLOC(TexArrCount * sizeof(NyasHandle));
    }

    ~NyasShader()
    {
        NYAS_FREE(UnitBlock);
        NYAS_FREE(SharedBlock);
        NYAS_FREE(Shared);
        NYAS_FREE(TexArrays);
        UnitBlock = NULL;
        SharedBlock = NULL;
        Shared = NULL;
        TexArrays = NULL;
    }
} NyasShader;

typedef struct NyasFramebuffer
{
    NyasResource Resource;
    NyasTexTarget Target[8];
} NyasFramebuffer;

typedef struct NyasDrawState
{
    NyasBlendFunc BlendSrc;
    NyasBlendFunc BlendDst;
    NyasDepthFunc Depth;
    NyasFaceCull FaceCulling;
    NyasDrawFlags EnableFlags;
    NyasDrawFlags DisableFlags;
    int ViewportMinX; // if (viewport_min_x == viewport_max_x): no-op
    int ViewportMinY;
    int ViewportMaxX;
    int ViewportMaxY;
    int ScissorMinX; // if (scissor_min_x == scissor_max_x): no-op
    int ScissorMinY;
    int ScissorMaxX;
    int ScissorMaxY;
    float BgColorR; // if (bg_color_a == -1.0f): no-op
    float BgColorG;
    float BgColorB;
    float BgColorA;

    NyasDrawState() :
        BlendSrc(NyasBlendFunc_Default), BlendDst(NyasBlendFunc_Default),
        Depth(NyasDepthFunc_Default), FaceCulling(NyasFaceCull_Default),
        EnableFlags(NyasDrawFlags_None), DisableFlags(NyasDrawFlags_None), ViewportMinX(0),
        ViewportMinY(0), ViewportMaxX(0), ViewportMaxY(0), ScissorMinX(0), ScissorMinY(0),
        ScissorMaxX(0), ScissorMaxY(0), BgColorR(0.0f), BgColorG(0.0f), BgColorB(0.0f),
        BgColorA(-1.0f)
    {
    }
} NyasDrawState;

typedef struct NyasDrawUnit
{
    NyasHandle Shader;
    NyasHandle Mesh;
    int Instances;

    NyasDrawUnit() : Instances(1) {}
} NyasDrawUnit;

typedef struct NyasDrawCmd
{
    NyasDrawState State;
    NyasDrawUnit *Units;
    int UnitCount;
    NyasHandle Framebuf;
    NyasHandle Shader;
    NyasDrawCmd() : Units(NULL), UnitCount(0), Framebuf(NyasCode_NoOp) {}
} NyasDrawCmd;

#endif // NYAS_TYPES_H