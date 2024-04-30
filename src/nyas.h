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

extern azdo::Textures GTextures;

namespace Nyas
{
NyasHandle CreateTexture();
NyasHandle CreateTexture(int w, int h, NyasTexType t, NyasTexFmt f, int count = 1);
void SetTexture(NyasHandle tex, NyasTexDesc *desc);
void LoadTexture(NyasHandle tex, NyasTexDesc *desc, const char *path, int index = 0);

NyasHandle CreateFramebuffer();
void SetFramebufferTarget(NyasHandle fb, int index, NyasTexTarget target);

// TODO(Renderer): Unificar load y reload
NyasHandle CreateMesh();
NyasHandle LoadMesh(const char *path);
void ReloadMesh(NyasHandle mesh, const char *path);

NyasHandle CreateShader(const NyasShaderDesc *desc);
void ReloadShader(NyasHandle shader);

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

typedef struct NyasCamera
{
    float View[16];
    float Proj[16];
    float Far;
    float Fov;
    NyasCamera() { memset(this, 0, sizeof(*this)); }
    void Navigate();

    // Position.
    inline NyVec3 Eye() const
    {
        float inv[16];
        mat4_inverse(inv, View);
        return { inv[12], inv[13], inv[14] };
    }

    inline NyVec3 Fwd() const { return { View[2], View[6], View[10] }; } // Forward vector.

    // Matrix with zeroed translation (i.e., projection * vec4(vec3(view)). For skybox.
    inline float *OriginViewProj(float out[16])
    {
        mat4_assign(out, View);
        out[3] = 0.0f;
        out[7] = 0.0f;
        out[11] = 0.0f;
        out[12] = 0.0f;
        out[13] = 0.0f;
        out[14] = 0.0f;
        out[15] = 0.0f;
        return mat4_multiply(out, Proj, out);
    }

    inline void Init(const NyasCtx& ctx, NyVec3 pos = { 0.0f, 2.0f, 2.0f },
        NyVec3 target = { 0.0f, 0.0f, -1.0f }, float far = 300.0f, float fov = 70.0f)
    {
        Far = far;
        Fov = fov;
        mat4_look_at(View, pos, target, NyVec3::Up());
        mat4_perspective_fov(Proj, to_radians(Fov), ctx.Platform.WindowSize.X, ctx.Platform.WindowSize.Y, 0.01f, Far);
    }
} NyasCamera;

typedef struct NyasEntity
{
    float Transform[16];
    NyasHandle Mesh;
    NyasHandle Shader;
} NyasEntity;

namespace Nyas
{
extern NyPool<NyasMesh> Meshes;
extern NyPool<NyasTexture> Textures;
extern NyPool<NyasShader> Shaders;
extern NyPool<NyasFramebuffer> Framebufs;
extern NyPool<NyasEntity> Entities;
extern NyasCamera Camera;
} // namespace Nyas

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

struct NySched
{
    struct Job
    {
        void (*Func)(void *);
        void *Args;
        Job() : Func(NULL), Args(NULL) {}
        Job(void (*func)(void *), void *args) : Func(func), Args(args) {}
    };

    struct _NyScheduler *_Sched;

    NySched(int thread_count);
    ~NySched();
    void Do(Job job);
    void Wait();
};

struct NyAssetLoader
{
    struct MeshArgs
    {
        const char *Path;
        NyasHandle *Mesh;
    };

    struct ShaderArgs
    {
        NyasShaderDesc Descriptor;
        NyasHandle *Shader;
    };

    struct EnvArgs
    {
        const char *Path;
		azdo::TexHandle *Sky;
		azdo::TexHandle *Irradiance;
		azdo::TexHandle *Pref;
        azdo::TexHandle *LUT;
    };

    NyArray<NySched::Job> Sequential;
    NyArray<NySched::Job> Async;
    void AddMesh(MeshArgs *args);
    void AddShader(ShaderArgs *args);
    void AddEnv(EnvArgs *args);
    void AddJob(NySched::Job job, bool async);
    void Load(int threads);
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
void LoadEnv(const char *path, azdo::TexHandle *lut, azdo::TexHandle *sky, azdo::TexHandle *irr, azdo::TexHandle *pref);

} // namespace NyUtil

// Geometry
extern NyasHandle NYAS_SPHERE;
extern NyasHandle NYAS_CUBE;
extern NyasHandle NYAS_QUAD;

#endif // NYAS_H
