#ifndef NYAS_TEXTURE_PACK_H
#define NYAS_TEXTURE_PACK_H

#include "nyas.h"
#include "renderer.h"
#include <queue>

struct NyasTexID
{
	int Index;
	float Layer;
};

template<size_t SIZE>
struct NyTextureContainer
{
	NyasTexFmt Format;
	int Width;
	int Height;
	int Levels;
	NyasResource Res;
	std::queue<float> FreeList;

	NyasTexArray(int width, int height, NyasTexFmt format, int levels)
		: Format(format)
		, Width(width)
		, Height(height)
		, Levels(levels)
	{
		FreeList.reserve(SIZE);
		float count = (float)SIZE;
		for (float i = 0; i < count; ++i)
		{
			FreeList.emplace(i);
		}
	}

	float Alloc()
	{
		NyasTexID tex = FreeList.front();
		FreeList.pop();
		return tex;
	}

	void Release(float layer)
	{
		NYAS_ASSERT(layer < (float)SIZE && "Texture layer out of range.");
		NYAS_ASSERT(layer >= 0.0f && "Texture layer has to be positive.");
		NYAS_ASSERT(FreeList.find(layer) == FreeList.end() && "Releasing a free layer.");
		FreeList.emplace(layer);
	}

	void _GPU_Create()
	{
		glGenTextures(1, &Res.Id);
	}

	void _GPU_Set(int layer, void *data, int level = 0)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(_GPU_Data._Type, _GPU_Data._Id);
		int w = Width >> level;
		int h = Height >> level;
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, level, 0, 0, layer, w, h, 1, _Format, _Type, data);
	}
};

using NyasTexArray = NyTextureContainer<64>;

struct NyasTexturePack
{
	std::vector<NyasTexArray> Textures;
	NyasTexID Create(int w, int h, NyasTexFmt fmt, int levels)
	{
		for (auto& Tex : Textures)
		{
			if (Tex.w)
			if (!Tex.FreeList.empty())
			{

			}
		}
	}
};

#endif // NYAS_TEXTURE_PACK_H