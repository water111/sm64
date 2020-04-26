#include <cstdio>
#include <assert.h>


extern "C" {
#include <ultra64.h>
#include "pc/graphics/pc_gfx.h"
#include "pc/cart/pc_cart.h"
#include "game/main.h"
#include "level_headers.h"
#include "types.h"
}

u32 size_of_macro_obj(const MacroObject* o) {
  u32 sz = 0;
  while( ((*o) & 0x1ff) >= 0x1f) {
    o += 5;
    sz += 5 * sizeof(MacroObject);
  }
  fprintf(stderr, "mo size %d\n", sz);
  return sz;
}


#define REG_DATA(x,y) register_cart_data((void*)x, y)
#define REG_MAC_DATA(x) register_cart_data((void*)x, size_of_macro_obj(x))

int main() {
  printf("mario\n");

  REG_MAC_DATA(wdw_seg7_area_1_macro_objs);
  REG_MAC_DATA(wdw_seg7_area_2_macro_objs);

  REG_MAC_DATA(jrb_seg7_area_1_macro_objs);
  REG_MAC_DATA(jrb_seg7_area_2_macro_objs);
  REG_MAC_DATA(bbh_seg7_macro_objs);
  REG_MAC_DATA(castle_grounds_seg7_macro_objs);
  REG_MAC_DATA(ttc_seg7_macro_objs);
  REG_MAC_DATA(vcutm_seg7_macro_objs);
  REG_MAC_DATA(cotmc_seg7_macro_objs);
  REG_MAC_DATA(sa_seg7_macro_objs);
  REG_MAC_DATA(ddd_seg7_area_1_macro_objs);
  REG_MAC_DATA(ddd_seg7_area_2_macro_objs);
  REG_MAC_DATA(bits_seg7_macro_objs);
  REG_MAC_DATA(ssl_seg7_area_1_macro_objs);
  REG_MAC_DATA(ssl_seg7_area_2_macro_objs);
  REG_MAC_DATA(ssl_seg7_area_3_macro_objs);
  REG_MAC_DATA(bitdw_seg7_macro_objs);
  REG_MAC_DATA(thi_seg7_area_1_macro_objs);
  REG_MAC_DATA(thi_seg7_area_2_macro_objs);
  REG_MAC_DATA(thi_seg7_area_3_macro_objs);
  REG_MAC_DATA(ttm_seg7_area_1_macro_objs);
  REG_MAC_DATA(ttm_seg7_area_2_macro_objs);
  REG_MAC_DATA(ttm_seg7_area_3_macro_objs);
  REG_MAC_DATA(ttm_seg7_area_4_macro_objs);
  REG_MAC_DATA(totwc_seg7_macro_objs);
  REG_MAC_DATA(inside_castle_seg7_area_1_macro_objs);
  REG_MAC_DATA(inside_castle_seg7_area_2_macro_objs);
  REG_MAC_DATA(inside_castle_seg7_area_3_macro_objs);
  REG_MAC_DATA(castle_courtyard_seg7_macro_objs);
  REG_MAC_DATA(bob_seg7_macro_objs);
  REG_MAC_DATA(sl_seg7_area_1_macro_objs);
  REG_MAC_DATA(sl_seg7_area_2_macro_objs);
  REG_MAC_DATA(hmc_seg7_macro_objs);
  REG_MAC_DATA(rr_seg7_macro_objs);
  REG_MAC_DATA(pss_seg7_macro_objs);
  REG_MAC_DATA(ccm_seg7_area_1_macro_objs);
  REG_MAC_DATA(ccm_seg7_area_2_macro_objs);
  REG_MAC_DATA(bitfs_seg7_macro_objs);
  REG_MAC_DATA(wmotr_seg7_macro_objs);
  REG_MAC_DATA(wf_seg7_macro_objs);
  REG_MAC_DATA(lll_seg7_area_1_macro_objs);
  REG_MAC_DATA(lll_seg7_area_2_macro_objs);

  REG_DATA(wdw_seg7_area_1_collision, so_wdw_seg7_area_1_collision);
  REG_DATA(wdw_seg7_area_2_collision, so_wdw_seg7_area_2_collision);
  REG_DATA(wdw_seg7_collision_square_floating_platform, so_wdw_seg7_collision_square_floating_platform);
  REG_DATA(wdw_seg7_collision_arrow_lift, so_wdw_seg7_collision_arrow_lift);
  REG_DATA(wdw_seg7_collision_070184C8, so_wdw_seg7_collision_070184C8);
  REG_DATA(wdw_seg7_collision_07018528, so_wdw_seg7_collision_07018528);
  REG_DATA(wdw_seg7_collision_express_elevator_platform, so_wdw_seg7_collision_express_elevator_platform);
  REG_DATA(wdw_seg7_collision_rect_floating_platform, so_wdw_seg7_collision_rect_floating_platform);
  REG_DATA(wdw_seg7_collision_070186B4, so_wdw_seg7_collision_070186B4);


  reload_all();
  pc_gfx_init();
  pc_port_main_func();
  return 0;
}
