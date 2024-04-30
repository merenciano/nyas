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
    NyasTexFmt_COUNT,


    NyasTexFmt_BeginByte = NyasTexFmt_R_8,
    NyasTexFmt_BeginFloat16 = NyasTexFmt_R_16F,
    NyasTexFmt_BeginFloat32 = NyasTexFmt_R_32F,
    NyasTexFmt_BeginFloat = NyasTexFmt_BeginFloat16,
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
    NyasTexFlags_FlipVerticallyOnLoad = 1 << 1,
	NyasTexFlags_Cubemap = 1 << 2,
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
    NyasTexWrap WrapS = NyasTexWrap_Mirror;
    NyasTexWrap WrapT = NyasTexWrap_Mirror;
    NyasTexWrap WrapR = NyasTexWrap_Mirror;
    float BorderColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
};

struct NyasTexDesc
{
	NyasTexFmt Format = NyasTexFmt_SRGB_8;
    int Width = 0;
    int Height = 0;
	int Levels = 0;

    int Count = 1; // TexArrays
    NyasTexFlags Flags = NyasTexFlags_None;
    NyasTexType Type = NyasTexType_2D;
    
	NyasTexDesc() = default;
    NyasTexDesc(NyasTexType type, NyasTexFmt fmt, int w, int h, int count = 1) :
        Format(fmt), Width(w), Height(h), Count(count), Flags(NyasTexFlags_None), Type(type)
    {}
};

typedef struct NyasResource
{
    uint32_t Id = 0x7FFFFFFF;
    NyasResourceFlags Flags = NyasResourceFlags_Created;
} NyasResource;

typedef struct NyasShaderDesc
{
    const char *Name;
    int UniformSize;

    NyasShaderDesc(const char *id, int uniform_size = 0) :
        Name(id), UniformSize(uniform_size)
    {
    }
} NyasShaderDesc;

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

struct NyasShader
{
    NyasResource Resource;
    NyasResource ResUnif;
    const char *Name;
    int TexArrLocation;
	int CubemapArrLocation;
    void *UniformData;
    int UniformSize;

    NyasShader(NyasShaderDesc *desc) : Name(desc->Name), UniformSize(desc->UniformSize)
    {
        ResUnif.Flags = UniformSize ? NyasResourceFlags_Dirty : NyasResourceFlags_Unused;
        UniformData = NYAS_ALLOC(UniformSize);
    }

    ~NyasShader()
    {
        NYAS_FREE(UniformData);
        UniformData = NULL;
    }
};

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

// NEW TYPES
#include <vector>
namespace azdo
{
	typedef unsigned int ResourceID;

	struct TexInfo
	{
		NyasTexFmt Format = NyasTexFmt_RGB_8;
		int Width = 0;
		int Height = 0;
		int Levels = 0;

		TexInfo(NyasTexFmt fmt, int w, int h, int lvls) : Format(fmt), Width(w), Height(h), Levels(lvls) {}
		bool operator==(TexInfo rhs)
		{
			return (Format == rhs.Format) && (Width == rhs.Width) && (Height == rhs.Height) &&
				   (Levels == rhs.Levels);
		}
	};

	struct TexImage
	{
		void *Data[6] = {NULL, NULL, NULL, NULL, NULL, NULL};
		int Level = 0;
	};

	struct TexArr
	{
		TexInfo Info;
		int16_t Count = 0;

		TexArr(TexInfo info, int count = 0) : Info(info), Count(count) {};
	};

	struct TexHandle
	{
		int16_t Index = -1;
		int16_t Layer = -1;
		NyasTexFlags Flags = NyasTexFlags_None;
	};

	struct Textures
	{
		void _Init();
		void _CreateTex();
		void _UpdateTex();

		TexHandle Alloc(TexInfo info, NyasTexFlags flags = NyasTexFlags_None);
		TexHandle Load(const char *path, NyasTexFmt fmt, int levels);
		//CubemapHandle LoadCubemap(const char *path[6], NyasTexFmt fmt, int levels);
		void Update(TexHandle h, TexImage img);
		void Sync();

		std::vector<TexArr> Tex;
		std::vector<ResourceID> Data;
		std::vector<std::pair<TexHandle, TexImage>> Updates;

		std::vector<TexArr> Cubemap;
		std::vector<ResourceID> CubeData;
		//std::vector<std::pair<CubemapHandle, CubemapImage>> CubemapUpdates;
	};
}

struct NyasTexTarget
{
    azdo::TexHandle Tex;
    NyasTexFace Face;
    NyasFbAttach Attach;
    int MipLevel;
};

struct NyasFramebuffer
{
    NyasResource Resource;
    NyasTexTarget Target[8];
};

#endif // NYAS_TYPES_H
