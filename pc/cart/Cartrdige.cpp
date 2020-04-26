#include <cstring>
#include "Cartrdige.h"

extern "C" {
#include "pc_cart.h"
}

Cartrdige gCart;

void register_cart_data(void* data, u32 size) {
  gCart.add(data, size);
}

void reload_all() {
  gCart.reset_all();
}


void Cartrdige::add(void *data, int size) {
  auto buffer = new uint8_t[size];
  memcpy(buffer, data, size);
  Entry e;
  e.size = size;
  e.dest = data;
  e.backup = buffer;
  entries.push_back(e);
}


void Cartrdige::reset_all() {
  for(auto& e : entries) {
    memcpy(e.dest, e.backup, e.size);
  }
}