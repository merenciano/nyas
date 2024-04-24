#include <vector>
#include <unordered_map>

namespace ex
{
    using idx_t = unsigned short;
    using ihnd_t = unsigned int;

    typedef int ResourceFlags;
    enum ResourceFlags_
    {
        ResourceFlags_None = 0,
        ResourceFlags_Dirty = 1,
        ResourceFlags_Invalid = 1 << 1,
        ResourceFlags_Release = 1 << 2,
    };

    struct InternalResource
    {
        ihnd_t Id;
        ResourceFlags Flags;
    };

    template<typename T>
    struct NyBuffer
    {
        enum Type
        {
            BufferType_None,
            BufferType_Vertex,
            BufferType_Index,
            BufferType_ShaderData,
            BufferType_Texture,
            BufferType_DrawIndirect,
            BufferType_InstanceData,
            BufferType_COUNT
        };
        InternalResource InternalRes;
        std::vector<T> Data;
        Type Type;
    };

    typedef int VtxAttribFlags;

    enum VtxAttribFlags_
    {
        VtxAttribFlags_None = 0,
        VtxAttribFlags_Pos = 1,
        VtxAttribFlags_Norm = 1 << 1,
        VtxAttribFlags_Tan = 1 << 2,
        VtxAttribFlags_BiTan = 1 << 3,
        VtxAttribFlags_UV = 1 << 4,
        VtxAttribFlags_Color = 1 << 5,
    };

    struct Mesh
    {
        VtxAttribFlags Attribs;
        int VtxOffset;
        int IdxOffset;
        int IdxCount;

        Mesh(VtxAttribFlags attr, int voffset, int ioffset, int icount)
            : Attribs(attr), VtxOffset(voffset), IdxOffset(ioffset), IdxCount(icount) {}
    };

    struct InternalMeshes
    {
        struct InternalMesh
        {
            NyBuffer<float> Vertices;
            NyBuffer<idx_t> Indices;
        };
        std::unordered_map<VtxAttribFlags, InternalMesh> Meshes;

        Mesh AddMesh(VtxAttribFlags attr, const std::vector<float> &v, const std::vector<idx_t> &idx)
        {
            Mesh m(attr, Meshes[attr].Vertices.Data.size(), Meshes[attr].Indices.Data.size(), idx.size());
            Meshes[attr].Vertices.Data.insert(Meshes[attr].Vertices.Data.end(), v.begin(), v.end());
            Meshes[attr].Indices.Data.insert(Meshes[attr].Indices.Data.end(), idx.begin(), idx.end());
            Meshes[attr].Vertices.InternalRes.Flags |= ResourceFlags_Dirty;
            Meshes[attr].Indices.InternalRes.Flags |= ResourceFlags_Dirty;
            return m;
        }
    };

    struct Context
    {
        InternalMeshes Meshes;
    };
}
