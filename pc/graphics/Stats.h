#ifndef SM64_STATS_H
#define SM64_STATS_H

struct RenderStats {
  int n_verts;
  int n_tris;
  int n_n64_drawcalls;
  int n_opengl_drawcalls;

  void reset() {
    n_verts = 0;
    n_tris = 0;
    n_n64_drawcalls = 0;
    n_opengl_drawcalls = 0;
  }
};

struct DisplayListStats {
  int n_entries;
  int resources;
  int total_resources;
  int total_entries;

};

#endif //SM64_STATS_H
