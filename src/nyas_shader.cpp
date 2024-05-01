#include "nyas_types.h"

#include <map>
#include <stdio.h>

#define NYAS_STR2(X) #X
#define NYAS_STR(X) NYAS_STR2(X)

// clang-format off
#define NYAS_SHADER_HEADER(_VERSION) \
		"#version " #_VERSION "\n"\
		"#define TEX_COUNT " NYAS_STR(NYAS_TEX_ARRAYS)"\n"\
		"#define CUBE_COUNT " NYAS_STR(NYAS_CUBEMAP_ARRAYS)"\n"\
		"layout(binding = 0) uniform samplerCubeArray u_cubemaps[CUBE_COUNT];\n"\
		"layout(binding = CUBE_COUNT) uniform sampler2DArray u_textures[TEX_COUNT];\n"
// clang-format on

typedef int NyasShaderStage; // enum NyasShaderStage_
enum NyasShaderStage_
{
    NyasShaderStage_Compute,
    NyasShaderStage_Vertex,
    NyasShaderStage_TessCtrl,
    NyasShaderStage_TessEval,
    NyasShaderStage_Geometry,
    NyasShaderStage_Fragment,
    NyasShaderStage_COUNT,
};

namespace azdo
{
const char Header[] = NYAS_SHADER_HEADER(450 core);

struct ShaderStageSrc
{
    char *AllocStringBlock(int len)
    {
        char *buf = (char *)NYAS_ALLOC(len);
        SrcStr.emplace_back(buf);
        SrcLen.emplace_back(len);
        return buf;
    }

    // Copies 'str' contents.
    ShaderStageSrc &Write(const char *str, int len)
    {
        strncpy(AllocStringBlock(len), str, len);
        return *this;
    }

    ShaderStageSrc &AddFile(const char *path)
    {
        FILE *f = fopen(path, "rb");
        if (!f)
        {
            NYAS_LOG_ERR("File open failed for %s. Ignoring shader AddSource.", path);
            return;
        }

        do
        {
            constexpr int BUFSIZE = 1024;
            char *buf = (char *)NYAS_ALLOC(BUFSIZE);
            int ret = fread(buf, 1, BUFSIZE, f);
            SrcStr.emplace_back(buf);
            SrcLen.emplace_back(ret);
        } while (!feof(f));

        fclose(f);
        return *this;
    }

    void Reset()
    {
        for (int i = 1; i < SrcStr.size(); ++i)
        {
            NYAS_FREE(SrcStr[i]);
        }

        SrcStr.resize(1);
        SrcLen.resize(1);
    }

    ShaderStageSrc() = default;
    ShaderStageSrc(const ShaderStageSrc &) = delete;
    ~ShaderStageSrc() { Reset(); }
    std::vector<char *> SrcStr { (char *)Header };
    std::vector<int> SrcLen { sizeof(Header) };
};

struct Pipeline
{
    NyasResource UnifBuffer;
    void *UnifData = NULL;
    int UnifSize = 0;

    Pipeline(int unif_size) : UnifSize(unif_size) { UnifData = NYAS_ALLOC(UnifSize); }
};

struct Shaders
{
    NyasHandle Alloc(int unif_size)
    {
        Shaders.emplace_back(unif_size);
        _ShaderIDs.emplace_back(NYAS_INVALID_RESOURCE_ID);
        return Shaders.size() - 1;
    }

    NyasHandle Load(int unif_size, NyasShaderStage *stages, const char **paths, int count)
    {
        NyasHandle h = Alloc(unif_size);
        std::map<NyasShaderStage, ShaderStageSrc> src_map;
        for (int i = 0; i < count; ++i)
        {
            src_map[stages[i]].AddFile(paths[i]);
        }

        for (auto &[st, src] : src_map)
        {
            Update(h, st, std::move(src));
        }
    }

    void Update(NyasHandle h, NyasShaderStage stage, ShaderStageSrc &&src)
    {
        Updates.emplace_back(h, stage, std::move(src));
    }

    void Sync(NyasHandle h);

    std::vector<Pipeline> Shaders;
    std::vector<NyResourceID> _ShaderIDs;
    std::vector<std::tuple<NyasHandle, NyasShaderStage, ShaderStageSrc>> Updates;
};
} // namespace azdo

#include <glad/glad.h>

GLenum _GL_ShaderType(NyasShaderStage stage)
{
    switch (stage)
    {
        case NyasShaderStage_Compute: return GL_COMPUTE_SHADER;
        case NyasShaderStage_Vertex: return GL_VERTEX_SHADER;
        case NyasShaderStage_TessCtrl: return GL_TESS_CONTROL_SHADER;
        case NyasShaderStage_TessEval: return GL_TESS_EVALUATION_SHADER;
        case NyasShaderStage_Geometry: return GL_GEOMETRY_SHADER;
        case NyasShaderStage_Fragment: return GL_FRAGMENT_SHADER;
        default: assert(false);
    }
}

void azdo::Shaders::Sync(NyasHandle shader_handle)
{
    if (!Updates.empty())
    {
        GLint err;
        GLchar out_log[2048];
        std::map<NyResourceID, NyResourceID[NyasShaderStage_COUNT]> shader_ids;

        for (auto &[h, stage, src] : Updates)
        {
            if (_ShaderIDs[h] == NYAS_INVALID_RESOURCE_ID)
            {
                _ShaderIDs[h] = glCreateProgram();
            }

            shader_ids[_ShaderIDs[h]][stage] = glCreateShader(_GL_ShaderType(stage));
            glShaderSource(shader_ids[_ShaderIDs[h]][stage], src.SrcStr.size(), src.SrcStr.data(),
                src.SrcLen.data());
            glCompileShader(shader_ids[_ShaderIDs[h]][stage]);
            glGetShaderiv(shader_ids[_ShaderIDs[h]][stage], GL_COMPILE_STATUS, &err);
            if (!err)
            {
                glGetShaderInfoLog(shader_ids[_ShaderIDs[h]][stage], 2048, NULL, out_log);
                NYAS_LOG_ERR("Stage(%d):\n%s\n", stage, out_log);
            }
            glAttachShader(_ShaderIDs[h], shader_ids[_ShaderIDs[h]][stage]);
        }

        Updates.clear();

        for (auto &[prog, shad] : shader_ids)
        {
            glLinkProgram(prog);
            glGetProgramiv(prog, GL_LINK_STATUS, &err);
            if (!err)
            {
                glGetProgramInfoLog(prog, 2048, NULL, out_log);
                NYAS_LOG_ERR("Program link error:\n%s\n", out_log);
            }

            for (int i = 0; i < NyasShaderStage_COUNT; ++i)
            {
                if (shad[i] != NYAS_INVALID_RESOURCE_ID)
                {
                    glDetachShader(prog, shad[i]);
                    glDeleteShader(shad[i]);
                }
            }

            for (int i = 0; i < _ShaderIDs.size(); ++i)
            {
                if (_ShaderIDs[i] == prog && Shaders[i].UnifSize)
                {
                    glGenBuffers(1, &Shaders[i].UnifBuffer.ID);
                    glBindBuffer(GL_UNIFORM_BUFFER, Shaders[i].UnifBuffer.ID);
                    glBufferData(GL_UNIFORM_BUFFER, Shaders[i].UnifSize, Shaders[i].UnifData,
                        GL_DYNAMIC_DRAW);
                }
            }
        }
    }

    glUseProgram(_ShaderIDs[shader_handle]);
    if (Shaders[shader_handle].UnifSize)
    {
        glBindBuffer(GL_UNIFORM_BUFFER, Shaders[shader_handle].UnifBuffer.ID);
        glBufferData(GL_UNIFORM_BUFFER, Shaders[shader_handle].UnifSize,
            Shaders[shader_handle].UnifData, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, Shaders[shader_handle].UnifBuffer.ID);
    }
}