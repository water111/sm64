#ifndef SM64_DISPLAYLISTN64_H
#define SM64_DISPLAYLISTN64_H

#include <cstdint>
#include <vector>
#include "DisplayListMemPool.h"
#include "Stats.h"

extern "C" {
#include "ultra64.h"
};

struct DisplayListNode {
  Gfx data;
};

class DisplayList {
public:
  DisplayList() = default;
  void init_mem(uint32_t _node_count, uint32_t size) {
    node_count = _node_count;
    pool.init_mem(size);
    delete[] nodes;
    nodes = new Gfx[node_count];
  }

  void process(Gfx* input);
  void clear() {
    used_nodes = 0;
    pool.free_all();
  }

  Gfx* nodes = nullptr;
  int node_count = 0;
  int used_nodes = 0;
  DisplayListMemPool pool;
  DisplayListStats getStats() {
    DisplayListStats s;
    s.resources = pool.used();
    s.total_resources = pool.size();
    s.n_entries = used_nodes;
    s.total_entries = node_count;
    return s;
  }
private:
  void insert(const Gfx& data);

};

#endif //SM64_DISPLAYLISTN64_H
