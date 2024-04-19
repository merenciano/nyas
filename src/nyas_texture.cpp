#include "nyas_types.h"
#include <glad/glad.h>
#include <vector>
#include <future>
#include <stdio.h>

#include "stb_image.h"

#define TEXARR_MAX 64

namespace azdo {
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
		return Format == rhs.Format && Width == rhs.Width && Height == rhs.Height &&
			Levels == rhs.Levels;
	}
};

struct TexImage
{
	void *Data;
	int Level;
};

struct TexArr
{
    TexInfo Info;
    int Count = 0;

	TexArr(TexInfo ainfo, int acount) : Info(ainfo), Count(acount) {};
};

struct TexHandle
{
    int Index;
    int Layer;
};

struct Textures
{
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
			case NyasTexFmt_Depth: return { GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT };
			default: printf("Unrecognized texture format: (%d).", Format); return { 0, 0, 0 };
		}
	}

	void inline _CreateTex()
	{
		for (int i = 0; i < Tex.size(); ++i)
		{
			if (Data[i] == -1)
			{
				auto Format = _GL_TexFmt(Tex[i].Info.Format);
				glGenTextures(1, &Data[i]);
				glBindTexture(GL_TEXTURE_2D_ARRAY, Data[i]);
				glTexStorage3D(GL_TEXTURE_2D_ARRAY, Tex[i].Info.Levels, Format.InternalFormat, Tex[i].Info.Width, Tex[i].Info.Height, TEXARR_MAX);
			}
		}
	}

	void _UpdateTex()
	{
		for (auto& t : Updates)
		{
			auto Format = _GL_TexFmt(Tex[t.first.Index].Info.Format);
			int w = Tex[t.first.Index].Info.Width >> t.second.Level;
			int h = Tex[t.first.Index].Info.Height >> t.second.Level;
			glTextureSubImage3D(Data[t.first.Index], t.second.Level, 0, 0, t.first.Layer, w, h, 1, Format.Format, Format.Type, t.second.Data);
		}
		Updates.clear();// TODO: Posible memleak
	}

	TexHandle Alloc(TexInfo Info)
	{
		for (int i = 0; i < Tex.size(); ++i)
		{
			if (Tex[i].Info == Info && Tex[i].Count != TEXARR_MAX)
			{
				return { i, Tex[i].Count++ };
			}
		}

		Tex.emplace_back(Info, 1);
		Data.emplace_back(-1);
		return {(int)Tex.size() - 1, 0};
	}

	TexHandle Load(const char *path, NyasTexFmt fmt, int levels)
	{
		int w, h, channels;
		azdo::TexImage img;
		if (fmt >= NyasTexFmt_BeginFloat)
		{
			img.Data = stbi_loadf(path, &w, &h, &channels, 0);
		}
		else
		{
			img.Data = stbi_load(path, &w, &h, &channels, 0);
		}

		if (!img.Data)
		{
			printf("The image '%s' couldn't be loaded", path);
		}
		
		TexHandle hnd = Alloc({fmt, w, h, levels});
		Update(hnd, img);
	}

	void Update(TexHandle h, TexImage img)
	{
		Updates.emplace_back(h, img);
	}

	void Sync()
	{
		_CreateTex();
		_UpdateTex();
		glBindTextures(1, Data.size(), Data.data());
	}

    std::vector<TexArr> Tex;
    std::vector<ResourceID> Data;
	std::vector<std::pair<TexHandle, TexImage>> Updates;
};
}// namespace azdo

