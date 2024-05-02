#include "nyas_types.h"

#include <map>
#include <stdio.h>
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