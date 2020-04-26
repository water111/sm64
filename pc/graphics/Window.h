#ifndef SM64_WINDOW_H
#define SM64_WINDOW_H

#include "Timer.h"
#include "Stats.h"

struct GLFWwindow;

class Window {
public:
  Window();
  GLFWwindow* _window = nullptr;

  void generate_ui();
  void end_frame();

  int _frame = 0;
  bool double_swap = false;
  Timer frame_timer;
  float last_frame_sec = 1;
  float ft_avg = 1;

  void set_stats(float _game, float _dl_copy, float _gfx) {
    game = _game;
    dl_copy = _dl_copy;
    gfx = _gfx;
  }

  float game = 0, dl_copy = 0, gfx = 0;
  RenderStats render_stats;
  DisplayListStats displaylist_stats;
};


#endif //SM64_WINDOW_H
