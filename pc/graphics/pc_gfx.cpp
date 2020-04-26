#include "DisplayListPool.h"
#include "Window.h"
#include "OpenGLRenderer.h"


extern "C" {
#include "pc_gfx.h"
#include "types.h"
}


DisplayListPool* gDisplayListPool = nullptr;
Window *gWindow = nullptr;
OpenGLRenderer *gRenderer = nullptr;

bool gWantExit = false;

static int frame_count = 0;
static int cont_idx = -10;
static float dl_proc_time = 0;
uint64_t game_frames;

void frame_stats() {
  //if(frame_count % 256 == 0)
//  printf("frame %05d gfx %.3f game+dl_proc %.3f dl_proc %.3f (sec %.3f min %.3f)\n", frame_count,
//    gGraphicsSync->last_gfx_ms, gGraphicsSync->last_game_ms, dl_proc_time, frame_count / 60.f, frame_count / 3600.f);

  gWindow->set_stats(
                     game_frames,
                     dl_proc_time,
                     0);
  gWindow->render_stats = gRenderer->stats;
  gRenderer->stats.reset();

  frame_count++;
}

// Initialze Graphics.
void pc_gfx_init() {
  gDisplayListPool = new DisplayListPool(8 * 1024, 2 * 1024 * 1024);
  gWindow = new Window;
  gRenderer = new OpenGLRenderer;
}


Timer render_time;


// call from game to provide a display list
void pc_gfx_set_display_list(void* display_list) {
  game_frames++;
  Timer display_list_processing_timer;

  // get a new display list
  auto dest = gDisplayListPool->alloc();
  dest->clear();

  // process N64 display list
  dest->process((Gfx*)display_list);

  // mark this new display list as available to the rendering thread
  gDisplayListPool->finish_loading(dest);
  dl_proc_time = display_list_processing_timer.getMs();

  if(render_time.getMs() > 0.3 || game_frames > 140000) {
    frame_stats();
    auto list = gDisplayListPool->get_list_for_rendering();
    gRenderer->render(*list);
    gWindow->end_frame();
    list->clear();
    gDisplayListPool->finish_rendering(list);
    render_time.start();
  }

}