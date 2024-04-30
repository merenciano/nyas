#include "nyas_render.h"
#include "nyas_types.h"

#include <future>
#include <stdio.h>
#include <vector>

#include <stb_image.h>

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

NyasTexture NyTextures::Alloc(NyasTexInfo info, NyasTexFlags flags)
{
    if (flags & NyasTexFlags_Cubemap)
    {
        for (int16_t i = 0; i < NYAS_CUBEMAP_ARRAYS; ++i)
        {
            if (Cubemaps[i].Info == info && Cubemaps[i].Count != NYAS_CUBEMAP_ARRAY_SIZE)
            {
                return { i, Cubemaps[i].Count++, flags };
            }

            if (!Cubemaps[i].Count)
            {
                NYAS_ASSERT(Cubemaps[i].Info == NyasTexInfo() &&
                            "This texture array should be default-initialized.");
                Cubemaps[i].Info = info;
                Cubemaps[i].Count = 1;
                Updates.emplace_back(NyasTexture({ i, 0, flags }), NyasTexImage(NULL));
                return { i, 0, flags };
            }
        }
        NYAS_ASSERT(
            false &&
            "Out of cubemap memory. Consider increasing NYAS_CUBEMAP_ARRAYS or NYAS_CUBEMAP_ARRAY_SIZE.");
    }
    else
    {
        for (int16_t i = 0; i < NYAS_TEX_ARRAYS; ++i)
        {
            if (Textures[i].Info == info && Textures[i].Count != NYAS_TEX_ARRAY_SIZE)
            {
                return { i, Textures[i].Count++, flags };
            }

            if (!Textures[i].Count)
            {
                NYAS_ASSERT(Textures[i].Info == NyasTexInfo() &&
                            "This texture array should be default-initialized.");
                Textures[i].Info = info;
                Textures[i].Count = 1;
                Updates.emplace_back(NyasTexture({ i, 0, flags }), NyasTexImage(NULL));
                return { i, 0, flags };
            }
        }
        NYAS_ASSERT(
            false &&
            "Out of tex memory. Consider increasing NYAS_TEX_ARRAYS or NYAS_TEX_ARRAY_SIZE.");
    }
}

NyasTexture NyTextures::Load(const char *path, NyasTexFmt fmt, int levels)
{
    int w, h, channels;
    int ch = _TexChannels(fmt);
    NyasTexImage img;
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

    NyasTexture hnd = Alloc({ fmt, w, h, levels });
    Update(hnd, img);
    return hnd;
}
#if 0
CubemapHandle Textures::LoadCubemap(const char *path[6], NyasTexFmt fmt, int levels)
{
	int w, h, channels = 0;
	int ch = _TexChannels(fmt);
	CubemapImage img;
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
void NyTextures::Update(NyasTexture h, NyasTexImage img)
{
    Updates.emplace_back(h, img);
}

void NyTextures::Sync()
{
    if (_TextureIDs[0] == (NyResourceID)-1)
    {
        nyas::render::_NyCreateTex(NyasTexFlags_Cubemap, 0, _CubemapIDs, NYAS_CUBEMAP_ARRAYS);
    	nyas::render::_NyCreateTex(0, NYAS_CUBEMAP_ARRAYS, _TextureIDs, NYAS_TEX_ARRAYS);
    }

    for (const auto &[tex, img] : Updates)
    {
        if (tex.Flags & NyasTexFlags_Cubemap)
        {
            if (img.Data[0])
            {
                nyas::render::_NySetTex(
                    _CubemapIDs[tex.Index], tex, img, Cubemaps[tex.Index].Info, 6);
            }
            else
            {
                // If Data[0] is NULL: this entry was added from Alloc(..)
                // so tex storage init is expected.
                nyas::render::_NyAllocTex(
                    _CubemapIDs[tex.Index], Cubemaps[tex.Index].Info, NYAS_CUBEMAP_ARRAY_SIZE * 6);
            }
        }
        else
        {
            if (img.Data[0])
            {
                nyas::render::_NySetTex(
                    _TextureIDs[tex.Index], tex, img, Textures[tex.Index].Info, 1);
            }
            else
            {
                // If Data[0] is NULL: this entry was added from Alloc(..)
                // so tex storage init is expected.
                nyas::render::_NyAllocTex(
                    _TextureIDs[tex.Index], Textures[tex.Index].Info, NYAS_TEX_ARRAY_SIZE);
            }
        }
    }
    Updates.clear();
}
