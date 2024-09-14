/*
    TODO:
    * Change flag values from enum to constexpr unsigned int (see constexpr
   'inline' unsigned int)
    * glTF Import/Export
*/

#ifndef NYAS_H
#define NYAS_H

#ifdef NYAS_USER_CONFIG_H
#include NYAS_USER_CONFIG_H
#endif
#include "nyconfig.h"

#include "nyas_math.h"
#include "nyas_platform.h"
#include "nyas_types.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

struct NyasCamera;
struct NyasEntity;
struct NyasCtx;

extern NyTextures GTextures;
extern NyPipelines GShaders;
extern NyMeshes GMeshes;
extern NyFramebuffers GFb;

namespace nyas {
void Draw(NyasDrawCmd *command);

NyasCtx *GetCurrentCtx();

bool InitIO(const char *title, int win_w, int win_h);
void PollIO();
void WindowSwap();
int ReadFile(const char *path, char **dst, size_t *size);
} // namespace nyas

struct NyasCtx {
    NyasPlatform Platform;
    NyasConfig Cfg;
    NyasIO IO;
};

struct NyasCamera {
    nyas::Mat4 View;
    nyas::Mat4 Proj;
    float Far;
    float Fov;
    NyasCamera() = default;
    void Navigate();

    // Position.
    inline nyas::Vec3 Eye() const {
        nyas::Mat4 inverse = nyas::Mat4::inv(View);
        return {inverse[12], inverse[13], inverse[14]};
    }

    inline nyas::Vec3 Fwd() const { return {View[2], View[6], View[10]}; } // Forward vector.

    // Matrix with zeroed translation (i.e., projection * vec4(vec3(view)). For
    // skybox.
    inline void OriginViewProj(nyas::Mat4 &out) {
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

    inline void Init(
        const NyasCtx &ctx, nyas::Vec3 pos = {0.0f, 2.0f, 2.0f},
        nyas::Vec3 target = {0.0f, 0.0f, -1.0f}, float far = 300.0f, float fov = 70.0f) {
        Far = far;
        Fov = fov;
        View = nyas::Mat4::look_at(pos, target, nyas::Vec3::up());
        Proj = nyas::Mat4::perspective(
            nyas::to_radians(fov), ctx.Platform.WindowSize.x, ctx.Platform.WindowSize.y, 0.01f, far);
    }
};

struct NyasEntity {
    nyas::Mat4 Transform;
    NyasHandle Mesh;
    NyasHandle Shader;
};

namespace nyas {
extern NyPool<NyasFramebuffer> Framebufs;
extern NyPool<NyasEntity> Entities;
extern NyasCamera Camera;
} // namespace nyas

// ---
// [UTILS]
// ---

// Nanosecond timer
struct NyChrono {
    int64_t Start{0};

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

enum NyasGeometry { NyasGeometry_Quad, NyasGeometry_Cube, NyasGeometry_Sphere, NyasGeometry_COUNT };

namespace NyUtil {
void LoadBasicGeometries();
// Environment maps
void LoadEnv(
    const char *path, NyasTexture *lut, NyasTexture *sky, NyasTexture *irr, NyasTexture *pref);
} // namespace NyUtil

// Geometry
extern NyasHandle NYAS_SPHERE;
extern NyasHandle NYAS_CUBE;
extern NyasHandle NYAS_QUAD;

#endif // NYAS_H
