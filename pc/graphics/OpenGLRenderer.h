#ifndef SM64_OPENGLRENDERER_H
#define SM64_OPENGLRENDERER_H


#include "DisplayList.h"
#include "Math.h"
#include "Stats.h"


/* All Shading Modes
#define	G_CC_SHADE 1                 SH         SH
#define G_CC_MODULATEIDECALA 2       T0*SH      T0
#define	G_CC_MODULATERGB 3           T0*SH      SH
#define G_CC_DECALRGBA 4             T0         T0
#define G_CC_MODULATERGBA 5          T0*SH      T0*SH
#define G_CC_MODULATEIA 6            T0*SH      T0*SH
#define G_CC_SHADEFADEA 7            SH         EV
#define G_CC_MODULATEIFADEA 8        T0*SH      T0*EV
#define G_CC_DECALFADEA 9            T0         T0*EV
#define G_CC_FADEA 10                T0*EV      T0*EV
#define G_CC_BLENDRGBFADEA 11        (T0-SH)*T0A - SH   EV
#define G_CC_MODULATERGBFADE 12      T0*SH      EV
#define G_CC_DECALFADE 13            T0         EV
#define G_CC_MODULATERGBFADEA 14     T0*SH      T0*EV
#define G_CC_BLENDRGBA 15           (T0-SH)*T0A + SH  SH
#define G_CC_MODULATEFADE 16         T0*SH      EV*T0
#define G_CC_DECALRGB 17             T0         SH
#define G_CC_PASS2 18
#define G_CC_FADE 19                 SH*EV      SH*EV
#define G_CC_TRILERP 20
#define G_CC_DECALRGB2 21
#define G_CC_MODULATERGBA_PRIM 22
 */

enum class ShadingMode {
  // set by DPSetCombineMode
  // (a-b)*c+d
  ZZZS_ZZZS,  // G_CC_SHADE, rgba = shade
  TZSZ_ZZZS,  // G_CC_MODULATERGB, rgb = shade*tex, a = shade
  ZZZT_ZZZE,  // G_CC_DECALFADE,
  ZZZT_ZZZS,  // G_CC_DECALRGB
  TZSZ_ZZZT,  // G_CC_MODULATEIDECALA
  TZEZ_TZEZ,  // G_CC_FADEA
  ZZZT_ZZZT,  // G_CC_DECALRGBA
  TZSZ_TZSZ,  // G_CC_MODULATERGBA
  TZSZ_TZEZ,  // G_CC_MODULATEIFADEA
  TZSZ_ZZZE,  // G_CC_MODULATERGBFADE
  ZZZS_ZZZE,  // G_CC_SHADEFADEA
};

enum class BlendMode {
  OPAQUE_NOZ,
  OPAQUE_YESZ,
  TRANS_NOZ,
};



template<int max>
struct MatrixStack {
  void push_load(Mat4f& m) {
    // push
    if(idx == max) {
      assert(false);
    }
    stack[idx] = mat;
    idx++;
    no_push_load(m);
  }

  void push_mul(Mat4f& m) {
    if(idx == max) {
      assert(false);
    }
    stack[idx] = mat;
    idx++;
    no_push_mul(m);
  }

  void no_push_load(Mat4f& m) {
    mat = m;
  }

  void no_push_mul(Mat4f& m) {
    Mat4f temp = mat;
    matmul(&mat, temp, m);
  }

  void pop() {
    if(idx > 0) {
      idx --;
    } else {
      assert(false);
    }

    mat = stack[idx];
  }

  Mat4f stack[max];
  Mat4f mat;
  int idx = 0;
};

struct VertexData {
  float pos[3];
  float rgba[4];
  float tc[2];
};

struct TileData {
  u8 fmt; // rgba, ....
  u8 siz;
  u32 line, tmem;
  u8 pal;
  u32 cmt, maskt, shiftt, cms, masks, shifts;

  float uls, ult, lrs, lrt;
};

class OpenGLRenderer {
public:
  static constexpr int VERTEX_BUFFER_SIZE = 1024 * 16; //??
  static constexpr int N64_VB_SIZE = 128; //??
  static constexpr int TRI_VERT_SIZE = 1024*16; //??
  OpenGLRenderer();
  void render(DisplayList& dl);
  RenderStats stats;
private:
  void compile_shaders();
  void init_buffers();
  void frame_begin();
  void debug_print_cmd(Gfx& cmd);
  void debug_print_geom_modes(u32 modes);
  void process_gfx(Gfx& cmd);

  void handle_scissor(PCG_Scissor& cmd);
  void handle_set_combine_mode(PCG_CombineMode& cmd);

  void handle_set_render_mode(PCG_CombineMode& cmd);
  void handle_clear_geom_mode(u32 modes);
  void handle_set_geom_mode(u32 modes);
  void handle_texture(PCG_Texture& cmd);
  void handle_matrix(PCG_Matrix& cmd);
  void handle_vertex(PCG_Vertex& vtx);
  void handle_triangle(u32 v0, u32 v1, u32 v2);
  void handle_load_block(PCG_LoadBlock& lb);

  void handle_load_block_rgba5551(PCG_LoadBlock& lb);
  void handle_load_block_ia88(PCG_LoadBlock& lb);
  void handle_load_block_ia44(PCG_LoadBlock& lb);

  void set_opengl_state();

  void flush_pending();
  void cleanup_pending_geometry();

  void load_data();
  void render_debug_wireframe();

  struct {
    ShadingMode shading_mode = ShadingMode::ZZZS_ZZZS;
    BlendMode  blend_mode = BlendMode::OPAQUE_YESZ;
    u32 tex_lod = 0;
    u32 tex_lut = G_TT_NONE;
    u32 tex_detail = G_TD_CLAMP;
    u32 tex_filt = G_TF_POINT;

    bool g_shade = false;
    bool g_lighting = false;
    bool g_shading_smooth = false;
    bool g_zbuffer = false;
    bool g_texture_gen = false;
    bool g_texture_gen_linear = false;
    bool g_cull_front = false;
    bool g_cull_back = false;
    bool g_cull_both = false;
    bool g_fog = false;
    bool g_clipping = false;
    int num_lights = 0;

    bool tex_on = false;
    float tex_s_scale = 1.f;
    float tex_t_scale = 1.f;
    u32 mip_max = 0;
    u32 tile_descriptor = 0;
    u32 alpha_mode = G_AC_NONE;

    float viewport_scale[3] = {};
    float viewport_trans[3] = {};

    float fill_color[4] = {0, 0, 0, 0};
    float env_color[4] = {0,0,0,0};
    MatrixStack<1> p_mat;
    MatrixStack<10> mv_mat;

    TileData tiles[8];
    PCG_TexImage tex_img;

    Mat4f get_mvp() {
      Mat4f result;

      //matmul(&result, p_mat.stack.back(), mv_mat.stack.back());
      matmul(&result, mv_mat.mat, p_mat.mat);
      return result;
    }

  } gfx_state;

  bool pending_geometry = false;
  VertexData vertex_buffer[VERTEX_BUFFER_SIZE];
  s32 vb_table[N64_VB_SIZE];
  u32 vb_ptr = 0;

  u32 tri_verts[TRI_VERT_SIZE];
  u32 tri_ptr = 0;

  unsigned int ogl_vertex_array, ogl_idx_array, ogl_vert_data, ogl_debug_tex;


};


#endif //SM64_OPENGLRENDERER_H
