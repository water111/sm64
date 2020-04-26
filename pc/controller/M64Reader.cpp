#include <cassert>
#include "M64Reader.h"

extern "C" {
#include "pc_cont.h"
#include "types.h"
}

const char* button_names[] = {
  "C_RIGHT",
  "C_LEFT",
  "C_DOWN",
  "C_UP",

  "R",
  "L",
  "INVALID!",
  "INVALID!",

  "DR",
  "DL",
  "DD",
  "DU",

  "START",
  "Z",
  "B",
  "A"
};

void print_button(u16 b) {
  for(int i = 0; i < 16; i++) {
    if(b & (1 << i)) {
      fprintf(stderr, "%s ", button_names[i]);
    }
  }
  fprintf(stderr, "\n");
}

M64Reader *gM64 = nullptr;

void pc_gfx_read_controllers(void* data) {
  if(!gM64) {
    gM64 = new M64Reader("./tas.m64");
    gM64->parse();
  }
  OSContPad *c = (OSContPad *) data;
  u16 button = 0;

  auto x = gM64->next();
//  fprintf(stderr, "[%06d] %d %d (%d) (%03d %03d)", frame_count, button & CONT_A, button & CONT_START, button,
//          x.stick[0], x.stick[1]);
  c[0].stick_x = x.stick[0];
  c[0].stick_y = x.stick[1];
  c[0].button = x.buttons[1] | (x.buttons[0] << 8);
//  print_button(c[0].button);
//  assert((c[0].button & 0xff) == 0);
}

M64Reader::M64Reader(const std::string &_filename) {
  assert(sizeof(ControllerState) == 4);
  filename = _filename;
}


bool M64Reader::parse() {
  auto fp = fopen(filename.c_str(), "rb");
  assert(fp);
  fseek(fp, 0, SEEK_END);
  auto sz = ftell(fp);
  assert((sz & 3) == 0);
  rewind(fp);
  data = new ControllerState[sz / 4];
  auto rv = fread(data, sz, 1, fp);
  assert(rv == 1);
  uint32_t version = *(uint32_t*)(data + 1);
  printf("got version %d\n", version);
  assert(version == 3);
  offset = 256;

  return true;
}

ControllerState& M64Reader::next() {
  auto& x = data[offset];
  offset += 1;
  return x;
}
