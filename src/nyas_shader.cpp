#include "nyas_types.h"
#include "nyas_render.h"

#include <stdio.h>

NyasShaderSrc &NyasShaderSrc::AddFile(const char *path)
{
    FILE *f = fopen(path, "rb");
    if (!f)
    {
        NYAS_LOG_ERR("File open failed for %s. Ignoring shader AddSource.", path);
        return *this;
    }

    do
    {
        constexpr int BUFSIZE = 1024;
        char         *buf     = (char *)NYAS_ALLOC(BUFSIZE);
        int           ret     = fread(buf, 1, BUFSIZE, f);
        SrcStr.emplace_back(buf);
        SrcLen.emplace_back(ret);
    } while (!feof(f));

    fclose(f);
    return *this;
}

NyasHandle NyPipelines::Load(int unif_size, NyasShaderStage *stages, const char **paths, int count)
{
    NyasHandle h = Alloc(unif_size);
    NyasPipelineBuilder pb = h;

    for (int i = 0; i < count; ++i)
    {
        pb.Source[stages[i]].AddFile(paths[i]);
    }

    Update(std::move(pb));
    return h;
}

NyasHandle NyPipelines::Load(int unif_size, const char *vert_path, const char *frag_path)
{
    NyasShaderStage stages[] = { NyasShaderStage_Vertex, NyasShaderStage_Fragment };
    const char *paths[] = {vert_path, frag_path};
    return Load(unif_size, stages, paths, 2);
}

void NyPipelines::Sync(NyasHandle shader_handle)
{
    for (auto &pb : Updates)
    {
        if (InternalIDs[pb.Pipeline] == NYAS_INVALID_RESOURCE_ID)
        {
            nyas::render::_NyCreatePipeline(&InternalIDs[pb.Pipeline], &Pipelines[pb.Pipeline]);
        }

        nyas::render::_NyBuildPipeline(InternalIDs[pb.Pipeline], &pb);
    }

    Updates.clear();
    nyas::render::_NyUsePipeline(InternalIDs[shader_handle], Pipelines[shader_handle]);
}
