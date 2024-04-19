#include <vector>
#include <stdio.h>

#define TEXARR_MAX 64

typedef int NyasTexFmt;

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

struct Resource
{
    unsigned int id;
};

typedef unsigned int ResourceID;

struct TexInfo
{
    NyasTexFmt fmt = NyasTexFmt_RGB_8;
    int width = 0;
    int height = 0;
    int levels = 0;
};

struct TexImage
{
	void *data;
	int level;
};

struct TexArr
{
    TexInfo info;
    int count = 0;

	TexArr(TexInfo ainfo, int acount) : info(ainfo), count(acount) {};
};

struct TexHandle
{
    int index;
    int layer;
};

struct Textures
{
    TexHandle Alloc(TexInfo info);
	void Update(TexHandle h, TexImage img);

    std::vector<TexArr> tex;
    std::vector<ResourceID> data;
	std::vector<std::pair<TexHandle, TexImage>> updates;
};

static bool operator==(TexInfo lhs, TexInfo rhs)
{
    return lhs.fmt == rhs.fmt && lhs.width == rhs.width && lhs.height == rhs.height &&
           lhs.levels == rhs.levels;
}

TexHandle Textures::Alloc(TexInfo info)
{
    for (int i = 0; i < tex.size(); ++i)
    {
        if (tex[i].info == info && tex[i].count != TEXARR_MAX)
        {
            return { i, tex[i].count++ };
        }
    }

    tex.emplace_back(info, 1);
	data.emplace_back(-1);
    return {(int)tex.size() - 1, 0};
}

void Textures::Update(TexHandle h, TexImage img)
{
	updates.emplace_back(h, img);
}

#include <glad/glad.h>

struct _GL_Format
{
	GLint internal_fmt;
	GLenum fmt;
	GLenum type;
};

static _GL_Format _GL_TexFmt(NyasTexFmt fmt)
{
    switch (fmt)
    {
        case NyasTexFmt_R_8: return { GL_R8, GL_RED, GL_UNSIGNED_BYTE };
        case NyasTexFmt_RG_8: return { GL_RG8, GL_RG, GL_UNSIGNED_BYTE };
        case NyasTexFmt_RGB_8: return { GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE };
        case NyasTexFmt_RGBA_8: return { GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE };
        case NyasTexFmt_SRGB_8: return { GL_SRGB8, GL_RGB, GL_UNSIGNED_BYTE };
        case NyasTexFmt_R_16F: return { GL_R16F, GL_RED, GL_HALF_FLOAT };
        case NyasTexFmt_RG_16F: return { GL_RG16F, GL_RG, GL_HALF_FLOAT };
        case NyasTexFmt_RGB_16F: return { GL_RGB16F, GL_RGB, GL_HALF_FLOAT };
        case NyasTexFmt_RGBA_16F: return { GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT };
        case NyasTexFmt_RGB_32F: return { GL_RGB32F, GL_RGB, GL_FLOAT };
        case NyasTexFmt_Depth: return { GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT };
        default: printf("Unrecognized texture format: (%d).", fmt); return { 0, 0, 0 };
    }
}

void _CreateTex(Textures *t)
{
	for (int i = 0; i < t->tex.size(); ++i)
	{
		if (t->data[i] == -1)
		{
			auto fmt = _GL_TexFmt(t->tex[i].info.fmt);
			glGenTextures(1, &t->data[i]);
			glBindTexture(GL_TEXTURE_2D_ARRAY, t->data[i]);
			glTexStorage3D(GL_TEXTURE_2D_ARRAY, t->tex[i].info.levels, fmt.internal_fmt, t->tex[i].info.width, t->tex[i].info.height, TEXARR_MAX);
		}
	}
}

void _UpdateTex(Textures *t)
{
	for (auto& tex : t->updates)
	{
		auto fmt = _GL_TexFmt(t->tex[tex.first.index].info.fmt);
		int w = t->tex[tex.first.index].info.width >> tex.second.level;
        int h = t->tex[tex.first.index].info.height >> tex.second.level;
		glTextureSubImage3D(t->data[tex.first.index], tex.second.level, 0, 0, tex.first.layer, w, h, 1, fmt.fmt, fmt.type, tex.second.data);
	}
	t->updates.clear();// TODO: Posible memleak
}
