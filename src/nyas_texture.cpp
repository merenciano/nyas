#include "nyas_types.h"
#include <glad/glad.h>
#include <vector>
#include <future>
#include <stdio.h>

#include "stb_image.h"

namespace azdo {
bool operator==(TexInfo lhs, TexInfo rhs)
{
	return !memcmp(&lhs, &rhs, sizeof(TexInfo));
}

struct _GL_Format
{
	GLint InternalFormat;
	GLenum Format;
	GLenum Type;
};

static inline _GL_Format _GL_TexFmt(NyasTexFmt Format)
{
	switch (Format)
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
		case NyasTexFmt_Depth: return { GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT };
       	case NyasTexFmt_DepthStencil: return { GL_DEPTH24_STENCIL8, GL_DEPTH_COMPONENT, GL_FLOAT };
		default: printf("Unrecognized texture format: (%d).", Format); return { 0, 0, 0 };
	}
}

static int _TexChannels(NyasTexFmt fmt)
{
	switch (fmt)
	{
		case NyasTexFmt_RGBA_16F:
		case NyasTexFmt_RGBA_8: return 4;
		case NyasTexFmt_RGB_16F:
		case NyasTexFmt_RGB_8:
		case NyasTexFmt_SRGB_8: return 3;
		case NyasTexFmt_RG_16F:
		case NyasTexFmt_RG_8: return 2;
		case NyasTexFmt_R_16F:
		case NyasTexFmt_R_8: return 1;
		default: return 0;
	}
}

void Textures::_Init()
{
	#if 0
	Data.resize(NYAS_TEX_ARRAYS);
	glCreateTextures(GL_TEXTURE_2D_ARRAY, NYAS_TEX_ARRAYS, Data.data());
	CubeData.resize(NYAS_CUBEMAP_ARRAYS);
	glCreateTextures(GL_TEXTURE_CUBE_MAP_ARRAY, NYAS_CUBEMAP_ARRAYS, CubeData.data());
	#endif
}

void Textures::_CreateTex()
{
	for (int i = 0; i < (int)Tex.size(); ++i)
	{
		if (Data[i] == 0x7FFFFFFF)
		{
			auto Format = _GL_TexFmt(Tex[i].Info.Format);
			glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &Data[i]);
			glTextureStorage3D(Data[i], Tex[i].Info.Levels, Format.InternalFormat, Tex[i].Info.Width, Tex[i].Info.Height, NYAS_TEX_ARRAY_SIZE);
			glBindTextureUnit(NYAS_CUBEMAP_ARRAYS + i, Data[i]);
		}
	}

	for (int i = 0; i < (int)Cubemap.size(); ++i)
	{
		if (CubeData[i] == 0x7FFFFFFF)
		{
			auto Format = _GL_TexFmt(Cubemap[i].Info.Format);
			glCreateTextures(GL_TEXTURE_CUBE_MAP_ARRAY, 1, &CubeData[i]);
			glTextureStorage3D(CubeData[i], Cubemap[i].Info.Levels, Format.InternalFormat, Cubemap[i].Info.Width, Cubemap[i].Info.Height, NYAS_CUBEMAP_ARRAY_SIZE * 6);
			glBindTextureUnit(i, CubeData[i]);
		}
	}
}

void Textures::_UpdateTex()
{
	for (auto& t : Updates)
	{
		if (t.first.Flags & NyasTexFlags_Cubemap)
		{
			auto Format = _GL_TexFmt(Cubemap[t.first.Index].Info.Format);
			int w = Cubemap[t.first.Index].Info.Width >> t.second.Level;
			int h = Cubemap[t.first.Index].Info.Height >> t.second.Level;
			for (int i = 0; i < 6; ++i)
			{
				glTextureSubImage3D(CubeData[t.first.Index], t.second.Level, 0, 0, t.first.Layer * 6 + i, w, h, 1, Format.Format, Format.Type, t.second.Data[i]);
			}
		}
		else
		{
			auto Format = _GL_TexFmt(Tex[t.first.Index].Info.Format);
			int w = Tex[t.first.Index].Info.Width >> t.second.Level;
			int h = Tex[t.first.Index].Info.Height >> t.second.Level;
			glTextureSubImage3D(Data[t.first.Index], t.second.Level, 0, 0, t.first.Layer, w, h, 1, Format.Format, Format.Type, t.second.Data[0]);
		}
	}
	Updates.clear();// TODO: Posible memleak
}

TexHandle Textures::Alloc(TexInfo info, NyasTexFlags flags)
{
	if (flags & NyasTexFlags_Cubemap)
	{
		for (int16_t i = 0; i < (int16_t)Cubemap.size(); ++i)
		{
			if ((Cubemap[i].Info == info) && (Cubemap[i].Count != NYAS_CUBEMAP_ARRAY_SIZE))
			{
				return { i, Cubemap[i].Count++, flags };
			}
		}

		Cubemap.emplace_back(info, 1);
		CubeData.emplace_back(0x7FFFFFFF);
		int16_t idx = (int16_t)Cubemap.size() - 1;
		return {idx, 0, flags};
	}
	else
	{
		for (int16_t i = 0; i < (int16_t)Tex.size(); ++i)
		{
			if (Tex[i].Info == info && Tex[i].Count != NYAS_TEX_ARRAY_SIZE)
			{
				return { i, Tex[i].Count++, flags };
			}
		}

		Tex.emplace_back(info, 1);
		Data.emplace_back(0x7FFFFFFF);
		int16_t idx = (int16_t)Tex.size() - 1;
		return {idx, 0, flags};
	}
}

TexHandle Textures::Load(const char *path, NyasTexFmt fmt, int levels)
{
	int w, h, channels;
	int ch = _TexChannels(fmt);
	azdo::TexImage img;
	img.Level = 0;
	if (fmt >= NyasTexFmt_BeginFloat)
	{
		img.Data[0] = stbi_loadf(path, &w, &h, &channels, ch);
	}
	else
	{
		img.Data[0] = stbi_load(path, &w, &h, &channels, ch);
	}

	if (!img.Data[0])
	{
		printf("The image '%s' couldn't be loaded", path);
	}

	TexHandle hnd = Alloc({fmt, w, h, levels});
	Update(hnd, img);
	return hnd;
}
#if 0
CubemapHandle Textures::LoadCubemap(const char *path[6], NyasTexFmt fmt, int levels)
{
	int w, h, channels = 0;
	int ch = _TexChannels(fmt);
	azdo::CubemapImage img;
	img.Level = 0;
	if (fmt >= NyasTexFmt_BeginFloat)
	{
		for (int i = 0; i < 6; ++i)
		{
			img.Data[i] = stbi_loadf(path[i], &w, &h, &channels, ch);
			if (!img.Data[0])
			{
				printf("The image '%s' couldn't be loaded", path[i]);
			}
		}
	}
	else
	{
		for (int i = 0; i < 6; ++i)
		{
			img.Data[i] = stbi_load(path[i], &w, &h, &channels, ch);
			if (!img.Data[0])
			{
				printf("The image '%s' couldn't be loaded", path[i]);
			}
		}
	}

	CubemapHandle hnd = CubeAlloc({fmt, w, h, levels});
	Update(hnd, img);
	return hnd;
}
#endif
void Textures::Update(TexHandle h, TexImage img)
{
	Updates.emplace_back(h, img);
}

void Textures::Sync()
{
	_CreateTex();
	_UpdateTex();
}

}// namespace azdo

