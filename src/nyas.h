/*
    TODO:
    * Change flag values from enum to constexpr unsigned int (see constexpr 'inline' unsigned int)
    * glTF Import/Export
    * Async resource loader
    * Math lib
    * Vertex attributes in NyasPipeline. Vertex layout and in/out in NyasShaderSrc.

*/

#ifndef NYAS_H
#define NYAS_H

#ifdef NYAS_USER_CONFIG_H
#include NYAS_USER_CONFIG_H
#endif
#include "nyconfig.h"
#include "nyas_types.h"
#include "nyas_math.h"
#include "nyas_platform.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

struct NyasCamera;
struct NyasEntity;
struct NyasCtx;

extern NyTextures GTextures;
extern NyPipelines GShaders;

namespace nyas
{
NyasHandle CreateFramebuffer();
void SetFramebufferTarget(NyasHandle fb, int index, NyasTexTarget target);

// TODO(Renderer): Unificar load y reload
NyasHandle CreateMesh();
NyasHandle LoadMesh(const char *path);
void ReloadMesh(NyasHandle mesh, const char *path);

void Draw(NyasDrawCmd *command);

NyasCtx *GetCurrentCtx();

bool InitIO(const char *title, int win_w, int win_h);
void PollIO();
void WindowSwap();
int ReadFile(const char *path, char **dst, size_t *size);
} // namespace Nyas

typedef struct NyasCtx
{
    NyasPlatform Platform;
    NyasConfig Cfg;
    NyasIO IO;
} NyasCtx;

struct NyasCamera
{
    nym::mat4_t View;
	nym::mat4_t Proj;
    float Far;
    float Fov;
    NyasCamera() = default;
    void Navigate();

    // Position.
    inline nym::vec3_t Eye() const
    {
        nym::mat4_t inverse = nym::mat4_t::inv(View);
        return { inverse[12], inverse[13], inverse[14] };
    }

    inline nym::vec3_t Fwd() const { return { View[2], View[6], View[10] }; } // Forward vector.

    // Matrix with zeroed translation (i.e., projection * vec4(vec3(view)). For skybox.
    inline void OriginViewProj(nym::mat4_t& out)
    {
		out = View;
        out[3] = 0.0f;
        out[7] = 0.0f;
        out[11] = 0.0f;
        out[12] = 0.0f;
        out[13] = 0.0f;
        out[14] = 0.0f;
        out[15] = 0.0f;
		out = Proj * out;
    }

    inline void Init(const NyasCtx& ctx, nym::vec3_t pos = { 0.0f, 2.0f, 2.0f },
					 nym::vec3_t target = { 0.0f, 0.0f, -1.0f }, float far = 300.0f, float fov = 70.0f)
    {
        Far = far;
        Fov = fov;
		View = nym::mat4_t::look_at(pos, target, nym::vec3_t::up());
		Proj = nym::mat4_t::perspective(nym::to_radians(fov), ctx.Platform.WindowSize.X, ctx.Platform.WindowSize.Y, 0.01f, far);
    }
};

typedef struct NyasEntity
{
    float Transform[16];
    NyasHandle Mesh;
    NyasHandle Shader;
} NyasEntity;

namespace nyas
{
extern NyPool<NyasMesh> Meshes;
extern NyPool<NyasFramebuffer> Framebufs;
extern NyPool<NyasEntity> Entities;
extern NyasCamera Camera;
} // namespace nyas

// ---
// [UTILS]
// ---

// Nanosecond timer
struct NyChrono
{
    int64_t Start { 0 };

    static inline int64_t GetTime() { return NYAS_GET_TIME_NS; } // Current time in nanoseconds.
    static constexpr int64_t MicroSeconds(int64_t ns) { return ns / 1000; }
    static constexpr int64_t MilliSeconds(int64_t ns) { return MicroSeconds(ns) / 1000; }
    static constexpr int64_t Seconds(int64_t ns) { return MilliSeconds(ns) / 1000; }
    static constexpr double MicroSeconds(double ns) { return ns / 1000.0; }
    static constexpr double MilliSeconds(double ns) { return MicroSeconds(ns) / 1000.0; }
    static constexpr double Seconds(double ns) { return MilliSeconds(ns) / 1000.0; }

    NyChrono() : Start(GetTime()) {}
    NyChrono(int64_t start_ns) : Start(start_ns) {}
    void Restart() { Start = GetTime(); }
    int64_t Elapsed() { return GetTime() - Start; }
};

enum NyasGeometry
{
    NyasGeometry_Quad,
    NyasGeometry_Cube,
    NyasGeometry_Sphere,
    NyasGeometry_COUNT
};

namespace NyUtil
{
void LoadBasicGeometries();
// Environment maps
void LoadEnv(const char *path, NyasTexture *lut, NyasTexture *sky, NyasTexture *irr, NyasTexture *pref);
} // namespace NyUtil

// Geometry
extern NyasHandle NYAS_SPHERE;
extern NyasHandle NYAS_CUBE;
extern NyasHandle NYAS_QUAD;

#endif // NYAS_H
