#ifndef NYAS_AZDO_H
#define NYAS_AZDO_H

#include "nyas.h"
#include <glad/glad.h>
#include <vector>

enum BufferType
{
    Vertex,
    Index,
    DrawIndirect
};

template<typename T>
struct NyasBuffer
{

    NyasBuffer(BufferType type) : InternalID(-1), Type(type), Dirty(false) {}
    std::vector<T> Data;
    unsigned int InternalID;
    BufferType Type;
    bool Dirty;
};

typedef struct DrawIndirectData {
    unsigned int Count;
    unsigned int InstanceCount;
    unsigned int FirstIdx;
    int BaseVtx;
    unsigned int BaseInstance;
} DrawIndirectData;

struct NyMeshOffset
{
    ptrdiff_t BaseVtx;
    ptrdiff_t FirstIdx;
    ptrdiff_t ElementCount;
};

struct NyMergedMesh
{
    NyMergedMesh() : Vertices(Vertex), Indices(Index), DrawIndirect(BufferType::DrawIndirect) {}

    void InitGPU()
    {
        glGenBuffers(1, &Vertices.InternalID);
        glGenBuffers(1, &Indices.InternalID);
        glGenBuffers(1, &DrawIndirect.InternalID);
    }

    void Bind()
    {
        glBindBuffer(GL_ARRAY_BUFFER, Vertices.InternalID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Indices.InternalID);
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, DrawIndirect.InternalID);
        if (Vertices.Dirty)
        {
            glBufferData(GL_ARRAY_BUFFER, Vertices.Data.size() * sizeof(float), Vertices.Data.data(), GL_DYNAMIC_DRAW);
        }

        if (Indices.Dirty)
        {
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, Indices.Data.size() * sizeof(unsigned short), Indices.Data.data(), GL_DYNAMIC_DRAW);
        }

        if (DrawIndirect.Dirty)
        {
            glBufferData(GL_DRAW_INDIRECT_BUFFER, DrawIndirect.Data.size() * sizeof(DrawIndirectData), DrawIndirect.Data.data(), GL_DYNAMIC_DRAW);
        }
    }

    void Add(BufferType type, void *data, ptrdiff_t count)
    {
        switch (type)
        {
            case Vertex:
            {
                for (int i = 0; i < count; ++i)
                {
                    Vertices.Data.emplace_back(((float*)data)[i]);
                }
                Vertices.Dirty = true;
                break;
            }
            case Index:
            {
                for (int i = 0; i < count; ++i)
                {
                    Indices.Data.emplace_back(((unsigned short*)data)[i]);
                }
                Indices.Dirty = true;
                break;
            }
            case BufferType::DrawIndirect:
            {
                for (int i = 0; i < count; ++i)
                {
                    DrawIndirect.Data.emplace_back(((DrawIndirectData*)data)[i]);
                }
                DrawIndirect.Dirty = true;
                break;
            }
        }
    }

    NyMergedMesh LoadFile(const char* path)
    {
        auto Mesh = Nyas::LoadMesh(path);
        Nyas::
    }

    void Draw()
    {
        Bind();
        glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_SHORT, 0, DrawIndirect.Data.size(), 0);
    }

    NyasBuffer<float> Vertices;
    NyasBuffer<unsigned short> Indices;
    NyasBuffer<DrawIndirectData> DrawIndirect;
};

#endif // NYAS_AZDO_H
