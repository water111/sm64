#include "OpenGLRenderer.h"
#include <string>
#include <vector>
#include <stdexcept>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cassert>

#include "Math.h"
#include "shaders.h"

constexpr bool PRINT_CMDS = false;
constexpr bool PRINT_DEBUG = false;
constexpr bool GETCHAR_FRAME = false;
constexpr bool DONT_DIE = true;

OpenGLRenderer::OpenGLRenderer() {
  compile_shaders();
  init_buffers();
}

void OpenGLRenderer::compile_shaders() {
  printf("--- COMPILE SHADERS ---\n");
  for(auto& shader : shaders) {
    printf("  [%s]...\n", shader.name.c_str());
    shader.vtx_shader = glCreateShader(GL_VERTEX_SHADER);
    auto src = shader.vtx_source.c_str();
    glShaderSource(shader.vtx_shader, 1, &src, nullptr);
    glCompileShader(shader.vtx_shader);

    constexpr int len = 1024;
    int compile_ok;
    char err[len];

    glGetShaderiv(shader.vtx_shader, GL_COMPILE_STATUS, &compile_ok);
    if(!compile_ok) {
      glGetShaderInfoLog(shader.vtx_shader, len, nullptr, err);
      printf("Vertex shader bad: %s\n", err);
      assert(false);
    }


    shader.frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
    src = shader.frag_source.c_str();
    glShaderSource(shader.frag_shader, 1, &src, nullptr);
    glCompileShader(shader.frag_shader);

    glGetShaderiv(shader.frag_shader, GL_COMPILE_STATUS, &compile_ok);
    if(!compile_ok) {
      glGetShaderInfoLog(shader.frag_shader, len, nullptr, err);
      printf("frag shader bad: %s\n", err);
      assert(false);
    }

    shader.program = glCreateProgram();
    glAttachShader(shader.program, shader.vtx_shader);
    glAttachShader(shader.program, shader.frag_shader);
    glLinkProgram(shader.program);


    glGetShaderiv(shader.vtx_shader, GL_LINK_STATUS, &compile_ok);
    if(!compile_ok) {
      glGetShaderInfoLog(shader.vtx_shader, len, nullptr, err);
      printf("linking bad: %s\n", err);
      assert(false);
    }

    for(auto& uniform : shader.uniforms) {
      uniform.id = glGetUniformLocation(shader.program, uniform.name.c_str());
      assert(uniform.id != -1);
    }

    glDeleteShader(shader.vtx_shader);
    glDeleteShader(shader.frag_shader);
    printf("  OK!\n");
  }
}



void OpenGLRenderer::init_buffers() {
  glGenVertexArrays(1, &ogl_vertex_array);
  glGenBuffers(1, &ogl_idx_array);
  glGenBuffers(1, &ogl_vert_data);

  glBindVertexArray(ogl_vertex_array);

  glBindBuffer(GL_ARRAY_BUFFER, ogl_vert_data);
  // tODO static draw?
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer), vertex_buffer, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ogl_idx_array);
  // tODO static draw?
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(tri_verts), tri_verts, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), 0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(7 * sizeof(float)));
  glEnableVertexAttribArray(2);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  glGenTextures(1, &ogl_debug_tex);
  glBindTexture(GL_TEXTURE_2D, ogl_debug_tex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

}

void OpenGLRenderer::render(DisplayList &dl) {
  frame_begin();

  if(PRINT_CMDS) {
    printf("-- Display List --\n");
  }

  for(int i = 0; i < dl.used_nodes; i++) {
    if(PRINT_CMDS) {
      debug_print_cmd(dl.nodes[i]);
    }
    process_gfx(dl.nodes[i]);
  }

  if(GETCHAR_FRAME) {
    getchar();
  }

  flush_pending();
}

void OpenGLRenderer::process_gfx(Gfx &cmd) {
  switch(cmd.type) {


    case PCG_NO_OP:
      break;

    case PCG_SCISSOR: // DOES NOTHING
      handle_scissor(cmd.data.scissor);
      break;

    case PCG_SET_COMBINE_MODE:
      handle_set_combine_mode(cmd.data.combine_mode);
      break;
    case PCG_SET_RENDER_MODE:
      handle_set_render_mode(cmd.data.combine_mode);
      break;
    case PCG_CLEAR_GEOM_MODE:
      handle_clear_geom_mode(cmd.data.data_32);
      break;
    case PCG_SET_GEOM_MODE:
      handle_set_geom_mode(cmd.data.data_32);
      break;
    case PCG_TEXTURE:
      handle_texture(cmd.data.texture);
      break;
    case PCG_MATRIX:
      handle_matrix(cmd.data.matrix);
      break;
    case PCG_VERTEX:
      handle_vertex(cmd.data.vertex);
      break;

    case PCG_TWO_TRIANGLES:
      stats.n_n64_drawcalls++;
      handle_triangle(cmd.data.two_triangles.v00, cmd.data.two_triangles.v01, cmd.data.two_triangles.v02);
      handle_triangle(cmd.data.two_triangles.v10, cmd.data.two_triangles.v11, cmd.data.two_triangles.v12);
      // TODO DEBUG REMOV
//      flush_pending();
      break;

    case PCG_ONE_TRIANGLE:
      stats.n_n64_drawcalls++;
      handle_triangle(cmd.data.one_triangle.v0,cmd.data.one_triangle.v1,cmd.data.one_triangle.v2);
//      flush_pending();
      break;

    case PCG_POP_MATRIX:
      flush_pending();
      if(cmd.data.data_32 == G_MTX_MODELVIEW) {
        gfx_state.mv_mat.pop();
      }
      break;

    case PCG_SET_TEX_LOD:
      flush_pending();
      gfx_state.tex_lod = cmd.data.data_32;
      break;
    case PCG_SET_TEX_LUT:
      flush_pending();
      gfx_state.tex_lut = cmd.data.data_32;
      break;
    case PCG_NUM_LIGHTS:
      flush_pending();
      gfx_state.num_lights = cmd.data.data_32;
      break;
    case PCG_VIEWPORT:
      flush_pending();
      for(int i = 0; i < 3; i++) {
        gfx_state.viewport_scale[i] = cmd.data.viewport.ptr->vp.vscale[i];
        gfx_state.viewport_trans[i] = cmd.data.viewport.ptr->vp.vtrans[i];
      }
      break;
    case PCG_SET_TEX_DETAIL:
      flush_pending();
      gfx_state.tex_detail = cmd.data.data_32;
      break;



    case PCG_SET_DEPTH_SOURCE:
      if(cmd.data.data_32 != G_ZS_PIXEL) {
        assert(false);
      }
      break;

    case PCG_SET_TEX_CONV:
      if(cmd.data.data_32 != G_TC_FILT) {
        assert(false);
      }
      break;

    case PCG_SET_COMB_KEY:
      if(cmd.data.data_32 != G_CK_NONE) {
        assert(false);
      }
      break;

    case PCG_SET_TEX_FILT:
      flush_pending();
      gfx_state.tex_filt = cmd.data.data_32;
      break;

    case PCG_SET_ALPHA_COMPARE:
      flush_pending();
      gfx_state.alpha_mode = cmd.data.data_32;
//      assert(gfx_state.alpha_mode != G_AC_DITHER);
      break;

    case PCG_SET_ENV_COLOR:
      flush_pending();
      gfx_state.env_color[0] = cmd.data.rgba8.r / 255.f;
      gfx_state.env_color[1] = cmd.data.rgba8.g / 255.f;
      gfx_state.env_color[2] = cmd.data.rgba8.b / 255.f;
      gfx_state.env_color[3] = cmd.data.rgba8.a / 255.f;
      break;

    case PCG_SET_FILL_COLOR:
    {
      // maybe we can get away without this...?
      flush_pending();
      u16 lo = cmd.data.data_32;
      u16 hi = (cmd.data.data_32 >> 16);
      assert(lo == hi);
      gfx_state.fill_color[3] = (lo & 1);
      gfx_state.fill_color[2] = ((lo >> 1 ) & 0b11111) / 31.f;
      gfx_state.fill_color[1] = ((lo >> 6 ) & 0b11111) / 31.f;
      gfx_state.fill_color[0] = ((lo >> 11) & 0b11111) / 31.f;
      if(PRINT_DEBUG) {
        printf("fill color (0x%x) %.3f %.3f %.3f %3.f\n",
          lo,
          gfx_state.fill_color[0],
               gfx_state.fill_color[1],
               gfx_state.fill_color[2],
               gfx_state.fill_color[3]);
      }
    } break;


    case PCG_SET_TILE:
    {
      flush_pending();
      auto& x = cmd.data.set_tile;
      assert(x.tile < 8);
      auto& dest = gfx_state.tiles[x.tile];

      dest.fmt = x.fmt;
      dest.siz = x.siz;
      dest.pal = x.palette;
      dest.line = x.line;
      dest.tmem = x.tmem;
      dest.cmt = x.cmt;
      dest.cms = x.cms;
      dest.maskt = x.maskt;
      dest.masks = x.masks;
      dest.shiftt = x.shiftt;
      dest.shifts = x.shifts;
    } break;

    case PCG_SET_TILE_SIZE:
    {
      flush_pending();
      auto& x = cmd.data.set_tile_size;
      assert(x.tile < 8);
      auto& dest = gfx_state.tiles[x.tile];

      dest.uls = x.uls / 4.f;
      dest.ult = x.ult / 4.f;
      dest.lrs = x.lrs / 4.f;
      dest.lrt = x.lrt / 4.f;
    } break;

    case PCG_SET_TEX_IM:
      flush_pending();
      gfx_state.tex_img = cmd.data.tex_image;
      break;

    case PCG_LOAD_BLOCK:
      flush_pending();
      handle_load_block(cmd.data.load_block);
      break;


    // ones that we are ignoring for now but are likely to be needed later:


    case PCG_SET_BLEND_COLOR:

    case PCG_SET_FOG_COLOR:

      if(!DONT_DIE)
        assert(false);
      break;

    // unimplemented texture:
    case PCG_LOAD_TEX_BLOCK:
      if(!DONT_DIE)
        assert(false);
      break;

    // don't care currently, but might in the future
    case PCG_TEXTURE_RECTANGLE:
    case PCG_SET_TEX_PERSP:
    case PCG_LIGHT:
    case PCG_DP_FILL_RECTANGLE:
    case PCG_PERSP_NORMALIZE:
    case PCG_SET_FOG_POS:
      break;

    // ones that are probably very safe to ignore
    case PCG_SET_DEPTH_IMAGE:
    case PCG_SET_COLOR_IMAGE:
    case PCG_FULL_SYNC:
      break;

    default:
      printf("unhandled command!\n");
      debug_print_cmd(cmd);
      if(!DONT_DIE)
        assert(false);
  }
}

void OpenGLRenderer::handle_scissor(PCG_Scissor &cmd) {
  flush_pending();
  // commands are 0.0 to 1023.75.

  // todo, scissor. maybe we can get away without scissor? requires actual px.
}

void OpenGLRenderer::handle_matrix(PCG_Matrix &cmd) {
  flush_pending();
  // convert to floating point
  Mat4f temp;
  guMtxL2F(temp.data, (Mtx*)cmd.ptr);

  if(cmd.param & G_MTX_PROJECTION) {
    if(cmd.param & G_MTX_LOAD) {
      gfx_state.p_mat.no_push_load(temp);
    } else {
      gfx_state.p_mat.no_push_mul(temp);
    }
  } else {
    if(cmd.param & G_MTX_PUSH) {
      if(cmd.param & G_MTX_LOAD) {
        gfx_state.mv_mat.push_load(temp);
      } else {
        gfx_state.mv_mat.push_mul(temp);
      }
    } else {
      if(cmd.param & G_MTX_LOAD) {
        gfx_state.mv_mat.no_push_load(temp);
      } else {
        gfx_state.mv_mat.no_push_mul(temp);
      }
    }
  }
}

inline void xform_pt(float* out, Vtx& in, Mat4f& x, float scale) {
  float pt[4] = {(float)in.v.ob[0], (float)in.v.ob[1],(float)in.v.ob[2],1.f};
  out[0] = 0;
  out[1] = 0;
  out[2] = 0;
  out[3] = 0;

  for(int i = 0; i < 4; i++) {
    for(int j = 0; j < 4; j++) {
      out[i] += pt[j] * x.data[j][i] * scale;
    }
  }
}

void OpenGLRenderer::handle_vertex(PCG_Vertex &vtx) {
  stats.n_verts += vtx.n;
  // check for overflow:
  if(vtx.n + vb_ptr >= VERTEX_BUFFER_SIZE) {
    assert(false);
  }

  float tex_scale[] = {(gfx_state.tex_s_scale /65536.f) / ((gfx_state.tiles[0].lrs - gfx_state.tiles[0].uls + 1) *32),
                       (gfx_state.tex_t_scale /65536.f) /  ((gfx_state.tiles[0].lrt - gfx_state.tiles[0].ult + 1) * 64)};

//  printf("ts: %.3f %3.f (%d %d)\n", tex_scale[0], tex_scale[1], gfx_state.tiles[0].lrs - gfx_state.tiles[0].uls, gfx_state.tiles[0].lrt - gfx_state.tiles[0].ult);

  for(int i = 0; i < vtx.n; i++) {
    int n64_idx = vtx.v0 + i;
    vb_table[n64_idx] = vb_ptr;
    // todo, all the other stuff!
//    Mat4f temp = gfx_state.get_mvp();
//    xform_pt(vertex_buffer[vb_ptr].pos, vtx.ptr[i], temp, 1.f);
    for(int j = 0; j < 3; j++) {
      vertex_buffer[vb_ptr].pos[j] = vtx.ptr[i].v.ob[j];
    }

    if(gfx_state.g_lighting) {
      for(int j = 0; j < 4; j++) {
        vertex_buffer[vb_ptr].rgba[j] = .6f;
      }
    } else {
      for(int j = 0; j < 4; j++) {
        vertex_buffer[vb_ptr].rgba[j] = vtx.ptr[i].v.cn[j] / 255.f;
      }
    }


    for(int j = 0; j < 2; j++) {
      vertex_buffer[vb_ptr].tc[j] = vtx.ptr[i].v.tc[j] * tex_scale[j];
    }
//    printf("TC %.3f %3.f (%d %d)\n", vertex_buffer[vb_ptr].tc[0], vertex_buffer[vb_ptr].tc[1], vtx.ptr[i].v.tc[0], vtx.ptr[i].v.tc[1]);
    vb_ptr++;
  }
}

void OpenGLRenderer::handle_triangle(u32 v0, u32 v1, u32 v2) {
  stats.n_tris++;
  pending_geometry = true;
  if(tri_ptr + 3 >= TRI_VERT_SIZE) {
    assert(false);
  }

  tri_verts[tri_ptr++] = vb_table[v0];
  tri_verts[tri_ptr++] = vb_table[v1];
  tri_verts[tri_ptr++] = vb_table[v2];
}



void OpenGLRenderer::handle_set_combine_mode(PCG_CombineMode &cmd) {
  flush_pending();
  if(cmd.mode1 == G_CC_SHADE && cmd.mode2 == G_CC_SHADE) {
    gfx_state.shading_mode = ShadingMode::ZZZS_ZZZS;


  } else if(cmd.mode1 == G_CC_MODULATERGB && cmd.mode2 == G_CC_MODULATERGB) {
    gfx_state.shading_mode = ShadingMode::TZSZ_ZZZS;

  } else if(cmd.mode1 == G_CC_DECALFADE && cmd.mode2 == G_CC_DECALFADE) {
    gfx_state.shading_mode = ShadingMode::ZZZT_ZZZE;

  } else if(cmd.mode1 == G_CC_DECALRGB && cmd.mode2 == G_CC_DECALRGB) {
    gfx_state.shading_mode = ShadingMode::ZZZT_ZZZS;

  } else if(cmd.mode1 == G_CC_MODULATEIDECALA && cmd.mode2 == G_CC_MODULATEIDECALA) {
    gfx_state.shading_mode = ShadingMode::TZSZ_ZZZT;

  } else if(cmd.mode1 == G_CC_FADEA && cmd.mode2 == G_CC_FADEA) {
    gfx_state.shading_mode = ShadingMode::TZEZ_TZEZ;

  } else if(cmd.mode1 == G_CC_DECALRGBA && cmd.mode2 == G_CC_DECALRGBA) {
    gfx_state.shading_mode = ShadingMode::ZZZT_ZZZT;

  } else if((cmd.mode1 == G_CC_MODULATERGBA && cmd.mode2 == G_CC_MODULATERGBA)||
    (cmd.mode1 == G_CC_MODULATEIA && cmd.mode2 == G_CC_MODULATEIA)) {
    gfx_state.shading_mode = ShadingMode::TZSZ_TZSZ;

  } else if(cmd.mode1 == G_CC_MODULATEIFADEA && cmd.mode2 == G_CC_MODULATEIFADEA) {
    gfx_state.shading_mode = ShadingMode::TZSZ_TZEZ;

  } else if (cmd.mode1 == G_CC_MODULATERGBFADE && cmd.mode2 == G_CC_MODULATERGBFADE) {
    gfx_state.shading_mode = ShadingMode::TZSZ_ZZZE;

  } else if(cmd.mode1 == G_CC_SHADEFADEA && cmd.mode2 == G_CC_SHADEFADEA) {
    gfx_state.shading_mode = ShadingMode::ZZZS_ZZZE;

  }

  else {
    if(PRINT_DEBUG)
      printf("[ERROR] unknown color combiner mode\n");
    gfx_state.shading_mode = ShadingMode::ZZZT_ZZZT;
    if(!DONT_DIE)
      assert(false);
  }


}

void OpenGLRenderer::handle_set_render_mode(PCG_CombineMode &cmd) {
  flush_pending();
  if((cmd.mode1 == G_RM_OPA_SURF && cmd.mode2 == G_RM_OPA_SURF2) ||
     (cmd.mode1 == G_RM_AA_OPA_SURF && cmd.mode2 == G_RM_AA_OPA_SURF2) ||
    (cmd.mode1 == G_RM_AA_TEX_EDGE && cmd.mode2 == G_RM_AA_TEX_EDGE2)) // TODO (not sure about this one!
  {
    if(PRINT_DEBUG) {
      printf("set render mode opaque, no z\n");
    }
    gfx_state.blend_mode = BlendMode::OPAQUE_NOZ;
  } else if((cmd.mode1 == G_RM_AA_ZB_OPA_SURF && cmd.mode2 == G_RM_AA_ZB_OPA_SURF2)||
    (cmd.mode1 == G_RM_AA_ZB_OPA_DECAL && cmd.mode2 == G_RM_AA_ZB_OPA_DECAL2) ||// TODO (not sure about this one!
    (cmd.mode1 == G_RM_AA_ZB_TEX_EDGE && cmd.mode2 == G_RM_AA_ZB_TEX_EDGE2) || // TODO (not sure about this one!
    (cmd.mode1 == G_RM_AA_ZB_XLU_SURF && cmd.mode2 == G_RM_AA_ZB_XLU_SURF2)||// TODO (not sure about this one! def wrong.
    (cmd.mode1 == G_RM_AA_ZB_XLU_DECAL && cmd.mode2 == G_RM_AA_ZB_XLU_DECAL2)||// TODO (not sure about this one! def wrong.
    (cmd.mode1 == G_RM_AA_ZB_XLU_INTER && cmd.mode2 == G_RM_AA_ZB_XLU_INTER2))  // TODO (not sure about this one! def wrong.
  {
    if(PRINT_DEBUG) {
      printf("set render mode opaque, yes z\n");
    }
    gfx_state.blend_mode = BlendMode::OPAQUE_YESZ;
  } else if((cmd.mode1 == G_RM_AA_XLU_SURF && cmd.mode2 == G_RM_AA_XLU_SURF2) || (cmd.mode1 == G_RM_XLU_SURF && cmd.mode2 == G_RM_XLU_SURF2)) {
    gfx_state.blend_mode = BlendMode::TRANS_NOZ;
    if(PRINT_DEBUG) {
      printf("set render mode trans, no z\n");
    }
  }

  else {
    if(PRINT_DEBUG)
      printf("[ERROR] unknown handle_set_render_mode mode\n");
    if(!DONT_DIE)
      assert(false);
  }
}

void OpenGLRenderer::handle_texture(PCG_Texture &cmd) {
  flush_pending();
  gfx_state.tex_on = cmd.on;
  gfx_state.tex_s_scale = cmd.sc / 1.f;
  gfx_state.tex_t_scale = cmd.tc / 1.f;
  gfx_state.mip_max = cmd.level;
  gfx_state.tile_descriptor = cmd.tile;
}

void OpenGLRenderer::handle_set_geom_mode(u32 modes) {
  flush_pending();
  if(modes & G_SHADE) {
    gfx_state.g_shade = true;
  }
  if(modes & G_LIGHTING) {
    gfx_state.g_lighting = true;
  }
  if(modes & G_SHADING_SMOOTH) {
    gfx_state.g_shading_smooth = true;
  }
  if(modes & G_ZBUFFER) {
    gfx_state.g_zbuffer = true;
  }
  if(modes & G_TEXTURE_GEN) {
    gfx_state.g_texture_gen = true;
  }
  if(modes & G_TEXTURE_GEN_LINEAR) {
    gfx_state.g_texture_gen_linear = true;
  }
  if(modes & G_CULL_FRONT) {
    gfx_state.g_cull_front = true;
  }
  if(modes & G_CULL_BACK) {
    gfx_state.g_cull_back = true;
  }
  if(modes & G_CULL_BOTH) {
    gfx_state.g_cull_both = true;
  }
  if(modes & G_FOG) {
    gfx_state.g_fog = true;
  }
  if(modes & G_CLIPPING) {
    gfx_state.g_clipping = true;
  }
}

void OpenGLRenderer::handle_clear_geom_mode(u32 modes) {
  flush_pending();
  if(modes & G_SHADE) {
    gfx_state.g_shade = false;
  }
  if(modes & G_LIGHTING) {
    gfx_state.g_lighting = false;
  }
  if(modes & G_SHADING_SMOOTH) {
    gfx_state.g_shading_smooth = false;
  }
  if(modes & G_ZBUFFER) {
    gfx_state.g_zbuffer = false;
  }
  if(modes & G_TEXTURE_GEN) {
    gfx_state.g_texture_gen = false;
  }
  if(modes & G_TEXTURE_GEN_LINEAR) {
    gfx_state.g_texture_gen_linear = false;
  }
  if(modes & G_CULL_FRONT) {
    gfx_state.g_cull_front = false;
  }
  if(modes & G_CULL_BACK) {
    gfx_state.g_cull_back = false;
  }
  if(modes & G_CULL_BOTH) {
    gfx_state.g_cull_both = false;
  }
  if(modes & G_FOG) {
    gfx_state.g_fog = false;
  }
  if(modes & G_CLIPPING) {
    gfx_state.g_clipping = false;
  }
}

void OpenGLRenderer::frame_begin() {
  pending_geometry = false;
  tri_ptr = 0;
  vb_ptr = 0;
  // clear the screen
  glDisable(GL_SCISSOR_TEST);
  glDepthMask(GL_TRUE);
  glClearColor(0.f, 0.1f, 0.f, 1.f);
  glClear(GL_COLOR_BUFFER_BIT);
  // clear the depth buffer
  glClearDepthf(1.f);
  glClear(GL_DEPTH_BUFFER_BIT);
}

void OpenGLRenderer::debug_print_cmd(Gfx &cmd) {
  auto type = cmd.type;
  if(type > PCG_NYI) {
    printf("[INVALID]\n");
    return;
  }

  printf("[%20s] ", cmd_names[type]);

  switch(type) {
    case PCG_SCISSOR:
      printf("(%.3f, %.3f) to (%.3f, %.3f), mode %d\n",
        cmd.data.scissor.ulx, cmd.data.scissor.uly,
        cmd.data.scissor.lrx, cmd.data.scissor.lry,
        cmd.data.scissor.mode);
      break;

    case PCG_SET_COMBINE_MODE:
    case PCG_SET_RENDER_MODE:
      printf("%d %d\n", cmd.data.combine_mode.mode1, cmd.data.combine_mode.mode2);
      break;

    case PCG_SET_TEX_LOD:
    case PCG_SET_TEX_PERSP:
    case PCG_SET_TEX_FILT:
    case PCG_SET_TEX_DETAIL:
    case PCG_SET_COMB_KEY:
    case PCG_SET_ALPHA_COMPARE:
    case PCG_NUM_LIGHTS:
    case PCG_SET_DEPTH_SOURCE:
      printf("%d\n", cmd.data.data_32);
      break;

    case PCG_SET_TEX_LUT:
      switch(cmd.data.data_32) {
        case G_TT_NONE:
          printf("G_TT_NONE\n");
          break;
        case G_TT_RGBA16:
          printf("G_TT_RGBA16\n");
          break;
        case G_TT_IA16:
          printf("G_TT_IA16\n");
          break;
        default:
          assert(false);
      }
      break;

    case PCG_SET_TEX_CONV:
      switch(cmd.data.data_32) {
        case G_TC_CONV:
          printf("G_TC_CONV\n");
          break;
        case G_TC_FILTCONV:
          printf("G_TC_FILTCONV\n");
          break;
        case G_TC_FILT:
          printf("G_TC_FILT\n");
          break;
        default:
          assert(false);
      }
      break;

    case PCG_TEXTURE:
      printf("en? %d, s: %d t: %d mip: %d tdi: %d\n",
        cmd.data.texture.on,
        cmd.data.texture.sc,
        cmd.data.texture.tc,
        cmd.data.texture.level,
        cmd.data.texture.tile);
      break;

    case PCG_VIEWPORT:
      printf("scale (%.3f %.3f %3.f) trans (%.3f %.3f %3.f)\n",
        cmd.data.viewport.ptr->vp.vscale[0] * 0.25f,
        cmd.data.viewport.ptr->vp.vscale[1] * 0.25f,
        cmd.data.viewport.ptr->vp.vscale[2] * 0.25f,
        cmd.data.viewport.ptr->vp.vtrans[0] * 0.25f,
        cmd.data.viewport.ptr->vp.vtrans[1] * 0.25f,
        cmd.data.viewport.ptr->vp.vtrans[2] * 0.25f);
      break;

    case PCG_SET_TILE:
    {
      printf("fmt ");
      auto x = cmd.data.set_tile;
      switch(x.fmt) {
        case G_IM_FMT_RGBA:
          printf("RGBA ");
          break;
        case G_IM_FMT_YUV:
          printf("YUV ");
          break;
        case G_IM_FMT_CI:
          printf("CI ");
          break;
        case G_IM_FMT_IA:
          printf("IA ");
          break;
        case G_IM_FMT_I:
          printf("I ");
          break;
        default:
          assert(false);
      }
      printf("size: %d line %d tmem %d tile %d pal %d cm/mask/shift (%d,%d,%d), (%d,%d,%d)\n",
        x.siz, x.line, x.tmem, x.tile, x.palette, x.cms, x.masks, x.shifts, x.cmt, x.maskt, x.shiftt);

    } break;

    case PCG_SET_TILE_SIZE:
      printf("(%.3f %.3f) (%.3f %.3f)\n",
        cmd.data.set_tile_size.uls / 4.f, cmd.data.set_tile_size.lrs / 4.f,
        cmd.data.set_tile_size.ult / 4.f, cmd.data.set_tile_size.lrt / 4.f);
      break;

    case PCG_SET_TEX_IM:
      printf("format ");
      switch(cmd.data.tex_image.fmt) {
        case G_IM_FMT_RGBA:
          printf("RGBA ");
          break;
        case G_IM_FMT_CI:
          printf("CI ");
          break;
        case G_IM_FMT_IA:
          printf("IA ");
          break;
        case G_IM_FMT_I:
          printf("I ");
          break;
      }

      printf("size %d width %d\n", cmd.data.tex_image.siz, cmd.data.tex_image.width);
      break;



    case PCG_MATRIX:
      printf("%s %s %s\n",
             (cmd.data.matrix.param & G_MTX_PUSH) ? "PUSH" : "NOPUSH",
             (cmd.data.matrix.param & G_MTX_LOAD) ? "LOAD" : "MUL",
             (cmd.data.matrix.param & G_MTX_PROJECTION) ? "PROJ" : "MV");
      break;

    case PCG_CLEAR_GEOM_MODE:
    case PCG_SET_GEOM_MODE:
      debug_print_geom_modes(cmd.data.data_32);
      printf("\n");
      break;

    case PCG_NYI:
      printf("%d\n", cmd.data.data_32);
      break;

    default:
      printf("\n");
  }
}

void OpenGLRenderer::debug_print_geom_modes(u32 modes) {
  if(modes & G_SHADE) {
    printf("G_SHADE ");
  }
  if(modes & G_LIGHTING) {
    printf("G_LIGHTING ");
  }
  if(modes & G_SHADING_SMOOTH) {
    printf("G_SHADING_SMOOTH ");
  }
  if(modes & G_ZBUFFER) {
    printf("G_ZBUFFER ");
  }
  if(modes & G_TEXTURE_GEN) {
    printf("G_TEXTURE_GEN ");
  }
  if(modes & G_TEXTURE_GEN_LINEAR) {
    printf("G_TEXTURE_GEN_LINEAR ");
  }
  if(modes & G_CULL_FRONT) {
    printf("G_CULL_FRONT ");
  }
  if(modes & G_CULL_BACK) {
    printf("G_CULL_BACK ");
  }
  if(modes & G_CULL_BOTH) {
    printf("G_CULL_BOTH ");
  }
  if(modes & G_FOG) {
    printf("G_FOG ");
  }
  if(modes & G_CLIPPING) {
    printf("G_CLIPPING ");
  }
}

void OpenGLRenderer::flush_pending() {
  if(!pending_geometry) return;
  stats.n_opengl_drawcalls++;
  // render
  load_data();
  set_opengl_state();
  render_debug_wireframe();

  // cleanup
  cleanup_pending_geometry();
}



void OpenGLRenderer::cleanup_pending_geometry() {
  pending_geometry = false;
//  vb_ptr = 0;
// TODO MARK STALE VERTS!
  tri_ptr = 0;
}

void OpenGLRenderer::load_data() {

}


void debug_dump_to_file(const char* name, void* mem, u32 size) {
  FILE* fp = fopen(name, "wb");
  assert(fp);
  fwrite(mem, size, 1, fp);
  fclose(fp);
}

void memcpy_swap16(void* dst, void* src, u64 size) {
  assert(!(size&1));
  for(int i = 0; i < size/2; i++) {
    ((u8*)dst)[i*2 + 1] = ((u8*)src)[i*2];
    ((u8*)dst)[i*2] = ((u8*)src)[i*2 + 1];
  }
}

bool got_tex = false;
u8 tex_buffer[32768];
void OpenGLRenderer::handle_load_block(PCG_LoadBlock &lb) {
  assert(lb.tile < 8);
  auto& tile = gfx_state.tiles[lb.tile];

  if(tile.fmt == G_IM_FMT_RGBA && tile.siz == G_IM_SIZ_16b) {
    handle_load_block_rgba5551(lb);
  } else if(tile.fmt == G_IM_FMT_IA && tile.siz == G_IM_SIZ_16b) {
    handle_load_block_ia88(lb);
  } else if(tile.fmt == G_IM_FMT_IA && tile.siz == G_IM_SIZ_8b) {
    handle_load_block_ia44(lb);
  }

  else {
    if(!DONT_DIE)
      assert(false);
  }

}

void OpenGLRenderer::handle_load_block_rgba5551(PCG_LoadBlock &lb) {
//  printf("Load Block rgba5551!\n");
//  printf("tile: %d, ul (%d, %d), lrs %d, dxt %d\n", lb.tile, lb.uls, lb.ult, lb.lrs, lb.dxt);
  assert(lb.tile < 8);
  auto& tile = gfx_state.tiles[lb.tile];
  assert(tile.siz == G_IM_SIZ_16b); // rgba 5551?
  assert(tile.fmt == G_IM_FMT_RGBA);


//  printf("tile width: %d\n", tile.line);
  if(!DONT_DIE)
    assert(tile.line == 0);


//  printf("tmem 0x%x\n", tile.tmem);

  assert(lb.dxt < (1 << 12));


  float dwc = (1.f / lb.dxt) * 2048.f;
//  printf("dwc %.3f\n", dwc);

  int px_width = (int)(dwc) * (1 << (4 - tile.siz));
//  printf("px_width %d\n", px_width);

  memcpy_swap16(tex_buffer, gfx_state.tex_img.ptr, 4096);
//  debug_dump_to_file("tex.data", tex_buffer, 4096);

  glBindTexture(GL_TEXTURE_2D, ogl_debug_tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, px_width, 32, 0, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, tex_buffer);

  got_tex = true;
}

void OpenGLRenderer::handle_load_block_ia88(PCG_LoadBlock &lb) {
//  printf("Load Block ia88!\n");
//  printf("tile: %d, ul (%d, %d), lrs %d, dxt %d\n", lb.tile, lb.uls, lb.ult, lb.lrs, lb.dxt);
  assert(lb.tile < 8);
  auto& tile = gfx_state.tiles[lb.tile];
  assert(tile.siz == G_IM_SIZ_16b);
  assert(tile.fmt == G_IM_FMT_IA);


//  printf("tile width: %d\n", tile.line);
  if(!DONT_DIE)
    assert(tile.line == 0);


//  printf("tmem 0x%x\n", tile.tmem);

  assert(lb.dxt < (1 << 12));


  float dwc = (1.f / lb.dxt) * 2048.f;
//  printf("dwc %.3f\n", dwc);

  int px_width = (int)(dwc) * (1 << (4 - tile.siz));
//  printf("px_width %d\n", px_width);

  // OpenGL Doesn't have an IA format.
  // 2048 entries, each 4 bytes - max
  for(int i = 0; i < 2048; i++) {
    // I
    for(int j = 0; j < 3; j++) {
      tex_buffer[i*4 +j] = ((u8*)gfx_state.tex_img.ptr)[i*2];
    }
    // A
    tex_buffer[i+4 + 3] = ((u8*)gfx_state.tex_img.ptr)[i*2 + 1];
  }

//  debug_dump_to_file("tex.data", tex_buffer, 8192);

  glBindTexture(GL_TEXTURE_2D, ogl_debug_tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, px_width, 32, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, tex_buffer);

//  assert(false);
  got_tex = true;
}

void OpenGLRenderer::handle_load_block_ia44(PCG_LoadBlock &lb) {
//  printf("Load Block ia88!\n");
//  printf("tile: %d, ul (%d, %d), lrs %d, dxt %d\n", lb.tile, lb.uls, lb.ult, lb.lrs, lb.dxt);
  assert(lb.tile < 8);
  auto& tile = gfx_state.tiles[lb.tile];
  assert(tile.siz == G_IM_SIZ_8b);
  assert(tile.fmt == G_IM_FMT_IA);


//  printf("tile width: %d\n", tile.line);
  if(!DONT_DIE)
    assert(tile.line == 0);


//  printf("tmem 0x%x\n", tile.tmem);

  assert(lb.dxt < (1 << 12));


  float dwc = (1.f / lb.dxt) * 2048.f;
//  printf("dwc %.3f\n", dwc);

  int px_width = (int)(dwc) * (1 << (4 - tile.siz));
//  printf("px_width %d\n", px_width);

  // OpenGL Doesn't have an IA format.
  // 2048 entries, each 4 bytes - max
  for(int i = 0; i < 4096; i++) {
    // I
    for(int j = 0; j < 3; j++) {
      tex_buffer[i*4 +j] = ((u8*)gfx_state.tex_img.ptr)[i] >> 4;
    }
    // A
    tex_buffer[i+4 + 3] = ((u8*)gfx_state.tex_img.ptr)[i] & 0xf;
  }

//  debug_dump_to_file("tex.data", tex_buffer, 8192);

  glBindTexture(GL_TEXTURE_2D, ogl_debug_tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, px_width, 32, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, tex_buffer);

//  assert(false);
  got_tex = true;
}

void OpenGLRenderer::set_opengl_state() {
  if(gfx_state.g_zbuffer) {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
  } else {
    glDisable(GL_DEPTH_TEST);
  }

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  if(gfx_state.blend_mode == BlendMode::OPAQUE_YESZ) {
    glDepthMask(GL_TRUE);
  } else {
    glDepthMask(GL_FALSE);
  }

  if(gfx_state.blend_mode == BlendMode::OPAQUE_YESZ || gfx_state.blend_mode == BlendMode::OPAQUE_NOZ) {
    glDisable(GL_BLEND);
  } else {

    glEnable(GL_BLEND);
  }

}

static void setup_cc(Shader & s, int A, int B, int C, int a, int b, int c) {
  glUniform1i(s.get_uniform(UniformKind::A), A);
  glUniform1i(s.get_uniform(UniformKind::B), B);
  glUniform1i(s.get_uniform(UniformKind::C), C);
  glUniform1i(s.get_uniform(UniformKind::a), a);
  glUniform1i(s.get_uniform(UniformKind::b), b);
  glUniform1i(s.get_uniform(UniformKind::c), c);
}


void OpenGLRenderer::render_debug_wireframe() {
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, ogl_debug_tex);
  auto& current_shader = shaders[(int)Shader::Kind::SUPER];
  glUniform1i(current_shader.get_uniform(UniformKind::TEX0), 0);
  glUniform4fv(current_shader.get_uniform(UniformKind::ENV), 1, gfx_state.env_color);


  switch(gfx_state.shading_mode) {
    case ShadingMode::ZZZS_ZZZS:
      setup_cc(current_shader, SUPER_CC_ZERO, SUPER_CC_ZERO, SUPER_CC_SH, SUPER_CC_ZERO, SUPER_CC_ZERO, SUPER_CC_SH);
      break;
    case ShadingMode::TZSZ_ZZZS:
      setup_cc(current_shader, SUPER_CC_T0, SUPER_CC_SH, SUPER_CC_ZERO, SUPER_CC_T0, SUPER_CC_SH, SUPER_CC_ZERO);
      break;
    case ShadingMode::ZZZT_ZZZE:
      setup_cc(current_shader, SUPER_CC_ZERO, SUPER_CC_ZERO, SUPER_CC_T0, SUPER_CC_ZERO, SUPER_CC_ZERO, SUPER_CC_ENV);
      break;
    case ShadingMode::ZZZT_ZZZS:
      setup_cc(current_shader, SUPER_CC_ZERO, SUPER_CC_ZERO, SUPER_CC_T0, SUPER_CC_ZERO, SUPER_CC_ZERO, SUPER_CC_SH);
      break;
    case ShadingMode::TZSZ_ZZZT:
      setup_cc(current_shader, SUPER_CC_T0, SUPER_CC_SH, SUPER_CC_ZERO, SUPER_CC_ZERO, SUPER_CC_ZERO, SUPER_CC_T0);
      break;
    case ShadingMode::TZEZ_TZEZ:
      setup_cc(current_shader, SUPER_CC_T0, SUPER_CC_ENV, SUPER_CC_ZERO, SUPER_CC_T0, SUPER_CC_ENV, SUPER_CC_ZERO);
      break;
    case ShadingMode::ZZZT_ZZZT:
      setup_cc(current_shader, SUPER_CC_ZERO, SUPER_CC_ZERO, SUPER_CC_T0, SUPER_CC_ZERO, SUPER_CC_ZERO, SUPER_CC_T0);
      break;
    case ShadingMode::TZSZ_TZSZ:
      setup_cc(current_shader, SUPER_CC_T0, SUPER_CC_SH, SUPER_CC_ZERO, SUPER_CC_T0, SUPER_CC_SH, SUPER_CC_ZERO);
      break;
    case ShadingMode::TZSZ_TZEZ:
      setup_cc(current_shader, SUPER_CC_T0, SUPER_CC_SH, SUPER_CC_ZERO, SUPER_CC_T0, SUPER_CC_ENV, SUPER_CC_ZERO);
      break;
    case ShadingMode::TZSZ_ZZZE:
      setup_cc(current_shader, SUPER_CC_T0, SUPER_CC_SH, SUPER_CC_ZERO, SUPER_CC_ZERO, SUPER_CC_ZERO, SUPER_CC_ENV);
      break;
    case ShadingMode::ZZZS_ZZZE:
      setup_cc(current_shader, SUPER_CC_ZERO, SUPER_CC_ZERO, SUPER_CC_SH, SUPER_CC_ZERO, SUPER_CC_ZERO, SUPER_CC_ENV);
      break;

    default:
      setup_cc(current_shader, SUPER_CC_ZERO, SUPER_CC_ZERO, SUPER_CC_ZERO, SUPER_CC_ZERO, SUPER_CC_ZERO, SUPER_CC_ZERO);
      if(!DONT_DIE)
        assert(false);
  }


  current_shader.activate();
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  glBindVertexArray(ogl_vertex_array);
  glUniformMatrix4fv(current_shader.get_uniform(UniformKind::MV_MAT), 1, GL_FALSE, &gfx_state.p_mat.mat.data[0][0]);
  glUniformMatrix4fv(current_shader.get_uniform(UniformKind::P_MAT), 1, GL_FALSE, &gfx_state.mv_mat.mat.data[0][0]);
  glUniform3fv(current_shader.get_uniform(UniformKind::SCALE), 1, gfx_state.viewport_scale);
  glBindBuffer(GL_ARRAY_BUFFER, ogl_vert_data);
  glBufferSubData(GL_ARRAY_BUFFER, 0, vb_ptr * sizeof(VertexData), vertex_buffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ogl_idx_array);
  glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(u32) * tri_ptr, tri_verts);
  glDrawElements(GL_TRIANGLES, tri_ptr, GL_UNSIGNED_INT, 0);

}