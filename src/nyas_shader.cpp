#include "nyas_render.h"
#include "nyas_types.h"

#include <stdio.h>

NyasShaderSrc &NyasShaderSrc::AddFile(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        NYAS_LOG_ERR("File open failed for %s. Ignoring shader AddSource.", path);
        return *this;
    }

    do {
        constexpr int BUFSIZE = 1024;
        char *buf = (char *)NYAS_ALLOC(BUFSIZE);
        int ret = fread(buf, 1, BUFSIZE, f);
        SrcStr.emplace_back(buf);
        SrcLen.emplace_back(ret);
    } while (!feof(f));

    fclose(f);
    return *this;
}

NyasHandle NyPipelines::Load(
    int unif_size, NyasShaderStage *stages, const char **paths, int count,
    NyasVtxAttribFlags attribs) {
    NyasHandle h = Alloc(unif_size);
    Pipelines[h].Attribs = attribs;
    NyasPipelineBuilder pb = h;

    for (int i = 0; i < count; ++i) {
        pb.Source[stages[i]].AddFile(paths[i]);
    }

    Update(std::move(pb));
    return h;
}

NyasHandle NyPipelines::Load(
    int unif_size, const char *vert_path, const char *frag_path, NyasVtxAttribFlags attribs) {
    NyasShaderStage stages[] = {NyasShaderStage_Vertex, NyasShaderStage_Fragment};
    const char *paths[] = {vert_path, frag_path};
    return Load(unif_size, stages, paths, 2, attribs);
}

void NyPipelines::Sync(NyasHandle shader_handle) {
    for (auto &pb : Updates) {
        if (InternalIDs[pb.Pipeline] == NYAS_INVALID_RESOURCE_ID) {
            nyas::render::_NyCreatePipeline(&InternalIDs[pb.Pipeline], &Pipelines[pb.Pipeline]);
        }

        nyas::render::_NyBuildPipeline(InternalIDs[pb.Pipeline], &pb);
    }

    Updates.clear();
    nyas::render::_NyUsePipeline(InternalIDs[shader_handle], &Pipelines[shader_handle]);
}

NyasHandle NyFramebuffers::Alloc(int target_count) {
    NyasHandle ret = Fb.size();
    Fb.emplace_back();
    Fb[ret].TargetCount = target_count;
    Fb[ret].Resource.ID = 0;
    Fb[ret].Resource.Flags = NyasResourceFlags_Dirty;
    return ret;
}

void NyFramebuffers::Update(NyasHandle handle, NyasTexTarget target, int index) {
    NYAS_ASSERT(index >= 0);
    NYAS_ASSERT(index < Fb[handle].TargetCount);
    Fb[handle].Target[index] = target;
    Fb[handle].Resource.Flags = NyasResourceFlags_Dirty;
}

void NyFramebuffers::Sync(NyasHandle handle) {
    if (handle == NyasCode_Default) {
        nyas::render::_NyUseFramebuf(0);
        return;
    }

    if (!(Fb[handle].Resource.Flags & NyasResourceFlags_Created)) {
        nyas::render::_NyCreateFramebuf(&Fb[handle]);
        Fb[handle].Resource.Flags |= NyasResourceFlags_Created;
    }

    nyas::render::_NyUseFramebuf(Fb[handle].Resource.ID);
    if (Fb[handle].Resource.Flags & NyasResourceFlags_Dirty) {
        for (int i = 0; i < Fb[handle].TargetCount; ++i) {
            nyas::render::_NySetFramebuf(Fb[handle].Resource.ID, &Fb[handle].Target[i]);
        }
        Fb[handle].Resource.Flags &= ~NyasResourceFlags_Dirty;
    }
}
