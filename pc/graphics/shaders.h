#ifndef SM64_SHADERS_H
#define SM64_SHADERS_H

/////////////////////////
// DEBUG ORANGE SHADER //
/////////////////////////

// Draw it in orange always.



static std::string orange_vtx_shader = R"(

#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec4 rgba_in;

uniform mat4 mv_mat;
uniform mat4 p_mat;
uniform vec3 scale;

void main() {
  gl_Position = mv_mat * p_mat * vec4(position.x, position.y, position.z, 1.0);
  gl_Position.x /= scale.x / 512.f;
  gl_Position.y /= scale.y / 512.f;
  gl_Position.z /= 2048.f;
}

)";

static std::string orange_frag_shader = R"(

#version 330 core

out vec4 gl_FragColor;

uniform vec3 debug_color;


void main() {
  gl_FragColor = vec4(debug_color.x, debug_color.y, debug_color.z, 1.0);
}

)";

static std::string shadeonly_vtx_shader = R"(

#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec4 rgba_in;

uniform mat4 mv_mat;
uniform mat4 p_mat;
uniform vec3 scale;

out vec4 fragment_color;

void main() {
  gl_Position = mv_mat * p_mat * vec4(position.x, position.y, position.z, 1.0);
  gl_Position.x /= scale.x / 512.f;
  gl_Position.y /= scale.y / 512.f;
  gl_Position.z /= 2048.f;
  fragment_color = rgba_in;

}

)";

static std::string shadeonly_frag_shader = R"(

#version 330 core

out vec4 gl_FragColor;

in vec4 fragment_color;

void main() {
  gl_FragColor = fragment_color;
}

)";



static std::string shade_only_vtx_shader = R"(
#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec4 rgba_in;
layout (location = 2) in vec2 tex_coord_in;

uniform mat4 mv_mat;
uniform mat4 p_mat;
uniform vec3 scale;

out vec4 fragment_color;
out vec2 tex_coord;

void main() {
  gl_Position = mv_mat * p_mat * vec4(position.x, position.y, position.z, 1.0);
  gl_Position.x /= scale.x / 512.f;
  gl_Position.y /= scale.y / 512.f;
  gl_Position.z /= 4096.f;
  fragment_color = rgba_in;
  tex_coord = tex_coord_in;
}
)";

static std::string shade_only_frag_shader = R"(

#version 330 core

out vec4 gl_FragColor;
in vec4 fragment_color;
in vec2 tex_coord;

uniform sampler2D debug_tex;

void main() {
  gl_FragColor = vec4(fragment_color.x * texture(debug_tex, tex_coord).r, fragment_color.y * texture(debug_tex, tex_coord).g, fragment_color.z * texture(debug_tex, tex_coord).b, fragment_color.w);
  //gl_FragColor = vec4(texture(debug_tex, tex_coord).r, texture(debug_tex, tex_coord).g, texture(debug_tex, tex_coord).b, fragment_color.w);
  //gl_FragColor = texture(debug_tex, tex_coord);
  //gl_FragColor = vec4(fragment_color.x,  texture(debug_tex, tex_coord).g, texture(debug_tex, tex_coord).b, 1.0);
}

)";


static std::string super_vtx_shader = R"(
#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec4 rgba_in;
layout (location = 2) in vec2 tex_coord_in;

uniform mat4 mv_mat;
uniform mat4 p_mat;
uniform vec3 scale;

out vec4 SH;
out vec2 tex_coord;

void main() {

  gl_Position = mv_mat * p_mat * vec4(position.x, position.y, position.z, 1.0);
  gl_Position.x /= scale.x / 512.f;
  gl_Position.y /= scale.y / 512.f;
  gl_Position.z /= 4096.f;

  SH = rgba_in;
  tex_coord = tex_coord_in;
}


  )";

static std::string super_frag_shader = R"(
#version 330 core

out vec4 gl_FragColor;
in vec4 SH;
in vec2 tex_coord;

uniform sampler2D tex_T0;
uniform int a_idx, b_idx, c_idx, A_idx, B_idx, C_idx;
uniform vec4 ENV;

void main() {
  vec3 A = vec3(0);
  vec3 B = vec3(0);
  vec3 C = vec3(0);

  float a = 0, b = 0, c = 0;


  vec4 T0 = texture(tex_T0, tex_coord);

  if(A_idx == 0) A = T0.rgb;
  else if(A_idx == 1) A = SH.rgb;
  else if(A_idx == 2) A = ENV.rgb;

  if(B_idx == 0) B = T0.rgb;
  else if(B_idx == 1) B = SH.rgb;
  else if(B_idx == 2) B = ENV.rgb;

  if(C_idx == 0) C = T0.rgb;
  else if(C_idx == 1) C = SH.rgb;
  else if(C_idx == 2) C = ENV.rgb;

  if(a_idx == 0) a = T0.a;
  else if(a_idx == 1) a = SH.a;
  else if(a_idx == 2) a = ENV.a;

  if(b_idx == 0) b = T0.a;
  else if(b_idx == 1) b = SH.a;
  else if(b_idx == 2) b = ENV.a;

  if(c_idx == 0) c = T0.a;
  else if(c_idx == 1) c = SH.a;
  else if(c_idx == 2) c = ENV.a;

  gl_FragColor = vec4(A*B + C, a*b + c);
}
  )";

enum class UniformKind {
  MV_MAT,
  P_MAT,
  SCALE,
  DEBUG_COLOR,
  DEBUG_TEX,

  TEX0,
  a,
  b,
  c,
  A,
  B,
  C,
  ENV
};

enum Shader_CC {
  SUPER_CC_T0 = 0,
  SUPER_CC_SH = 1,
  SUPER_CC_ENV = 2,
  SUPER_CC_ZERO = 3
};

struct UniformMap {
  std::string name;
  UniformKind kind;
  int id = -1;
};

struct Shader {
  enum class Kind {
    ORANGE,
    RGBA_SHADE,
    SHADE_ONLY,
    SUPER
  } kind;

  std::string name;
  const std::string& vtx_source;
  const std::string& frag_source;

  std::vector<UniformMap> uniforms;

  int vtx_shader;
  int frag_shader;
  int program;

  void activate() {
    glUseProgram(program);
  }

  int get_uniform(UniformKind k) {
    for(auto& x : uniforms) {
      if(x.kind == k) return x.id;
    }
    throw std::runtime_error("bad uniform");
  }
};

static Shader shaders[] = {
  {Shader::Kind::ORANGE, "orange-debug", orange_vtx_shader, orange_frag_shader,
    {{"mv_mat", UniformKind::MV_MAT},
      {"p_mat", UniformKind::P_MAT},
      {"scale", UniformKind::SCALE},
      {"debug_color", UniformKind::DEBUG_COLOR}}
  },

  {Shader::Kind::RGBA_SHADE, "RGBA_SHADE", shade_only_vtx_shader, shade_only_frag_shader,
    {{"mv_mat", UniformKind::MV_MAT},
      {"p_mat", UniformKind::P_MAT},
      {"scale", UniformKind::SCALE},
      {"debug_tex", UniformKind::DEBUG_TEX}}
  },

  {Shader::Kind::SHADE_ONLY, "shade-only", shadeonly_vtx_shader, shadeonly_frag_shader,
      {{"mv_mat", UniformKind::MV_MAT},
       {"p_mat", UniformKind::P_MAT},
       {"scale", UniformKind::SCALE}}
  },

  {Shader::Kind::SUPER, "super", super_vtx_shader, super_frag_shader,
    {{"mv_mat", UniformKind::MV_MAT},
      {"p_mat", UniformKind::P_MAT},
      {"scale", UniformKind::SCALE},
      {"tex_T0", UniformKind::TEX0},
      {"a_idx", UniformKind::a},
      {"b_idx", UniformKind::b},
      {"c_idx", UniformKind::c},
      {"A_idx", UniformKind::A},
      {"B_idx", UniformKind::B},
      {"C_idx", UniformKind::C},
      {"ENV", UniformKind::ENV}
    }
   }
};

static const char* cmd_names[] = {
  "PCG_VIEWPORT",
  "PCG_SCISSOR",
  "PCG_CLEAR_GEOM_MODE",
  "PCG_END_DISPLAY_LIST",
  "PCG_VERTEX",
  "PCG_SET_TEX_IM",
  "PCG_DISPLAY_LIST",
  "PCG_NO_OP",
  "PCG_ONE_TRIANGLE",
  "PCG_SET_COMBINE_MODE",
  "PCG_SET_TEX_LOD",
  "PCG_SET_TEX_LUT",
  "PCG_SET_TEX_DETAIL",
  "PCG_SET_TEX_PERSP",
  "PCG_SET_TEX_FILT",
  "PCG_SET_TEX_CONV",
  "PCG_SET_COMB_KEY",
  "PCG_SET_ALPHA_COMPARE",
  "PCG_SET_RENDER_MODE",
  "PCG_SET_COLOR_DITHER",
  "PCG_SET_CYCLE_TYPE",
  "PCG_SET_GEOM_MODE",
  "PCG_NUM_LIGHTS",
  "PCG_TEXTURE",
  "PCG_SET_DEPTH_SOURCE",
  "PCG_SET_DEPTH_IMAGE",
  "PCG_SET_COLOR_IMAGE",
  "PCG_SET_FILL_COLOR",
  "PCG_DP_FILL_RECTANGLE",
  "PCG_FULL_SYNC",
  "PCG_TEXTURE_RECTANGLE",
  "PCG_SET_TILE",
  "PCG_SET_TILE_SIZE",
  "PCG_LOAD_BLOCK",
  "PCG_MATRIX",
  "PCG_POP_MATRIX",
  "PCG_PERSP_NORMALIZE",
  "PCG_SET_ENV_COLOR",
  "PCG_BRANCH_LIST",
  "PCG_LOAD_TEX_BLOCK",
  "PCG_LIGHT",
  "PCG_TWO_TRIANGLES",
  "PCG_GEOM_MODE",
  "PCG_SET_FOG_COLOR",
  "PCG_SET_FOG_POS",
  "PCG_SET_BLEND_COLOR",
  "PCG_NYI",
};

#endif //SM64_SHADERS_H
