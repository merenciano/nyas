#include "nyas.h"
#include "nyas_render.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"

#define TINYOBJ_LOADER_C_IMPLEMENTATION

#include "tinyobj_loader_c.h"

#define MEM_ALIGN 8
#define MEM_ALIGN_MOD(ADDRESS) ((ADDRESS) & (MEM_ALIGN - 1))

#define NYAS_MOUSE_BUTTON_UPDATE(MBTN)                                         \
  G_Ctx->IO.MouseButton[(MBTN)] =                                              \
      ((G_Ctx->IO.MouseButton[(MBTN)] << 1) |                                  \
       glfwGetMouseButton((GLFWwindow *)G_Ctx->Platform.InternalWindow,        \
                          (MBTN))) &                                           \
      3

#define NYAS_KEY_UPDATE(KEY)                                                   \
  (G_Ctx->IO.Keys[(KEY)] =                                                     \
       ((G_Ctx->IO.Keys[(KEY)] << 1) |                                         \
        glfwGetKey((GLFWwindow *)G_Ctx->Platform.InternalWindow, (KEY))) &     \
       3)

NyasCtx DefaultCtx;
NyasCtx *G_Ctx = &DefaultCtx;

namespace nyas {
NyPool<NyasMesh> Meshes;
NyPool<NyasFramebuffer> Framebufs;

NyPool<NyasEntity> Entities;
NyasCamera Camera;

static inline void _NyReadInput(void) {
  NYAS_KEY_UPDATE(NyasKey_Invalid);
  NYAS_KEY_UPDATE(NyasKey_Space);
  NYAS_KEY_UPDATE(NyasKey_Apostrophe);
  NYAS_KEY_UPDATE(NyasKey_Comma);
  NYAS_KEY_UPDATE(NyasKey_Minus);
  NYAS_KEY_UPDATE(NyasKey_Period);
  NYAS_KEY_UPDATE(NyasKey_Slash);
  NYAS_KEY_UPDATE(NyasKey_0);
  NYAS_KEY_UPDATE(NyasKey_1);
  NYAS_KEY_UPDATE(NyasKey_2);
  NYAS_KEY_UPDATE(NyasKey_3);
  NYAS_KEY_UPDATE(NyasKey_4);
  NYAS_KEY_UPDATE(NyasKey_5);
  NYAS_KEY_UPDATE(NyasKey_6);
  NYAS_KEY_UPDATE(NyasKey_7);
  NYAS_KEY_UPDATE(NyasKey_8);
  NYAS_KEY_UPDATE(NyasKey_9);
  NYAS_KEY_UPDATE(NyasKey_Semicolon);
  NYAS_KEY_UPDATE(NyasKey_Equal);
  NYAS_KEY_UPDATE(NyasKey_A);
  NYAS_KEY_UPDATE(NyasKey_B);
  NYAS_KEY_UPDATE(NyasKey_C);
  NYAS_KEY_UPDATE(NyasKey_D);
  NYAS_KEY_UPDATE(NyasKey_E);
  NYAS_KEY_UPDATE(NyasKey_F);
  NYAS_KEY_UPDATE(NyasKey_G);
  NYAS_KEY_UPDATE(NyasKey_H);
  NYAS_KEY_UPDATE(NyasKey_I);
  NYAS_KEY_UPDATE(NyasKey_J);
  NYAS_KEY_UPDATE(NyasKey_K);
  NYAS_KEY_UPDATE(NyasKey_L);
  NYAS_KEY_UPDATE(NyasKey_M);
  NYAS_KEY_UPDATE(NyasKey_N);
  NYAS_KEY_UPDATE(NyasKey_O);
  NYAS_KEY_UPDATE(NyasKey_P);
  NYAS_KEY_UPDATE(NyasKey_Q);
  NYAS_KEY_UPDATE(NyasKey_R);
  NYAS_KEY_UPDATE(NyasKey_S);
  NYAS_KEY_UPDATE(NyasKey_T);
  NYAS_KEY_UPDATE(NyasKey_U);
  NYAS_KEY_UPDATE(NyasKey_V);
  NYAS_KEY_UPDATE(NyasKey_W);
  NYAS_KEY_UPDATE(NyasKey_X);
  NYAS_KEY_UPDATE(NyasKey_Y);
  NYAS_KEY_UPDATE(NyasKey_Z);
  NYAS_KEY_UPDATE(NyasKey_LeftBracket);
  NYAS_KEY_UPDATE(NyasKey_Backslash);
  NYAS_KEY_UPDATE(NyasKey_RightBracket);
  NYAS_KEY_UPDATE(NyasKey_GraveAccent);
  NYAS_KEY_UPDATE(NyasKey_World1);
  NYAS_KEY_UPDATE(NyasKey_World2);
  NYAS_KEY_UPDATE(NyasKey_Escape);
  NYAS_KEY_UPDATE(NyasKey_Enter);
  NYAS_KEY_UPDATE(NyasKey_Tab);
  NYAS_KEY_UPDATE(NyasKey_Backspace);
  NYAS_KEY_UPDATE(NyasKey_Insert);
  NYAS_KEY_UPDATE(NyasKey_Delete);
  NYAS_KEY_UPDATE(NyasKey_Right);
  NYAS_KEY_UPDATE(NyasKey_Left);
  NYAS_KEY_UPDATE(NyasKey_Down);
  NYAS_KEY_UPDATE(NyasKey_Up);
  NYAS_KEY_UPDATE(NyasKey_PageUp);
  NYAS_KEY_UPDATE(NyasKey_PageDown);
  NYAS_KEY_UPDATE(NyasKey_Home);
  NYAS_KEY_UPDATE(NyasKey_End);
  NYAS_KEY_UPDATE(NyasKey_CapsLock);
  NYAS_KEY_UPDATE(NyasKey_ScrollLock);
  NYAS_KEY_UPDATE(NyasKey_NumLock);
  NYAS_KEY_UPDATE(NyasKey_PrintScreen);
  NYAS_KEY_UPDATE(NyasKey_Pause);
  NYAS_KEY_UPDATE(NyasKey_F1);
  NYAS_KEY_UPDATE(NyasKey_F2);
  NYAS_KEY_UPDATE(NyasKey_F3);
  NYAS_KEY_UPDATE(NyasKey_F4);
  NYAS_KEY_UPDATE(NyasKey_F5);
  NYAS_KEY_UPDATE(NyasKey_F6);
  NYAS_KEY_UPDATE(NyasKey_F7);
  NYAS_KEY_UPDATE(NyasKey_F8);
  NYAS_KEY_UPDATE(NyasKey_F9);
  NYAS_KEY_UPDATE(NyasKey_F10);
  NYAS_KEY_UPDATE(NyasKey_F11);
  NYAS_KEY_UPDATE(NyasKey_F12);
  NYAS_KEY_UPDATE(NyasKey_F13);
  NYAS_KEY_UPDATE(NyasKey_F14);
  NYAS_KEY_UPDATE(NyasKey_F15);
  NYAS_KEY_UPDATE(NyasKey_F16);
  NYAS_KEY_UPDATE(NyasKey_F17);
  NYAS_KEY_UPDATE(NyasKey_F18);
  NYAS_KEY_UPDATE(NyasKey_F19);
  NYAS_KEY_UPDATE(NyasKey_F20);
  NYAS_KEY_UPDATE(NyasKey_F21);
  NYAS_KEY_UPDATE(NyasKey_F22);
  NYAS_KEY_UPDATE(NyasKey_F23);
  NYAS_KEY_UPDATE(NyasKey_F24);
  NYAS_KEY_UPDATE(NyasKey_F25);
  NYAS_KEY_UPDATE(NyasKey_Kp0);
  NYAS_KEY_UPDATE(NyasKey_Kp1);
  NYAS_KEY_UPDATE(NyasKey_Kp2);
  NYAS_KEY_UPDATE(NyasKey_Kp3);
  NYAS_KEY_UPDATE(NyasKey_Kp4);
  NYAS_KEY_UPDATE(NyasKey_Kp5);
  NYAS_KEY_UPDATE(NyasKey_Kp6);
  NYAS_KEY_UPDATE(NyasKey_Kp7);
  NYAS_KEY_UPDATE(NyasKey_Kp8);
  NYAS_KEY_UPDATE(NyasKey_Kp9);
  NYAS_KEY_UPDATE(NyasKey_KpDecimal);
  NYAS_KEY_UPDATE(NyasKey_KpDivide);
  NYAS_KEY_UPDATE(NyasKey_KpMultiply);
  NYAS_KEY_UPDATE(NyasKey_KpSubtract);
  NYAS_KEY_UPDATE(NyasKey_KpAdd);
  NYAS_KEY_UPDATE(NyasKey_KpEnter);
  NYAS_KEY_UPDATE(NyasKey_KpEqual);
  NYAS_KEY_UPDATE(NyasKey_LeftShift);
  NYAS_KEY_UPDATE(NyasKey_LeftControl);
  NYAS_KEY_UPDATE(NyasKey_LeftAlt);
  NYAS_KEY_UPDATE(NyasKey_LeftSuper);
  NYAS_KEY_UPDATE(NyasKey_RightShift);
  NYAS_KEY_UPDATE(NyasKey_RightControl);
  NYAS_KEY_UPDATE(NyasKey_RightAlt);
  NYAS_KEY_UPDATE(NyasKey_RightSuper);
  NYAS_KEY_UPDATE(NyasKey_Menu);

  NYAS_MOUSE_BUTTON_UPDATE(NyasMouseButton_Left);
  NYAS_MOUSE_BUTTON_UPDATE(NyasMouseButton_Right);
  NYAS_MOUSE_BUTTON_UPDATE(NyasMouseButton_Middle);

  double x, y;
  glfwGetCursorPos((GLFWwindow *)G_Ctx->Platform.InternalWindow, &x, &y);
  G_Ctx->IO.MousePosition = {(float)x, (float)y};
}

static void _NyScrollCallback(GLFWwindow *window, double x_offset,
                              double y_offset) {
  NY_UNUSED(window);
  G_Ctx->IO.MouseScroll = {(float)x_offset, (float)y_offset};
}

static void _NyCursorEnterCallback(GLFWwindow *window, int entered) {
  NY_UNUSED(window);
  G_Ctx->Platform.WindowHovered = entered;
}

static void _NyWindowFocusCallback(GLFWwindow *window, int focused) {
  NY_UNUSED(window);
  G_Ctx->Platform.WindowFocused = focused;
}

NyasCtx *GetCurrentCtx() { return G_Ctx; }

bool InitIO(const char *title, int win_w, int win_h) {
  if (!glfwInit()) {
    return false;
  }

  G_Ctx->Platform.InternalWindow =
      glfwCreateWindow(win_w, win_h, title, NULL, NULL);
  if (!G_Ctx->Platform.InternalWindow) {
    glfwTerminate();
    return false;
  }

  glfwMakeContextCurrent((GLFWwindow *)G_Ctx->Platform.InternalWindow);
  nyas::render::_NySetProcLoader((void *(*)(const char *))glfwGetProcAddress);

  glfwSwapInterval(1);
  glfwSetScrollCallback((GLFWwindow *)G_Ctx->Platform.InternalWindow,
                        _NyScrollCallback);
  glfwSetCursorEnterCallback((GLFWwindow *)G_Ctx->Platform.InternalWindow,
                             _NyCursorEnterCallback);
  glfwSetWindowFocusCallback((GLFWwindow *)G_Ctx->Platform.InternalWindow,
                             _NyWindowFocusCallback);
  glfwSetInputMode((GLFWwindow *)G_Ctx->Platform.InternalWindow,
                   GLFW_STICKY_KEYS, GLFW_TRUE);
  glfwSetInputMode((GLFWwindow *)G_Ctx->Platform.InternalWindow,
                   GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);

  G_Ctx->Platform.WindowSize = {win_w, win_h};

  G_Ctx->Platform.ShowCursor = true;
  G_Ctx->Platform.CaptureMouse = true;
  G_Ctx->Platform.CaptureKeyboard = true;

  G_Ctx->Cfg.Navigation.Speed = 10.0f;
  G_Ctx->Cfg.Navigation.DragSensibility = 0.001f;
  G_Ctx->Cfg.Navigation.ScrollSensibility = 1.0f;

  PollIO();
  return true;
}

int ReadFile(const char *path, char **dst, size_t *size) {
  FILE *f = fopen(path, "rb");
  if (!f) {
    NYAS_LOG_ERR("File open failed for %s.", path);
    return NyasError_File;
  }

  fseek(f, 0L, SEEK_END);
  *size = ftell(f) + 1;
  rewind(f);

  *dst = (char *)NYAS_ALLOC(*size);
  if (!*dst) {
    NYAS_LOG_ERR("Alloc (%lu bytes) failed.", *size);
    fclose(f);
    return NyasError_Alloc;
  }

  if (fread(*dst, *size - 1, 1, f) != 1) {
    NYAS_LOG_ERR("File read failed for %s.", path);
    NYAS_FREE(*dst);
    fclose(f);
    return NyasError_File;
  }

  fclose(f);
  (*dst)[*size - 1] = '\0';
  return NyasCode_Ok;
}

void PollIO() {
  NYAS_ASSERT(G_Ctx->Platform.InternalWindow &&
              "The IO system is uninitalized");
  G_Ctx->IO.MouseScroll = {0.0f, 0.0f};
  glfwPollEvents();
  _NyReadInput();
  G_Ctx->Platform.WindowClosed =
      glfwWindowShouldClose((GLFWwindow *)G_Ctx->Platform.InternalWindow);
  glfwGetWindowSize((GLFWwindow *)G_Ctx->Platform.InternalWindow,
                    &G_Ctx->Platform.WindowSize.x,
                    &G_Ctx->Platform.WindowSize.y);

  if (G_Ctx->Platform.ShowCursor) {
    glfwSetInputMode((GLFWwindow *)G_Ctx->Platform.InternalWindow, GLFW_CURSOR,
                     GLFW_CURSOR_NORMAL);
  } else {
    glfwSetInputMode((GLFWwindow *)G_Ctx->Platform.InternalWindow, GLFW_CURSOR,
                     GLFW_CURSOR_DISABLED);
  }
}

void WindowSwap() {
  NYAS_ASSERT(G_Ctx->Platform.InternalWindow &&
              "The IO system is uninitalized");
  glfwSwapBuffers((GLFWwindow *)G_Ctx->Platform.InternalWindow);
}

template <typename T>
static inline void _NyCheckHandle(NyasHandle h, const NyPool<T> &pool) {
  NY_UNUSED(h), NY_UNUSED(pool);
  NYAS_ASSERT(h >= 0 && pool.Arr.Size > h && "Invalid handle range.");
}

static void _NyReadFile(void *_1, const char *path, int _2, const char *_3,
                        char **buf, size_t *size) {
  (void)_1, (void)_2, (void)_3;
  ReadFile(path, buf, size);
}

static NyasHandle _CreateMeshHandle(void) { return Meshes.Add(NyasMesh()); }

static NyasHandle _CreateFramebufHandle(void) {
  return Framebufs.Add(NyasFramebuffer());
}

static NyDrawIdx _CheckVertex(const float *v, const float *end,
                              const float *newvtx) {
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

static void _SetMeshObj(NyasMesh *mesh, const char *path) {
  tinyobj_attrib_t attrib;
  tinyobj_shape_t *shapes = NULL;
  size_t shape_count;
  tinyobj_material_t *mats = NULL;
  size_t mats_count;

  int result =
      tinyobj_parse_obj(&attrib, &shapes, &shape_count, &mats, &mats_count,
                        path, _NyReadFile, NULL, TINYOBJ_FLAG_TRIANGULATE);

  NYAS_ASSERT(result == TINYOBJ_SUCCESS && "Obj loader failed.");
  if (result != TINYOBJ_SUCCESS) {
    NYAS_LOG_ERR("Error loading obj. Err: %d", result);
  }

  size_t vertex_count = attrib.num_face_num_verts * 3;

  NYAS_FREE(mesh->Vtx);
  NYAS_FREE(mesh->Indices);

  mesh->Attribs = NyasVtxAttribFlags_Position | NyasVtxAttribFlags_Normal |
                  NyasVtxAttribFlags_Tangent | NyasVtxAttribFlags_Bitangent |
                  NyasVtxAttribFlags_UV;
  mesh->VtxSize = vertex_count * 14 * sizeof(float);
  mesh->ElementCount = vertex_count;
  mesh->Vtx = (float *)NYAS_ALLOC(mesh->VtxSize);
  mesh->Indices =
      (NyDrawIdx *)NYAS_ALLOC(mesh->ElementCount * sizeof(NyDrawIdx));

  float *vit = mesh->Vtx;

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
      nym::vec3_t delta_p1 = (nym::vec3_t)&v2[0] - (nym::vec3_t)&v1[0];
      nym::vec3_t delta_p2 = (nym::vec3_t)&v3[0] - (nym::vec3_t)&v1[0];
      nym::vec2_t delta_uv1 = (nym::vec2_t)&v2[12] - (nym::vec2_t)&v1[12];
      nym::vec2_t delta_uv2 = (nym::vec2_t)&v3[12] - (nym::vec2_t)&v1[12];
      float r =
          1.0f / (delta_uv1[0] * delta_uv2[1] - delta_uv1[1] * delta_uv2[0]);

      nym::vec3_t tn = delta_p1 * delta_uv2[1];
      nym::vec3_t bitn = delta_p2 * delta_uv1[0];
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
      NyDrawIdx nxt_idx = _CheckVertex(mesh->Vtx, vit, v1);
      mesh->Indices[index_offset++] = nxt_idx;
      if (nxt_idx * 14 == (vit - mesh->Vtx)) {
        for (int j = 0; j < 14; ++j) {
          *vit++ = v1[j];
        }
      }

      nxt_idx = _CheckVertex(mesh->Vtx, vit, v2);
      mesh->Indices[index_offset++] = nxt_idx;
      if (nxt_idx * 14 == (vit - mesh->Vtx)) {
        for (int j = 0; j < 14; ++j) {
          *vit++ = v2[j];
        }
      }

      nxt_idx = _CheckVertex(mesh->Vtx, vit, v3);
      mesh->Indices[index_offset++] = nxt_idx;
      if (nxt_idx * 14 == (vit - mesh->Vtx)) {
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

static void _SetMeshMsh(NyasMesh *mesh, const char *path) {
  char *data;
  size_t sz;
  _NyReadFile(NULL, path, 0, NULL, &data, &sz);
  if (!data || !sz) {
    NYAS_LOG_ERR("Problem reading file %s", path);
    return;
  }

  NYAS_FREE(mesh->Vtx);
  NYAS_FREE(mesh->Indices);

  mesh->Attribs = NyasVtxAttribFlags_Position | NyasVtxAttribFlags_Normal |
                  NyasVtxAttribFlags_Tangent | NyasVtxAttribFlags_Bitangent |
                  NyasVtxAttribFlags_UV;
  mesh->VtxSize = *(size_t *)data;
  data += sizeof(size_t);
  mesh->Vtx = (float *)NYAS_ALLOC(mesh->VtxSize);
  memcpy(mesh->Vtx, data, mesh->VtxSize);
  data += mesh->VtxSize;

  mesh->ElementCount = (*(size_t *)data) / sizeof(NyDrawIdx);
  data += sizeof(size_t);
  mesh->Indices =
      (NyDrawIdx *)NYAS_ALLOC(mesh->ElementCount * sizeof(NyDrawIdx));
  memcpy(mesh->Indices, data, mesh->ElementCount * sizeof(NyDrawIdx));

  NYAS_FREE(data - mesh->VtxSize - (2 * sizeof(size_t)));
}

void ReloadMesh(NyasHandle msh, const char *path) {
  NyasMesh *m = &Meshes[msh];
  size_t len = strlen(path);
  const char *extension = path + len;
  while (*--extension != '.') {
  }
  extension++;
  if (!strcmp(extension, "obj")) {
    _SetMeshObj(m, path);
  } else if (!strcmp(extension, "msh")) {
    _SetMeshMsh(m, path);
  } else {
    NYAS_LOG_ERR("Extension (%s) of file %s not recognised.", extension, path);
  }

  m->Resource.Flags |= NyasResourceFlags_Dirty;
}

static NyasHandle _NewMesh(void) {
  NyasHandle mesh_handle = _CreateMeshHandle();
  Meshes[mesh_handle].Resource.ID = 0;
  Meshes[mesh_handle].Resource.Flags = NyasResourceFlags_Dirty;
  Meshes[mesh_handle].Attribs = 0;
  Meshes[mesh_handle].Vtx = NULL;
  Meshes[mesh_handle].Indices = NULL;
  Meshes[mesh_handle].VtxSize = 0;
  Meshes[mesh_handle].ElementCount = 0;
  Meshes[mesh_handle].ResVtx.ID = 0;
  Meshes[mesh_handle].ResVtx.Flags = NyasResourceFlags_Dirty;
  Meshes[mesh_handle].ResIdx.ID = 0;
  Meshes[mesh_handle].ResIdx.Flags = NyasResourceFlags_Dirty;

  return mesh_handle;
}

NyasHandle CreateMesh(void) { return _NewMesh(); }

NyasHandle LoadMesh(const char *path) {
  NyasHandle mesh_handle = _NewMesh();
  ReloadMesh(mesh_handle, path);
  return mesh_handle;
}

NyasHandle CreateFramebuffer(void) {
  NyasHandle framebuffer = _CreateFramebufHandle();
  Framebufs[framebuffer].Resource.ID = 0;
  Framebufs[framebuffer].Resource.Flags = NyasResourceFlags_Dirty;
  for (int i = 0; i < 8; ++i) {
    Framebufs[framebuffer].Target[i].Tex.Index = NyasCode_None;
  }
  return framebuffer;
}

void SetFramebufferTarget(NyasHandle framebuffer, int index,
                          NyasTexTarget target) {
  Framebufs[framebuffer].Resource.Flags |= NyasResourceFlags_Dirty;
  Framebufs[framebuffer].Target[index] = target;
}

using namespace nyas::render;

static void _SyncMesh(NyasHandle msh, NyasHandle shader) {
  _NyCheckHandle(msh, Meshes);
  //_NyCheckHandle(shader, Shaders);
  NyasMesh *m = &Meshes[msh];

  if (!(m->Resource.Flags & NyasResourceFlags_Created)) {
    _NyCreateMesh(&m->Resource.ID, &m->ResVtx.ID, &m->ResIdx.ID);
    m->Resource.Flags |= NyasResourceFlags_Created;
  }

  if (m->Resource.Flags & NyasResourceFlags_Dirty) {
    _NySetMesh(m, GShaders.InternalIDs[shader]);
    m->Resource.Flags &= ~NyasResourceFlags_Dirty;
  }
}

static void _SyncFramebuf(NyasHandle framebuffer) {
  NyasFramebuffer *fb = &Framebufs[framebuffer];
  if (!(fb->Resource.Flags & NyasResourceFlags_Created)) {
    _NyCreateFramebuf(fb);
    fb->Resource.Flags |= NyasResourceFlags_Created;
  }

  _NyUseFramebuf(fb->Resource.ID);
  if (fb->Resource.Flags & NyasResourceFlags_Dirty) {
    for (int i = 0; i < 2; ++i) // TODO: ya sabes
    {
      _NySetFramebuf(fb->Resource.ID, &fb->Target[i]);
    }
    fb->Resource.Flags &= ~NyasResourceFlags_Dirty;
  }
}

void Draw(NyasDrawCmd *cmd) {
  GTextures.Sync();
  if (cmd->Framebuf != NyasCode_NoOp) {
    if (cmd->Framebuf == NyasCode_Default) {
      _NyUseFramebuf(0);
    } else {
      _SyncFramebuf(cmd->Framebuf);
    }
  }

  if (cmd->Shader != NyasCode_NoOp) {
    GShaders.Sync(cmd->Shader);
  }

  NyasDrawState &s = cmd->State;
  _NyClearColor(s.BgColorR, s.BgColorG, s.BgColorB, s.BgColorA);
  _NyClear(s.EnableFlags & NyasDrawFlags_ColorClear,
           s.EnableFlags & NyasDrawFlags_DepthClear,
           s.EnableFlags & NyasDrawFlags_StencilClear);

  _NyViewport({s.ViewportMinX, s.ViewportMinY, s.ViewportMaxX, s.ViewportMaxY});
  _NyScissor({s.ScissorMinX, s.ScissorMinY, s.ScissorMaxX, s.ScissorMaxY});

  if (s.DisableFlags & NyasDrawFlags_DepthTest) {
    _NyDisableDepthTest();
  } else if (s.EnableFlags & NyasDrawFlags_DepthTest) {
    _NyEnableDepthTest();
  }

  if (s.DisableFlags & NyasDrawFlags_DepthWrite) {
    _NyDisableDepthMask();
  } else if (s.EnableFlags & NyasDrawFlags_DepthWrite) {
    _NyEnableDepthMask();
  }

  if (s.DisableFlags & NyasDrawFlags_StencilTest) {
    _NyDisableStencilTest();
  } else if (s.EnableFlags & NyasDrawFlags_StencilTest) {
    _NyEnableStencilTest();
  }

  if (s.DisableFlags & NyasDrawFlags_StencilWrite) {
    _NyDisableStencilMask();
  } else if (s.EnableFlags & NyasDrawFlags_StencilWrite) {
    _NyEnableStencilMask();
  }

  if (s.DisableFlags & NyasDrawFlags_Blend) {
    _NyDisableBlend();
  } else if (s.EnableFlags & NyasDrawFlags_Blend) {
    _NyEnableBlend();
  }

  if (s.DisableFlags & NyasDrawFlags_FaceCulling) {
    _NyDisableCull();
  } else if (s.EnableFlags & NyasDrawFlags_FaceCulling) {
    _NyEnableCull();
  }

  if (s.DisableFlags & NyasDrawFlags_Scissor) {
    _NyDisableScissor();
  } else if (s.EnableFlags & NyasDrawFlags_Scissor) {
    _NyEnableScissor();
  }

  _NySetDepthFunc(s.Depth);
  _NySetBlend(s.BlendSrc, s.BlendDst);
  _NySetCull(s.FaceCulling);

  for (int i = 0; i < cmd->UnitCount; ++i) {
    NyasMesh *imsh = &Meshes[cmd->Units[i].Mesh];
    _NyCheckHandle(cmd->Units[i].Mesh, Meshes);
    NYAS_ASSERT(imsh->ElementCount && "Attempt to draw an uninitialized mesh");

    if (imsh->Resource.Flags & NyasResourceFlags_Dirty) {
      _SyncMesh(cmd->Units[i].Mesh, cmd->Units[i].Shader);
    }

    _NyUseMesh(imsh);
    _NyDraw(imsh->ElementCount, sizeof(NyDrawIdx) == 4,
            cmd->Units[i].Instances);
  }
}
} // namespace nyas

NyasHandle NYAS_SPHERE;
NyasHandle NYAS_CUBE;
NyasHandle NYAS_QUAD;

void NyasCamera::Navigate() {
  nym::vec3_t eye = Eye();
  nym::vec3_t fwd = nym::vec3_t::normal(Fwd()) * -1.0f;
  static nym::vec2_t mouse_down_pos = {0.0f, 0.0f};
  float speed = G_Ctx->Cfg.Navigation.Speed * G_Ctx->Platform.DeltaTime;

  // Rotation
  if (G_Ctx->IO.MouseButton[NyasMouseButton_Right] == NyasKeyState_DOWN) {
    mouse_down_pos = G_Ctx->IO.MousePosition;
  }

  if (G_Ctx->IO.MouseButton[NyasMouseButton_Right] == NyasKeyState_PRESSED) {
    nym::vec2_t curr_pos = G_Ctx->IO.MousePosition;
    nym::vec2_t offset = {(curr_pos.x - mouse_down_pos.x) *
                              G_Ctx->Cfg.Navigation.DragSensibility,
                          (mouse_down_pos.y - curr_pos.y) *
                              G_Ctx->Cfg.Navigation.DragSensibility};

    fwd += (nym::vec3_t::cross(nym::vec3_t::up(), fwd) * -offset.x) +
           (nym::vec3_t::up() * offset.y);
    mouse_down_pos = curr_pos;
  }

  // Position
  if (G_Ctx->IO.Keys[NyasKey_W] == NyasKeyState_PRESSED) {
    eye += fwd * speed;
  }

  if (G_Ctx->IO.Keys[NyasKey_S] == NyasKeyState_PRESSED) {
    eye += fwd * -speed;
  }

  if (G_Ctx->IO.Keys[NyasKey_A] == NyasKeyState_PRESSED) {
    eye += nym::vec3_t::cross(nym::vec3_t::up(), fwd) * speed;
  }

  if (G_Ctx->IO.Keys[NyasKey_D] == NyasKeyState_PRESSED) {
    eye += nym::vec3_t::cross(nym::vec3_t::up(), fwd) * -speed;
  }

  if (G_Ctx->IO.Keys[NyasKey_Space] == NyasKeyState_PRESSED) {
    eye += (nym::vec3_t)nym::vec3_t::up() * speed;
  }

  if (G_Ctx->IO.Keys[NyasKey_LeftShift] == NyasKeyState_PRESSED) {
    eye += nym::vec3_t::up() * -speed;
  }

  mat4_look_at(View, eye, eye + fwd, nym::vec3_t::up());

  // Zoom
  if (G_Ctx->IO.MouseScroll.y != 0.0f) {
    Fov -= G_Ctx->IO.MouseScroll.y * G_Ctx->Cfg.Navigation.ScrollSensibility;
    Fov = nym::clamp(Fov, 1.0f, 120.0f);
    Proj = nym::mat4_t::perspective(
        nym::to_radians(Fov), G_Ctx->Platform.WindowSize.x,
        (float)G_Ctx->Platform.WindowSize.y, 0.1f, Far);
  }
}

static void _MeshSetCube(NyasMesh *mesh) {
  static const float VERTICES[] = {
      -0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f, 0.0f, 0.0f,
      0.5f,  -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f, 1.0f, 0.0f,
      0.5f,  0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f, 1.0f, 1.0f,
      -0.5f, 0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f, 0.0f, 1.0f,

      -0.5f, -0.5f, 0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
      0.5f,  -0.5f, 0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
      0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
      -0.5f, 0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,

      -0.5f, 0.5f,  0.5f,  -1.0f, 0.0f,  0.0f,  1.0f, 0.0f,
      -0.5f, 0.5f,  -0.5f, -1.0f, 0.0f,  0.0f,  1.0f, 1.0f,
      -0.5f, -0.5f, -0.5f, -1.0f, 0.0f,  0.0f,  0.0f, 1.0f,
      -0.5f, -0.5f, 0.5f,  -1.0f, 0.0f,  0.0f,  0.0f, 0.0f,

      0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
      0.5f,  0.5f,  -0.5f, 1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
      0.5f,  -0.5f, -0.5f, 1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
      0.5f,  -0.5f, 0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,

      -0.5f, -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,  0.0f, 1.0f,
      0.5f,  -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,  1.0f, 1.0f,
      0.5f,  -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f,  1.0f, 0.0f,
      -0.5f, -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f,  0.0f, 0.0f,

      -0.5f, 0.5f,  -0.5f, 0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
      0.5f,  0.5f,  -0.5f, 0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
      0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
      -0.5f, 0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
  };

  static const NyDrawIdx INDICES[] = {
      0,  2,  1,  2,  0,  3,  4,  5,  6,  6,  7,  4,  8,  9,  10, 10, 11, 8,
      13, 12, 14, 12, 15, 14, 16, 17, 18, 18, 19, 16, 23, 22, 20, 22, 21, 20,
  };

  NYAS_FREE(mesh->Vtx);
  NYAS_FREE(mesh->Indices);

  mesh->Attribs = NyasVtxAttribFlags_Position | NyasVtxAttribFlags_Normal |
                  NyasVtxAttribFlags_UV;
  mesh->Vtx = (float *)NYAS_ALLOC(sizeof(VERTICES));
  memcpy(mesh->Vtx, VERTICES, sizeof(VERTICES));
  mesh->Indices = (NyDrawIdx *)NYAS_ALLOC(sizeof(INDICES));
  memcpy(mesh->Indices, INDICES, sizeof(INDICES));
  mesh->VtxSize = sizeof(VERTICES);
  mesh->ElementCount = sizeof(INDICES) / sizeof(*INDICES);
}

static void _MeshSetSphere(NyasMesh *mesh, int x_segments, int y_segments) {
  NYAS_ASSERT(y_segments > 2 && x_segments > 2 && "Invalid number of segments");

  const float x_step = 1.0f / (float)(y_segments - 1);
  const float y_step = 1.0f / (float)(x_segments - 1);

  NYAS_FREE(mesh->Vtx);
  NYAS_FREE(mesh->Indices);

  mesh->Attribs = NyasVtxAttribFlags_Position | NyasVtxAttribFlags_Normal |
                  NyasVtxAttribFlags_UV;
  mesh->VtxSize = y_segments * x_segments * 8 * sizeof(float);
  mesh->ElementCount = y_segments * x_segments * 6;
  mesh->Vtx = (float *)NYAS_ALLOC(mesh->VtxSize);
  mesh->Indices =
      (NyDrawIdx *)NYAS_ALLOC(mesh->ElementCount * sizeof(NyDrawIdx));

  float *v = mesh->Vtx;
  for (int y = 0; y < x_segments; ++y) {
    float py = sinf((float)-M_PI_2 + (float)M_PI * (float)y * x_step);
    for (int x = 0; x < y_segments; ++x) {
      float px = cosf(M_PI * 2.0f * x * y_step) * sinf(M_PI * y * x_step);
      float pz = sinf(M_PI * 2.0f * x * y_step) * sinf(M_PI * y * x_step);

      *v++ = px;
      *v++ = py;
      *v++ = pz;
      *v++ = px;
      *v++ = py;
      *v++ = pz;
      *v++ = (float)x * y_step;
      *v++ = (float)y * x_step;
    }
  }

  NyDrawIdx *i = mesh->Indices;
  for (int y = 0; y < x_segments; ++y) {
    for (int x = 0; x < y_segments; ++x) {
      *i++ = y * y_segments + x;
      *i++ = y * y_segments + x + 1;
      *i++ = (y + 1) * y_segments + x + 1;
      *i++ = y * y_segments + x;
      *i++ = (y + 1) * y_segments + x + 1;
      *i++ = (y + 1) * y_segments + x;
    }
  }
}

static void _MeshSetQuad(NyasMesh *mesh) {
  static const float VERTICES[] = {
      -1.0f, -1.0f, 0.0f,  0.0f, 0.0f, -1.0f, 0.0f, 0.0f,  1.0f, -1.0f, 0.0f,
      0.0f,  0.0f,  -1.0f, 1.0f, 0.0f, 1.0f,  1.0f, 0.0f,  0.0f, 0.0f,  -1.0f,
      1.0f,  1.0f,  -1.0f, 1.0f, 0.0f, 0.0f,  0.0f, -1.0f, 0.0f, 1.0f,
  };

  static const NyDrawIdx INDICES[] = {0, 1, 2, 0, 2, 3};

  NYAS_FREE(mesh->Vtx);
  NYAS_FREE(mesh->Indices);

  mesh->Attribs = NyasVtxAttribFlags_Position | NyasVtxAttribFlags_Normal |
                  NyasVtxAttribFlags_UV;
  mesh->Vtx = (float *)NYAS_ALLOC(sizeof(VERTICES));
  memcpy(mesh->Vtx, VERTICES, sizeof(VERTICES));
  mesh->Indices = (NyDrawIdx *)NYAS_ALLOC(sizeof(INDICES));
  memcpy(mesh->Indices, INDICES, sizeof(INDICES));
  mesh->VtxSize = sizeof(VERTICES);
  mesh->ElementCount = sizeof(INDICES) / sizeof(*INDICES);
}

namespace NyUtil {
static void _MeshSetGeometry(NyasHandle msh, NyasGeometry geo) {
  NyasMesh *m = &nyas::Meshes[msh];

  switch (geo) {
  case NyasGeometry_Quad:
    _MeshSetQuad(m);
    break;
  case NyasGeometry_Cube:
    _MeshSetCube(m);
    break;
  case NyasGeometry_Sphere:
    _MeshSetSphere(m, 32, 32);
    break;
  default:
    break;
  }
  m->Resource.Flags |= NyasResourceFlags_Dirty;
}

void LoadBasicGeometries() {
  NYAS_SPHERE = nyas::CreateMesh();
  NYAS_CUBE = nyas::CreateMesh();
  NYAS_QUAD = nyas::CreateMesh();
  _MeshSetGeometry(NYAS_SPHERE, NyasGeometry_Sphere);
  _MeshSetGeometry(NYAS_CUBE, NyasGeometry_Cube);
  _MeshSetGeometry(NYAS_QUAD, NyasGeometry_Quad);
}

void LoadEnv(const char *path, NyasTexture *lut, NyasTexture *sky,
             NyasTexture *irr, NyasTexture *pref) {
  FILE *f = fopen(path, "r");
  char hdr[9];
  fread(hdr, 8, 1, f);
  hdr[8] = '\0';
  if (strncmp("NYAS_ENV", hdr, 9) != 0) {
    NYAS_LOG_ERR("Header of .env file is invalid. Aborting load_env of %s.",
                 path);
    return;
  }

  *sky = GTextures.Alloc({NyasTexFmt_RGB_16F, 1024, 1024, 1},
                         NyasTexFlags_Cubemap);
  size_t size = 1024 * 1024 * 3 * 2;
  NyasTexImage sky_img;
  sky_img.Level = 0;
  for (int i = 0; i < 6; ++i) {
    sky_img.Data[i] = NYAS_ALLOC(size);
    fread(sky_img.Data[i], size, 1, f);
    NYAS_ASSERT(sky_img.Data[i] && "The image couldn't be loaded");
  }
  GTextures.Update(*sky, sky_img);

  *irr = GTextures.Alloc({NyasTexFmt_RGB_16F, 1024, 1024, 1},
                         NyasTexFlags_Cubemap);
  NyasTexImage irrad_img;
  irrad_img.Level = 0;
  for (int i = 0; i < 6; ++i) {
    irrad_img.Data[i] = NYAS_ALLOC(size);
    fread(irrad_img.Data[i], size, 1, f);
    NYAS_ASSERT(irrad_img.Data[i] && "The image couldn't be loaded");
  }
  GTextures.Update(*irr, irrad_img);

  // TODO: Sampler - NyasTexFilter_LinearMipmapLinear
  *pref =
      GTextures.Alloc({NyasTexFmt_RGB_16F, 256, 256, 9}, NyasTexFlags_Cubemap);
  size = 256 * 256 * 3 * 2;
  for (int lod = 0; lod < 9; ++lod) {
    NyasTexImage img;
    img.Level = lod;
    for (int face = 0; face < 6; ++face) {
      img.Data[face] = NYAS_ALLOC(size);
      fread(img.Data[face], size, 1, f);
      NYAS_ASSERT(img.Data[face] && "The image couldn't be loaded");
    }
    GTextures.Update(*pref, img);
    size /= 4;
  }

  size = 512 * 512 * 2 * 2;
  void *img_pix{NYAS_ALLOC(size)};
  fread(img_pix, size, 1, f);
  *lut = GTextures.Alloc({NyasTexFmt_RG_16F, 512, 512, 1});
  GTextures.Update(*lut, {img_pix, 0});

  fclose(f);
}
} // namespace NyUtil
