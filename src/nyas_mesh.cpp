#include "nyas.h"
#include "nyas_render.h"
#include "nyas_types.h"

#include <set>

#define TINYOBJ_LOADER_C_IMPLEMENTATION
#include "tinyobj_loader_c.h"

static void
_NyReadFile(void *_1, const char *path, int _2, const char *_3, char **buf, size_t *size) {
    (void)_1, (void)_2, (void)_3;
    nyas::ReadFile(path, buf, size);
}

static NyDrawIdx _CheckVertex(const float *v, const float *end, const float *newvtx) {
    NyDrawIdx i = 0;
    for (; v < end; ++i, v += 14) {
        if ((v[0] == newvtx[0]) && (v[1] == newvtx[1]) && (v[2] == newvtx[2]) &&
            (v[3] == newvtx[3]) && (v[4] == newvtx[4]) && (v[5] == newvtx[5]) &&
            (v[12] == newvtx[12]) && (v[13] == newvtx[13])) {
            return i;
        }
    }
    return i;
}

void _SetMeshObj(NyMeshes::LoadedMesh *mesh, const char *path) {
    tinyobj_attrib_t attrib;
    tinyobj_shape_t *shapes = NULL;
    size_t shape_count;
    tinyobj_material_t *mats = NULL;
    size_t mats_count;

    int result = tinyobj_parse_obj(
        &attrib, &shapes, &shape_count, &mats, &mats_count, path, _NyReadFile, NULL,
        TINYOBJ_FLAG_TRIANGULATE);

    NYAS_ASSERT(result == TINYOBJ_SUCCESS && "Obj loader failed.");
    if (result != TINYOBJ_SUCCESS) {
        NYAS_LOG_ERR("Error loading obj. Err: %d", result);
    }

    size_t vertex_count = attrib.num_face_num_verts * 3;
    mesh->Vertices.clear();
    mesh->Indices.clear();

    /*mesh->Attribs = NyasVtxAttribFlags_Position | NyasVtxAttribFlags_Normal |
                    NyasVtxAttribFlags_Tangent | NyasVtxAttribFlags_Bitangent |
                    NyasVtxAttribFlags_UV;*/
    mesh->Vertices.resize(vertex_count * 14);
    mesh->Indices.resize(vertex_count);

    float *vit = mesh->Vertices.data();

    size_t index_offset = 0;
    for (size_t i = 0; i < attrib.num_face_num_verts; ++i) {
        for (int f = 0; f < attrib.face_num_verts[i] / 3; ++f) {
            tinyobj_vertex_index_t idx = attrib.faces[3 * f + index_offset];
            float v1[14], v2[14], v3[14];

            v1[0] = attrib.vertices[3 * idx.v_idx + 0];
            v1[1] = attrib.vertices[3 * idx.v_idx + 1];
            v1[2] = attrib.vertices[3 * idx.v_idx + 2];
            v1[3] = attrib.normals[3 * idx.vn_idx + 0];
            v1[4] = attrib.normals[3 * idx.vn_idx + 1];
            v1[5] = attrib.normals[3 * idx.vn_idx + 2];
            v1[12] = attrib.texcoords[2 * idx.vt_idx + 0];
            v1[13] = attrib.texcoords[2 * idx.vt_idx + 1];

            idx = attrib.faces[3 * f + index_offset + 1];
            v2[0] = attrib.vertices[3 * idx.v_idx + 0];
            v2[1] = attrib.vertices[3 * idx.v_idx + 1];
            v2[2] = attrib.vertices[3 * idx.v_idx + 2];
            v2[3] = attrib.normals[3 * idx.vn_idx + 0];
            v2[4] = attrib.normals[3 * idx.vn_idx + 1];
            v2[5] = attrib.normals[3 * idx.vn_idx + 2];
            v2[12] = attrib.texcoords[2 * idx.vt_idx + 0];
            v2[13] = attrib.texcoords[2 * idx.vt_idx + 1];

            idx = attrib.faces[3 * f + index_offset + 2];
            v3[0] = attrib.vertices[3 * idx.v_idx + 0];
            v3[1] = attrib.vertices[3 * idx.v_idx + 1];
            v3[2] = attrib.vertices[3 * idx.v_idx + 2];
            v3[3] = attrib.normals[3 * idx.vn_idx + 0];
            v3[4] = attrib.normals[3 * idx.vn_idx + 1];
            v3[5] = attrib.normals[3 * idx.vn_idx + 2];
            v3[12] = attrib.texcoords[2 * idx.vt_idx + 0];
            v3[13] = attrib.texcoords[2 * idx.vt_idx + 1];

            // Calculate tangent and bitangent
            nyas::Vec3 delta_p1 = (nyas::Vec3)&v2[0] - (nyas::Vec3)&v1[0];
            nyas::Vec3 delta_p2 = (nyas::Vec3)&v3[0] - (nyas::Vec3)&v1[0];
            nyas::Vec2 delta_uv1 = (nyas::Vec2)&v2[12] - (nyas::Vec2)&v1[12];
            nyas::Vec2 delta_uv2 = (nyas::Vec2)&v3[12] - (nyas::Vec2)&v1[12];
            float r = 1.0f / (delta_uv1[0] * delta_uv2[1] - delta_uv1[1] * delta_uv2[0]);

            nyas::Vec3 tn = delta_p1 * delta_uv2[1];
            nyas::Vec3 bitn = delta_p2 * delta_uv1[0];
            tn = (tn - (delta_p1 * delta_uv1[1])) * r;
            bitn = (bitn - (delta_p1 * delta_uv2[0])) * r;

            v1[6] = tn[0];
            v1[7] = tn[1];
            v1[8] = tn[2];
            v2[6] = tn[0];
            v2[7] = tn[1];
            v2[8] = tn[2];
            v3[6] = tn[0];
            v3[7] = tn[1];
            v3[8] = tn[2];

            v1[9] = bitn[0];
            v1[10] = bitn[1];
            v1[11] = bitn[2];
            v2[9] = bitn[0];
            v2[10] = bitn[1];
            v2[11] = bitn[2];
            v3[9] = bitn[0];
            v3[10] = bitn[1];
            v3[11] = bitn[2];

            // Check vertex rep
            NyDrawIdx nxt_idx = _CheckVertex(mesh->Vertices.data(), vit, v1);
            mesh->Indices[index_offset++] = nxt_idx;
            if (nxt_idx * 14 == (vit - mesh->Vertices.data())) {
                for (int j = 0; j < 14; ++j) {
                    *vit++ = v1[j];
                }
            }

            nxt_idx = _CheckVertex(mesh->Vertices.data(), vit, v2);
            mesh->Indices[index_offset++] = nxt_idx;
            if (nxt_idx * 14 == (vit - mesh->Vertices.data())) {
                for (int j = 0; j < 14; ++j) {
                    *vit++ = v2[j];
                }
            }

            nxt_idx = _CheckVertex(mesh->Vertices.data(), vit, v3);
            mesh->Indices[index_offset++] = nxt_idx;
            if (nxt_idx * 14 == (vit - mesh->Vertices.data())) {
                for (int j = 0; j < 14; ++j) {
                    *vit++ = v3[j];
                }
            }
        }
    }

    tinyobj_attrib_free(&attrib);
    tinyobj_shapes_free(shapes, shape_count);
    tinyobj_materials_free(mats, mats_count);
}

static void _SetMeshMsh(NyMeshes::LoadedMesh *mesh, const char *path) {
    char *data;
    size_t sz;
    _NyReadFile(NULL, path, 0, NULL, &data, &sz);
    if (!data || !sz) {
        NYAS_LOG_ERR("Problem reading file %s", path);
        return;
    }

    mesh->Vertices.clear();
    mesh->Indices.clear();

    mesh->Vertices.resize(*(size_t *)data / sizeof(float));
    data += sizeof(size_t);
    memcpy(mesh->Vertices.data(), data, mesh->Vertices.size() * sizeof(float));
    data += mesh->Vertices.size() * sizeof(float);

    mesh->Indices.resize((*(size_t *)data) / sizeof(NyDrawIdx));
    data += sizeof(size_t);
    memcpy(mesh->Indices.data(), data, mesh->Indices.size() * sizeof(NyDrawIdx));

    NYAS_FREE(data - mesh->Vertices.size() * sizeof(float) - (2 * sizeof(size_t)));
}

NyasHandle NyMeshes::Alloc(NyasVtxAttribFlags attribs) {
    NyasHandle handle = Meshes.size();
    Meshes.emplace_back(attribs, 0, 0, 0);
    auto &data = Data.emplace_back();
    data.Attribs = attribs;
    return handle;
}

void NyMeshes::Load(NyasHandle handle, const char *path) {
    auto LoadMesh = [](const char *path) {
        LoadedMesh mesh;
        size_t len = strlen(path);
        const char *extension = path + len;
        while (*--extension != '.') {
        }
        extension++;
        if (!strcmp(extension, "obj")) {
            _SetMeshObj(&mesh, path);
        } else if (!strcmp(extension, "msh")) {
            _SetMeshMsh(&mesh, path);
        } else {
            NYAS_LOG_ERR("Extension (%s) of file %s not recognised.", extension, path);
        }
        return mesh;
    };

    Updates.emplace_back(handle, std::async(std::launch::async, LoadMesh, path));
}

void NyMeshes::Update(NyasHandle handle, float *vtx, int vtx_count, NyDrawIdx *ind, int ind_count) {
    LoadedMesh mesh;
    mesh.Vertices.insert(mesh.Vertices.end(), vtx, vtx + vtx_count);
    mesh.Indices.insert(mesh.Indices.end(), ind, ind + ind_count);
    Updates.emplace_back(
        handle,
        std::async(std::launch::deferred, [](LoadedMesh &&mesh) { return mesh; }, std::move(mesh)));
}

NyMeshes::NyMeshArray *NyMeshes::GetData(NyasVtxAttribFlags attribs) {
    NyMeshArray *data = NULL;
    for (auto &attr_data : Data) {
        if (attr_data.Attribs == attribs) {
            data = &attr_data;
        }
    }

    NYAS_ASSERT(data);
    return data;
}

static int _GetStrideUnits(NyasVtxAttribFlags attribs) {
    int stride = 0;
    for (int i = 0; i < NyasVtxAttrib_COUNT; ++i) {
        if (attribs & (1 << i)) {
            stride += nyas::render::AttribSizes[i];
        }
    }
    return stride;
}

void NyMeshes::Sync(NyasVtxAttribFlags attribs) {
    std::set<NyMeshArray *> Dirty;
    for (auto &[hnd, future_mesh] : Updates) {
        LoadedMesh loaded = future_mesh.get();
        NyMeshArray *data = GetData(Meshes[hnd].Attribs);
        Meshes[hnd].Vertex = data->VtxData.size() / _GetStrideUnits(data->Attribs);
        Meshes[hnd].Index = data->IdxData.size();
        Meshes[hnd].Count = loaded.Indices.size();
        data->VtxData.insert(data->VtxData.end(), loaded.Vertices.begin(), loaded.Vertices.end());
        data->IdxData.insert(data->IdxData.end(), loaded.Indices.begin(), loaded.Indices.end());
        Dirty.emplace(data);
    }
    Updates.clear();

    if (!Dirty.empty()) {
        for (NyMeshArray *mesh : Dirty) {
            if ((int)mesh->VaoID == -1) {
                NYAS_ASSERT((int)mesh->VtxID == -1);
                NYAS_ASSERT((int)mesh->IdxID == -1);
                nyas::render::_NyCreateMesh(mesh->Attribs, &mesh->VaoID, &mesh->VtxID, &mesh->IdxID);
            }

            NYAS_ASSERT((int)mesh->VaoID != -1);
            NYAS_ASSERT((int)mesh->VtxID != -1);
            NYAS_ASSERT((int)mesh->IdxID != -1);
            nyas::render::_NySetMesh(mesh);
        }
    }

    NyMeshArray *mesh = GetData(attribs);
    nyas::render::_NyUseMesh(mesh->VaoID);
}
